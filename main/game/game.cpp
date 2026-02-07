/*
===============================================================================
  game.cpp — Moteur principal du jeu Baba
-------------------------------------------------------------------------------
  Rôle :
    - Gérer l’état global du jeu (GameState, GameMode).
    - Charger les niveaux (normaux + custom).
    - Appliquer le pipeline complet :
         * rules_parse()
         * transformations
         * step() (MOVE auto + YOU + interactions)
    - Gérer la caméra centrée sur YOU + joystick libre.
    - Gérer les transitions (fade-in / fade-out).
    - Gérer la musique (forcedMusic + musique par niveau).
    - Gérer le toggle son global (touche D).
    - Dessiner la grille.
    - Fournir l’écran de titre (GameMode::Menu).
    - Déléguer le menu OPTIONS à options.cpp (GameMode::Options).

  Notes :
    - Ce fichier ne contient plus aucun menu interne.
    - Ce fichier ne contient plus aucune logique filesystem interne.
    - Ce fichier est désormais un moteur pur.
===============================================================================
*/

#include "game.h"
#include "core/movement.h"
#include "core/sprites.h"
#include "core/audio.h"
#include "core/input.h"
#include "core/graphics.h"
#include "core/filesystem.h"
#include "core/types.h"

#include "game/levels.h"
#include "game/config.h"
#include "game/options.h"
#include "game/music_map.h"
#include "game/menu.h"

#include "assets/gfx/title.h"

#include <algorithm>
#include <vector>
#include <cmath>

#include <gamebuino.h>
extern gb_core g_core;

