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

namespace baba {

// ============================================================================
//  ÉTAT GLOBAL DU JEU
// ============================================================================
GameState g_state;
static GameMode  g_mode = GameMode::Menu;

// Son global ON/OFF (toggle avec D en gameplay)
bool soundEnabled = true;

// Musique forcée (choisie dans options.cpp)
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

constexpr int VIEW_TILES_W  = SCREEN_W / TILE_SIZE;
constexpr int VIEW_TILES_H  = SCREEN_H / TILE_SIZE;

struct Point { int x; int y; };
static int g_selectedYou = 0;

// Trouve toutes les positions YOU
static std::vector<Point> find_all_you(const Grid& g, const PropertyTable& props) {
    std::vector<Point> out;

    for (int y = 0; y < g.height; ++y)
        for (int x = 0; x < g.width; ++x)
            for (const auto& obj : g.cell(x, y).objects)
                if (props[(int)obj.type].you)
                    out.push_back({x, y});

    return out;
}

// Cible caméra = YOU sélectionné ou centre de masse
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

// Mise à jour caméra
static void update_camera(const Grid& g, const PropertyTable& props, int joyX, int joyY)
{
    Point target = compute_camera_target(g, props);

    float idealX = target.x - (VIEW_TILES_W / 2.0f);
    float idealY = target.y - (VIEW_TILES_H / 2.0f);

    const float joyFactor = 0.3f;
    g_camera.offsetX += joyX * joyFactor;
    g_camera.offsetY += joyY * joyFactor;

    float camX = idealX + g_camera.offsetX;
    float camY = idealY + g_camera.offsetY;

    float levelW = g.playMaxX - g.playMinX + 1;
    float levelH = g.playMaxY - g.playMinY + 1;

    // Clamp horizontal
    if (levelW < VIEW_TILES_W) {
        camX = g.playMinX - (VIEW_TILES_W - levelW) * 0.5f;
    } else {
        camX = std::clamp(camX,
                          (float)g.playMinX,
                          (float)(g.playMaxX - VIEW_TILES_W + 1));
    }

    // Clamp vertical
    if (levelH < VIEW_TILES_H) {
        camY = g.playMinY - (VIEW_TILES_H - levelH) * 0.5f;
    } else {
        camY = std::clamp(camY,
                          (float)g.playMinY,
                          (float)(g.playMaxY - VIEW_TILES_H + 1));
    }

	// Forcer la caméra à être alignée sur la grille
	g_camera.x = static_cast<int>(camX);
	g_camera.y = static_cast<int>(camY);

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

					// --- Récupération brute depuis levels_data (uint8_t) ---
					uint8_t raw = defaults[i]->data[y * defaults[i]->width + x];

					// --- Conversion propre vers ObjectType ---
					ObjectType obj = static_cast<ObjectType>(raw);

					// --- Conversion ObjectType → texte ---
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
    g_state.currentLevel = index;
    g_state.hasWon = false;
    g_state.hasDied = false;

    // -------------------------------------------------------------------------
    // 1) Charger la grille (niveaux normaux ou custom)
    // -------------------------------------------------------------------------
    if (index <= -1 && index >= -3) {
        // Niveaux custom : index = -1, -2, -3 → slots 0,1,2
        int slot = -1 - index;
        std::string text;

        if (fs_read_text(CUSTOM_SLOTS[slot], text)) {
            // Charge depuis texte
            load_level_from_text(text.c_str(), g_state.grid);
        } else {
            // Slot vide → fallback sur niveau 0
            load_level(0, g_state.grid);
        }
    } else {
        // Niveau normal
        load_level(index, g_state.grid);
    }

    // -------------------------------------------------------------------------
    // 2) Recalculer les règles + transformations
    // -------------------------------------------------------------------------
    rules_parse(g_state.grid, g_state.props, g_state.transforms);
    apply_transformations(g_state.grid, g_state.transforms);

    // -------------------------------------------------------------------------
    // 3) Réinitialiser la caméra et la sélection
    // -------------------------------------------------------------------------
    g_camera = Camera{};
    g_selectedYou = 0;

    // -------------------------------------------------------------------------
    // 4) Gestion de la musique
    // -------------------------------------------------------------------------
    MusicID music = MusicID::NONE;

    if (g_forcedMusic != MusicID::NONE) {
        // Musique forcée via options
        music = g_forcedMusic;

        if (!g_forceMusicAcrossLevels)
            g_forcedMusic = MusicID::NONE;

    } else {
        // Musique par défaut du niveau
        music = getMusicForLevel(index);
    }

    audio_request_music(music);

    // -------------------------------------------------------------------------
    // 5) Mise à jour caméra initiale
    // -------------------------------------------------------------------------
    update_camera(g_state.grid, g_state.props, 0, 0);
}


// ============================================================================
//  ÉCRAN DE TITRE (GameMode::Menu)
// ============================================================================
void game_show_title()
{
    gfx_clear(COLOR_BLACK);
    gfx_blit(title_pixels, 320, 240, 0, 0);
    gfx_flush();
}

// ============================================================================
//  game_update() — uniquement gameplay + redirection vers options
// ============================================================================
void game_update() {

    if (g_mode == GameMode::Options) {
        options_update();
        return;
    }

    if (g_mode == GameMode::Menu) {
        menu_update();
        return;
    }

    g_state.hasWon  = false;
    g_state.hasDied = false;

    // --- Toggle son global (D) ---
    if (g_keys.D) {
        printf("D DETECTED\n");
        soundEnabled = !soundEnabled;
        options_save();

        if (soundEnabled) {
            audio_set_music_volume(g_audio_settings.music_volume);
            audio_set_sfx_volume(g_audio_settings.sfx_volume);
        } else {
            audio_set_music_volume(0);
            audio_set_sfx_volume(0);
        }
    }

    // --- UNDO (C) ---
    if (g_keys.C && !g_undoStack.empty()) {
        printf("C DETECTED IN\n");

        const GameSnapshot& snap = g_undoStack.back();
        g_state.grid       = snap.grid;
        g_state.props      = snap.props;
        g_state.transforms = snap.transforms;
        g_undoStack.pop_back();

        rules_parse(g_state.grid, g_state.props, g_state.transforms);
        apply_transformations(g_state.grid, g_state.transforms);

        return;
    }

    // --- Toujours recalculer les règles AVANT mouvement ---
    rules_parse(g_state.grid, g_state.props, g_state.transforms);
    apply_transformations(g_state.grid, g_state.transforms);

    // --- Lecture du mouvement (sans else-if) ---
    int dx = 0, dy = 0;

    if (g_keys.left)  dx = -1;
    if (g_keys.right) dx = +1;
    if (g_keys.up)    dy = -1;
    if (g_keys.down)  dy = +1;

    // --- Mouvement ---
    if (dx != 0 || dy != 0) {

        // Sauvegarde UNDO
        if (g_undoStack.size() >= MAX_UNDO)
            g_undoStack.erase(g_undoStack.begin());

        g_undoStack.push_back({
            g_state.grid,
            g_state.props,
            g_state.transforms
        });

        MoveResult r = step(g_state.grid, g_state.props, g_state.transforms, dx, dy);

        g_state.hasWon  = r.hasWon;
        g_state.hasDied = r.hasDied;

        if (g_state.hasWon) {
            game_win_continue();
            return;
        }

        if (g_state.hasDied) {
            game_restart_after_death();
            return;
        }

        // Recalcul des règles après mouvement
        rules_parse(g_state.grid, g_state.props, g_state.transforms);
        apply_transformations(g_state.grid, g_state.transforms);
    }

    update_camera(g_state.grid, g_state.props, g_keys.joyX, g_keys.joyY);
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
//  DESSIN
// ============================================================================
void game_draw() {

    if (g_mode == GameMode::Menu) {
        menu_draw();
        return;
    }

    if (g_mode == GameMode::Options) {
        return;
    }

    // Fond noir rapide
    gfx_clear(COLOR_BLACK);

    int camTileX = (int)g_camera.x;
    int camTileY = (int)g_camera.y;

    int endX = std::min(camTileX + VIEW_TILES_W + 1, g_state.grid.width);
    int endY = std::min(camTileY + VIEW_TILES_H + 1, g_state.grid.height);

    // Dessin de la grille
    for (int y = camTileY; y < endY; ++y) {
        for (int x = camTileX; x < endX; ++x) {

            int screenX = (int)((x - g_camera.x) * TILE_SIZE);
            int screenY = (int)((y - g_camera.y) * TILE_SIZE);

            if (screenX >= SCREEN_W || screenX + TILE_SIZE <= 0 ||
                screenY >= SCREEN_H || screenY + TILE_SIZE <= 0)
                continue;

            if (!g_state.grid.in_play_area(x, y)) {
                gfx_fillRect(screenX, screenY, TILE_SIZE, TILE_SIZE, 0x7BEF);
                continue;
            }

            draw_cell(screenX, screenY, g_state.grid.cell(x, y), g_state.props);
        }
    }

    // Remplir la zone à droite si la grille ne couvre pas toute la largeur
    int drawnWidth = (endX - camTileX) * TILE_SIZE;
    if (drawnWidth < SCREEN_W) {
        gfx_fillRect(drawnWidth, 0, SCREEN_W - drawnWidth, SCREEN_H, 0x7BEF);
    }

    // Remplir la zone en bas si la grille ne couvre pas toute la hauteur
    int drawnHeight = (endY - camTileY) * TILE_SIZE;
    if (drawnHeight < SCREEN_H) {
        gfx_fillRect(0, drawnHeight, SCREEN_W, SCREEN_H - drawnHeight, 0x7BEF);
    }

    gfx_flush();
}

} // namespace baba
