/*
===============================================================================
  menu.cpp — Menu principal du moteur Baba
-------------------------------------------------------------------------------
  Rôle :
    - Gérer l’écran de titre et le menu principal.
    - Navigation simple :
        * Jouer
        * Options
        * Quitter (retour au titre)
    - Ne contient aucune logique de gameplay ou d’options.
    - S’intègre avec game.cpp (GameMode::Menu) et options.cpp.
===============================================================================
*/

#include "menu.h"
#include "core/graphics.h"
#include "core/input.h"
#include "game/game.h"
#include "options.h"
#include "assets/gfx/title.h"
extern gb_core g_core;

namespace baba {

// -----------------------------------------------------------------------------
//  État interne du menu
// -----------------------------------------------------------------------------
static int cursor = 0;

// -----------------------------------------------------------------------------
//  Initialisation
// -----------------------------------------------------------------------------
void menu_init() {
    cursor = 0;
}

// -----------------------------------------------------------------------------
//  Mise à jour (navigation)
// -----------------------------------------------------------------------------
void menu_update() {

    const int count = 3; // Jouer / Options / Quitter

    if ( g_core.buttons.pressed(gb_buttons::KEY_UP) )
        cursor = (cursor + count - 1) % count;

    if ( g_core.buttons.pressed(gb_buttons::KEY_DOWN) )
        cursor = (cursor + 1) % count;

    if ( g_core.buttons.pressed(gb_buttons::KEY_A) ) {
        if (cursor == 0) {
            // Jouer → charger niveau 0
            game_load_level(0);
            game_mode() = GameMode::Playing;
        }
        else if (cursor == 1) {
            // Options
            game_mode() = GameMode::Options;
        }
        else if (cursor == 2) {
            // Quitter → retour à l’écran de titre
            game_mode() = GameMode::Menu;
        }
    }
}

// -----------------------------------------------------------------------------
//  Dessin du menu principal
// -----------------------------------------------------------------------------
void menu_draw() {
    gfx_clear(COLOR_BLACK);

    // Affiche l’image de titre
    gfx_blit(title_pixels, 320, 240, 0, 0);

    // Menu texte
    const char* items[] = {
        "Jouer",
        "Options",
        "Quitter"
    };

    int y = 160;
    for (int i = 0; i < 3; i++) {
        gfx_text_center(y, items[i], (i == cursor) ? COLOR_YELLOW : COLOR_WHITE);
        y += 20;
    }

    gfx_flush();
}

} // namespace baba