namespace baba {

// ============================================================================
//  ÉTAT GLOBAL DU JEU
// ============================================================================
GameState g_state;
static GameMode  g_mode = GameMode::Menu;

bool soundEnabled = true;

static MusicID g_forcedMusic = MusicID::NONE;
static bool    g_forceMusicAcrossLevels = false;

std::vector<GameSnapshot> g_undoStack;

GameState& game_state() { return g_state; }
GameMode&  game_mode()  { return g_mode; }

void load_config();
void save_config();

void game_set_forced_music(MusicID id, bool persistAcrossLevels)
{
    g_forcedMusic = id;
    g_forceMusicAcrossLevels = persistAcrossLevels;
}

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

struct Point { int x; int y; };
static int g_selectedYou = 0;

// ============================================================================
//  Zoom (Q8.8) — niveaux : 20, 15, 10, 8, 6 tiles visibles
// ============================================================================
static const int ZOOM_FP_LEVELS[] = {
    256, // 1.0  → 20 tiles
    341, // 1.33 → 15 tiles
    512, // 2.0  → 10 tiles
    640, // 2.5  → 8 tiles
    853  // 3.33 → 6 tiles
};
static constexpr int ZOOM_LEVEL_COUNT = sizeof(ZOOM_FP_LEVELS) / sizeof(int);

static int g_zoomIndex = 0;
static int g_zoomFp    = ZOOM_FP_LEVELS[0];

static int view_tiles_w()
{
    int tiles = (SCREEN_W << 8) / (TILE_SIZE * g_zoomFp);
    return tiles > 0 ? tiles : 1;
}

static int view_tiles_h()
{
    int tiles = (SCREEN_H << 8) / (TILE_SIZE * g_zoomFp);
    return tiles > 0 ? tiles : 1;
}

// ============================================================================
//  YOU detection
// ============================================================================
static std::vector<Point> find_all_you(const Grid& g, const PropertyTable& props) {
    std::vector<Point> out;

    for (int y = 0; y < g.height; ++y)
        for (int x = 0; x < g.width; ++x)
            for (const auto& obj : g.cell(x, y).objects)
                if (props[(int)obj.type].you)
                    out.push_back({x, y});

    return out;
}

static Point compute_camera_target(const Grid& g, const PropertyTable& props) {
    auto yous = find_all_you(g, props);

    if (yous.empty())
        return {g.width / 2, g.height / 2};

    if (g_selectedYou < (int)yous.size())
        return yous[g_selectedYou];

    float sx = 0, sy = 0;
    for (auto& p : yous) { sx += p.x; sy += p.y; }
    return {int(sx / yous.size()), int(sy / yous.size())};
}

// ============================================================================
//  Mise à jour caméra (compatible zoom)
// ============================================================================
static void update_camera(const Grid& g, const PropertyTable& props,
                          int joyX, int joyY)
{
    const int vw = view_tiles_w();
    const int vh = view_tiles_h();

    float camX = g_camera.x;
    float camY = g_camera.y;

    // Déplacement fluide (tiles / seconde)
    const float camSpeed = 5.0f;
    const float dt = 1.0f / 60.0f;

    // IMPORTANT : pas d’inversion verticale
    camX += joyX * camSpeed * dt;
    camY += joyY * camSpeed * dt;

    // Limites du niveau
    float levelW = g.playMaxX - g.playMinX + 1;
    float levelH = g.playMaxY - g.playMinY + 1;

	// Clamp horizontal
	if (levelW <= vw) {
		camX = g.playMinX - (vw - levelW) * 0.5f;
	} else {
		camX = std::clamp(camX,
						  (float)g.playMinX,
						  (float)(g.playMaxX - vw + 1));
	}

	// Clamp vertical
	if (levelH <= vh) {
		camY = g.playMinY - (vh - levelH) * 0.5f;
	} else {
		camY = std::clamp(camY,
						  (float)g.playMinY,
						  (float)(g.playMaxY - vh + 1));
}

// --- Correction du décalage : alignement sur des tiles entiers ---
camX = floorf(camX + 0.5f);
camY = floorf(camY + 0.5f);

// Application
g_camera.x = camX;
g_camera.y = camY;

}


// ============================================================================
//  Transitions
// ============================================================================
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
//  Structure dossiers + niveaux custom
// ============================================================================
static const char* CUSTOM_SLOTS[3] = {
    "/babaisyou/levels/custom1.txt",
    "/babaisyou/levels/custom2.txt",
    "/babaisyou/levels/custom3.txt"
};

void ensure_custom_level_structure()
{
    if (!fs_exists("/babaisyou"))
        fs_mkdir("/babaisyou");

    if (!fs_exists("/babaisyou/levels"))
        fs_mkdir("/babaisyou/levels");

    const char* paths[3] = {
        CUSTOM_SLOTS[0],
        CUSTOM_SLOTS[1],
        CUSTOM_SLOTS[2]
    };

    const LevelInfo* defaults[3] = {
        &levels[0],
        &levels[1],
        &levels[2]
    };

    for (int i = 0; i < 3; ++i) {
        if (!fs_exists(paths[i])) {

            std::string out;

            for (int y = 0; y < defaults[i]->height; ++y) {
                for (int x = 0; x < defaults[i]->width; ++x) {

                    uint8_t raw = defaults[i]->data[y * defaults[i]->width + x];
                    ObjectType obj = static_cast<ObjectType>(raw);

                    out += object_type_to_text(obj);

                    if (x + 1 < defaults[i]->width)
                        out += ", ";
                }
                out += "\n";
            }

            fs_write_text(paths[i], out.c_str());
        }
    }
}

// ============================================================================
//  INITIALISATION
// ============================================================================
void game_init() {
    ensure_custom_level_structure();
    load_config();

    g_state = GameState{};
    sprites_init();

    g_mode = GameMode::Menu;
    menu_init();
}

// ============================================================================
//  CHARGEMENT DE NIVEAU
// ============================================================================
void game_load_level(int index)
{
    g_undoStack.clear();

    g_state.currentLevel = index;
    g_state.hasWon = false;
    g_state.hasDied = false;

    if (index <= -1 && index >= -3) {
        int slot = -1 - index;
        std::string text;

        if (fs_read_text(CUSTOM_SLOTS[slot], text)) {
            load_level_from_text(text.c_str(), g_state.grid);
        } else {
            load_level(0, g_state.grid);
        }
    } else {
        load_level(index, g_state.grid);
    }

    rules_parse(g_state.grid, g_state.props, g_state.transforms);
    apply_transformations(g_state.grid, g_state.transforms);

    g_camera = Camera{};
    g_selectedYou = 0;

    MusicID music = MusicID::NONE;

    if (g_forcedMusic != MusicID::NONE) {
        music = g_forcedMusic;

        if (!g_forceMusicAcrossLevels)
            g_forcedMusic = MusicID::NONE;

    } else {
        music = getMusicForLevel(index);
    }

    audio_request_music(music);

    update_camera(g_state.grid, g_state.props, 0, 0);
}

// ============================================================================
//  ÉCRAN DE TITRE
// ============================================================================
void game_show_title()
{
    gfx_clear(COLOR_BLACK);
    gfx_blit(title_pixels, 320, 240, 0, 0);
    gfx_flush();
}

// ============================================================================
//  game_update()
// ============================================================================
void game_update() {

    // ---------------------------------------------------------------------
    //  Modes spéciaux : Menu / Options
    // ---------------------------------------------------------------------
    if (g_mode == GameMode::Options) {
        options_update();
        return;
    }

    if (g_mode == GameMode::Menu) {
        menu_update();
        return;
    }

    // ---------------------------------------------------------------------
    //  Réinitialisation des flags (juste avant un éventuel déplacement)
    // ---------------------------------------------------------------------
    g_state.hasWon  = false;
    g_state.hasDied = false;

    // ---------------------------------------------------------------------
    //  Zoom
    // ---------------------------------------------------------------------
    if (g_core.buttons.pressed(gb_buttons::KEY_L1)) {
        if (g_zoomIndex > 0) {
            g_zoomIndex--;
            g_zoomFp = ZOOM_FP_LEVELS[g_zoomIndex];
        }
    }

    if (g_core.buttons.pressed(gb_buttons::KEY_R1)) {
        if (g_zoomIndex + 1 < ZOOM_LEVEL_COUNT) {
            g_zoomIndex++;
            g_zoomFp = ZOOM_FP_LEVELS[g_zoomIndex];
        }
    }

    // ---------------------------------------------------------------------
    //  Bouton D : recentrer ou changer de YOU
    // ---------------------------------------------------------------------
    if (g_core.buttons.pressed(gb_buttons::KEY_D)) {

        Point target = compute_camera_target(g_state.grid, g_state.props);

        const int vw = view_tiles_w();
        const int vh = view_tiles_h();

        float idealX = target.x - vw * 0.5f;
        float idealY = target.y - vh * 0.5f;

        if (fabsf(g_camera.x - idealX) > 0.1f ||
            fabsf(g_camera.y - idealY) > 0.1f)
        {
            g_camera.x = idealX;
            g_camera.y = idealY;
        }
        else {
            auto yous = find_all_you(g_state.grid, g_state.props);
            if (!yous.empty()) {
                g_selectedYou = (g_selectedYou + 1) % yous.size();
            }
        }
    }

    // ---------------------------------------------------------------------
    //  UNDO (C)
    // ---------------------------------------------------------------------
    if (g_core.buttons.pressed(gb_buttons::KEY_C) && !g_undoStack.empty()) {

        const GameSnapshot& snap = g_undoStack.back();
        g_state.grid       = snap.grid;
        g_state.props      = snap.props;
        g_state.transforms = snap.transforms;
        g_undoStack.pop_back();

        rules_parse(g_state.grid, g_state.props, g_state.transforms);
        apply_transformations(g_state.grid, g_state.transforms);

        return;
    }

    // ---------------------------------------------------------------------
    //  Pipeline règles AVANT déplacement
    // ---------------------------------------------------------------------
    rules_parse(g_state.grid, g_state.props, g_state.transforms);
    apply_transformations(g_state.grid, g_state.transforms);

    // ---------------------------------------------------------------------
    //  Déplacement du joueur
    // ---------------------------------------------------------------------
    int dx = 0, dy = 0;

    if (g_core.buttons.pressed(gb_buttons::KEY_LEFT))  dx = -1;
    if (g_core.buttons.pressed(gb_buttons::KEY_RIGHT)) dx = +1;
    if (g_core.buttons.pressed(gb_buttons::KEY_UP))    dy = -1;
    if (g_core.buttons.pressed(gb_buttons::KEY_DOWN))  dy = +1;

    if (dx != 0 || dy != 0) {

        // --- Sauvegarde pour UNDO ---
        if (g_undoStack.size() >= MAX_UNDO)
            g_undoStack.erase(g_undoStack.begin());

        g_undoStack.push_back({
            g_state.grid,
            g_state.props,
            g_state.transforms
        });

        // --- Déplacement ---
        MoveResult r = step(g_state.grid, g_state.props, g_state.transforms, dx, dy);

        g_state.hasWon  = r.hasWon;
        g_state.hasDied = r.hasDied;

        // --- Victoire ---
        if (g_state.hasWon) {
            g_mode = GameMode::Win;
            return;
        }

        // --- Mort (tous les YOU détruits) ---
        if (g_state.hasDied) {
            g_mode = GameMode::Dead;
            return;
        }

        // --- Pipeline règles APRÈS déplacement ---
        rules_parse(g_state.grid, g_state.props, g_state.transforms);
        apply_transformations(g_state.grid, g_state.transforms);
    }

    // ---------------------------------------------------------------------
    //  Joystick → déplacement caméra
    // ---------------------------------------------------------------------
    int rawX = g_core.joystick.get_x();
    int rawY = g_core.joystick.get_y();

    const int dead = 25;

    int joyX = 0;
    int joyY = 0;

    if (rawX >  dead) joyX = +1;
    if (rawX < -dead) joyX = -1;
    if (rawY >  dead) joyY = +1;
    if (rawY < -dead) joyY = -1;

    update_camera(g_state.grid, g_state.props, joyX, joyY);
}


// ============================================================================
//  Helpers progression
// ============================================================================
void game_win_continue() 
{ 
    int next = g_state.currentLevel + 1; 
    if (next >= levels_count()) next = 0;
    game_load_level(next); 
} 

void game_restart_after_death() 
{ 
    game_load_level(g_state.currentLevel); 
}

// ============================================================================
//  DESSIN (inchangé pour l’instant — patch final après graphics.cpp)
// ============================================================================
void game_draw() {
    
    if (g_mode == GameMode::Menu) {
        menu_draw();
        return;
    }

    if (g_mode == GameMode::Options) {
        return;
    }

    gfx_clear(COLOR_BLACK);

    // -------------------------------------------------------------------------
    // 1) Calcul du tile en pixels (zoom Q8.8)
    // -------------------------------------------------------------------------
    const int tile_px = (TILE_SIZE * g_zoomFp) >> 8;

    // -------------------------------------------------------------------------
    // 2) Zone visible en tiles (dépend du zoom)
    // -------------------------------------------------------------------------
    const int vw = view_tiles_w();
    const int vh = view_tiles_h();

    int camTileX = (int)g_camera.x;
    int camTileY = (int)g_camera.y;

    int endX = std::min(camTileX + vw + 1, g_state.grid.width);
    int endY = std::min(camTileY + vh + 1, g_state.grid.height);

    // -------------------------------------------------------------------------
    // 3) Dessin de la grille
    // -------------------------------------------------------------------------
    for (int y = camTileY; y < endY; ++y) {
        for (int x = camTileX; x < endX; ++x) {

            // Projection monde → écran
            int screenX = ((x - g_camera.x) * tile_px);
            int screenY = ((y - g_camera.y) * tile_px);

            // Clipping rapide
            if (screenX >= SCREEN_W || screenX + tile_px <= 0 ||
                screenY >= SCREEN_H || screenY + tile_px <= 0)
                continue;

            // Zone hors play area
            if (!g_state.grid.in_play_area(x, y)) {
                gfx_fillRect(screenX, screenY, tile_px, tile_px, 0x7BEF);
                continue;
            }

            // -----------------------------------------------------------------
            // 4) Choix entre version normale et version zoomée
            // -----------------------------------------------------------------
            if (g_zoomFp == 256) {
                // Zoom 1.0 → version originale
                draw_cell(screenX, screenY, g_state.grid.cell(x, y), g_state.props);
            } else {
                // Zoom ≠ 1.0 → version zoomée
                draw_cell_scaled(screenX, screenY, g_state.grid.cell(x, y),
                                 g_state.props, g_zoomFp);
            }
        }
    }

    // -------------------------------------------------------------------------
    // 5) Remplissage des zones hors grille (droite / bas)
    // -------------------------------------------------------------------------
    int drawnWidth  = (endX - camTileX) * tile_px;
    int drawnHeight = (endY - camTileY) * tile_px;

    if (drawnWidth < SCREEN_W) {
        gfx_fillRect(drawnWidth, 0, SCREEN_W - drawnWidth, SCREEN_H, 0x7BEF);
    }

    if (drawnHeight < SCREEN_H) {
        gfx_fillRect(0, drawnHeight, SCREEN_W, SCREEN_H - drawnHeight, 0x7BEF);
    }

    gfx_flush();
}


} // namespace baba
