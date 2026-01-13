/*
===============================================================================
  game.cpp — Implémentation de la logique de jeu
-------------------------------------------------------------------------------
  Rôle :
    - Initialiser l’état global du jeu.
    - Charger les niveaux.
    - Appliquer les règles (rules_parse).
    - Appliquer les déplacements (step).
    - Gérer les états (victoire, mort).
    - Dessiner la grille avec caméra (centrage sur YOU + joystick libre).
    - Fournir transitions (fade_in/out) et écran de titre.
    - Helpers de progression (win/continue, restart after death).
===============================================================================
*/

#include "game.h"
#include "core/movement.h"
#include "core/sprites.h"
#include "core/audio.h"
#include "game/levels.h"
#include "core/input.h"
#include "core/graphics.h"

#include "assets/gfx/title.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <algorithm> // min/max
#include "music_map.h"


namespace baba {

// ============================================================================
//  ÉTAT GLOBAL DU JEU
// ============================================================================
static GameState g_state;
static GameMode  g_mode = GameMode::Title;

// Musique forcée (optionnelle) et mode de persistance
static MusicID g_forcedMusic = MusicID::NONE;
static bool    g_forceMusicAcrossLevels = false; // true = persister sur tous les niveaux


GameState& game_state() { return g_state; }
GameMode&  game_mode()  { return g_mode; }

// Définir une musique forcée.
// - id != NONE : forcer cette musique.
// - id == NONE : revenir au comportement normal (musique par niveau).
// - persistAcrossLevels = true : la même musique est utilisée pour tous les niveaux.
// - persistAcrossLevels = false : override "one-shot" (réinitialisé au prochain chargement).
void game_set_forced_music(MusicID id, bool persistAcrossLevels)
{
    g_forcedMusic = id;
    g_forceMusicAcrossLevels = persistAcrossLevels;
}

// Récupérer la musique forcée actuelle (pour le menu, debug éventuel)
MusicID game_get_forced_music()
{
    return g_forcedMusic;
}


// ============================================================================
//  Caméra
// ============================================================================
struct Camera {
    float x = 0.0f;
    float y = 0.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
};

static Camera g_camera;

constexpr int SCREEN_W      = 320;
constexpr int SCREEN_H      = 240;
constexpr int VIEW_TILES_W  = SCREEN_W / TILE_SIZE; // 20
constexpr int VIEW_TILES_H  = SCREEN_H / TILE_SIZE; // 15

struct Point { int x; int y; };

// Trouve la position du premier objet YOU
static Point find_you(const Grid& g, const PropertyTable& props) {
    for (int y = 0; y < g.height; ++y) {
        for (int x = 0; x < g.width; ++x) {
            for (const auto& obj : g.cell(x, y).objects) {
                const Properties& pr = props[(int)obj.type];
                if (pr.isYou) {
                    return {x, y};
                }
            }
        }
    }
    return {g.width / 2, g.height / 2}; // fallback
}

// Mise à jour de la caméra
static void update_camera(const Grid& g, const PropertyTable& props,
                          int joyX, int joyY)
{
    Point youPos = find_you(g, props);

    float centerX = youPos.x - (VIEW_TILES_W / 2.0f);
    float centerY = youPos.y - (VIEW_TILES_H / 2.0f);

    if (joyX != 0 || joyY != 0) {
        g_camera.offsetX += joyX;
        g_camera.offsetY += joyY;
    } else {
        g_camera.offsetX = 0.0f;
        g_camera.offsetY = 0.0f;
    }

    g_camera.x = centerX + g_camera.offsetX;
    g_camera.y = centerY + g_camera.offsetY;

    // Clamp pour éviter de sortir de la grille
    float maxX = static_cast<float>(g.width - VIEW_TILES_W);
    float maxY = static_cast<float>(g.height - VIEW_TILES_H);
    if (g_camera.x < 0.0f) g_camera.x = 0.0f;
    if (g_camera.y < 0.0f) g_camera.y = 0.0f;
    if (g_camera.x > maxX) g_camera.x = maxX;
    if (g_camera.y > maxY) g_camera.y = maxY;
}

/*
===============================================================================
  Transitions (fade-in / fade-out)
===============================================================================
*/
void fade_out(int delayMs, int steps)
{
    for (int i = 0; i < steps; i++) {
        uint16_t shade = (i * 255 / steps);
        uint16_t color = (shade >> 3) << 11 | (shade >> 2) << 5 | (shade >> 3);
        gfx_clear(color);
        gfx_flush();
        vTaskDelay(pdMS_TO_TICKS(delayMs));
    }
}

void fade_in(int delayMs, int steps)
{
    for (int i = steps - 1; i >= 0; i--) {
        uint16_t shade = (i * 255 / steps);
        uint16_t color = (shade >> 3) << 11 | (shade >> 2) << 5 | (shade >> 3);
        gfx_clear(color);
        gfx_flush();
        vTaskDelay(pdMS_TO_TICKS(delayMs));
    }
}

// ============================================================================
//  INITIALISATION DU JEU
// ============================================================================
void game_init() {
    g_state = GameState{};
    sprites_init();
    game_load_level(0);
}

// ============================================================================
//  CHARGEMENT DE NIVEAU
// ============================================================================
// ============================================================================
//  CHARGEMENT DE NIVEAU
// ============================================================================
void game_load_level(int index) {
    g_state.currentLevel = index;
    g_state.hasWon = false;
    g_state.hasDied = false;

    load_level(index, g_state.grid);
    rules_parse(g_state.grid, g_state.props);

    g_camera = Camera{};

    // Sélection de la musique
    MusicID music = MusicID::NONE;

    if (g_forcedMusic != MusicID::NONE) {
        // Mode override
        music = g_forcedMusic;

        // Si l’override n’est pas persistant, on le consomme une fois
        if (!g_forceMusicAcrossLevels) {
            g_forcedMusic = MusicID::NONE;
        }
    } else {
        // Musique par défaut du niveau
        music = getMusicForLevel(index);
    }

    // Lecture de la musique choisie
    audio_play_music_id(music);
}


// ============================================================================
//  ÉCRAN DE TITRE
// ============================================================================
void game_show_title()
{
    gfx_clear(COLOR_BLACK);
    // Adapter si image en mode portrait: params (pixels, w, h, x, y)
    gfx_blit(title_pixels, 320, 240, 0, 0);
    gfx_flush();
}

// ============================================================================
//  game_update() — Mise à jour logique du jeu
// ============================================================================
void game_update() {
	// Réinitialiser les flags à chaque frame
    g_state.hasWon  = false;
    g_state.hasDied = false;

    // Lecture des entrées directionnelles
    int dx = 0, dy = 0;
    if (g_keys.left)  dx = -1;
    else if (g_keys.right) dx = +1;
    else if (g_keys.up)    dy = -1;
    else if (g_keys.down)  dy = +1;

    // Déplacement si demandé
    if (dx != 0 || dy != 0) {
		// step() : snapshot → push → move → effects
        MoveResult r = step(g_state.grid, g_state.props, dx, dy);
		
		// Recalcul des règles après chaque mouvement
        rules_parse(g_state.grid, g_state.props);
		
		// Mettre à jour les flags
        g_state.hasWon  = r.hasWon;
        g_state.hasDied = r.hasDied;
    }

    // Mise à jour caméra (toujours, même si pas de déplacement)
    update_camera(g_state.grid, g_state.props, g_keys.joyX, g_keys.joyY);
}

/* ===============================================================================
   Helpers de progression (utilisés par task_game)
   =============================================================================== 
*/
void game_win_continue() 
{ 
    int next = g_state.currentLevel + 1; 
    if (next >= levels_count()) {
        next = 0;
    } 
    game_load_level(next); 
} 

void game_restart_after_death() 
{ 
    game_load_level(g_state.currentLevel); 
}

/*
===============================================================================
  DESSIN DU JEU
===============================================================================
*/
void game_draw() {
    gfx_clear(COLOR_BLACK);

    int camTileX = static_cast<int>(g_camera.x);
    int camTileY = static_cast<int>(g_camera.y);

    int endX = std::min(camTileX + VIEW_TILES_W + 1, g_state.grid.width);
    int endY = std::min(camTileY + VIEW_TILES_H + 1, g_state.grid.height);

    for (int y = camTileY; y < endY; ++y) {
        for (int x = camTileX; x < endX; ++x) {
            int screenX = (x - g_camera.x) * TILE_SIZE;
            int screenY = (y - g_camera.y) * TILE_SIZE;

            if (!g_state.grid.in_play_area(x, y)) {
                gfx_fillRect(screenX, screenY, TILE_SIZE, TILE_SIZE, 0x8410);
                continue;
            }

            draw_cell(screenX, screenY, g_state.grid.cell(x, y));
        }
    }
}



} // namespace baba
