/*
===============================================================================
  task_game.cpp — Boucle de jeu principale (40 FPS)
-------------------------------------------------------------------------------
  Rôle :
    - Exécuter la logique du jeu selon l’état courant (GameMode).
    - Appeler :
        * game_show_title()   → écran de titre
        * game_update()       → logique de déplacement + règles
        * game_draw()         → rendu de la grille
    - Gérer les transitions :
        * Title → Playing
        * Playing → Win / Dead
        * Win / Dead → Restart
        * Menu → retour vers Playing
    - Maintenir une cadence stable (~40 FPS).
===============================================================================
*/

#include <cstdio>
#include <cstring>
#include "task_game.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "core/input.h"
#include "core/graphics.h"
#include "core/audio.h"

#include "game/game.h"
#include "game/config.h"
#include "game/options.h"
extern gb_core g_core;
namespace baba
{

// -----------------------------------------------------------------------------
//  Temps global du moteur (utilisé par grid.cpp pour l’effet WIN)
// -----------------------------------------------------------------------------
float g_time = 0.0f;

// -----------------------------------------------------------------------------
//  Variables internes
// -----------------------------------------------------------------------------
static GameMode s_prevMode = GameMode::Title;

// Cooldown pour les déplacements (en ms)
static uint32_t s_lastMoveTimeMs = 0;
static constexpr uint32_t MOVE_DELAY_MS = 120;

// -----------------------------------------------------------------------------
//  Actions à effectuer lors de l’entrée dans un nouvel état
// -----------------------------------------------------------------------------
static void on_enter_mode(GameMode m)
{
    switch (m)
    {
    case GameMode::Title:
        gfx_clear(COLOR_BLACK);
        game_show_title();
        gfx_text_center(200, "Press A to start", COLOR_WHITE);
        gfx_flush();
        break;

    case GameMode::Playing:
        gfx_clear(COLOR_BLACK);
        gfx_flush();
        break;

    case GameMode::Win:
        break;

    case GameMode::Dead:
        break;

    case GameMode::Menu:
        break;

    case GameMode::Options:
        // rien à faire ici : options_update() gère l'affichage
        break;
    }
}

// -----------------------------------------------------------------------------
//  Tâche de jeu principale (cadence ~40 FPS)
// -----------------------------------------------------------------------------
void task_game(void *)
{
    printf("[GameTask] Démarrage de la boucle de jeu.\n");

    game_init();

    game_mode() = GameMode::Title;
    on_enter_mode(game_mode());
    s_prevMode = game_mode();

    while (true)
    {
        g_core.pool();

        g_time += 0.025f;

        if (game_mode() != s_prevMode)
        {
            on_enter_mode(game_mode());
            s_prevMode = game_mode();
        }

        switch (game_mode())
        {
        case GameMode::Title:
            if ( g_core.buttons.pressed(gb_buttons::KEY_A)  )
            {
                game_load_level(0);
                game_mode() = GameMode::Playing;
            }
            break;

        case GameMode::Playing:
        {
            uint32_t nowMs = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);


            if (true)
            {
                game_update();
                s_lastMoveTimeMs = nowMs;
            }

            if (game_state().hasWon)
            {
                game_mode() = GameMode::Win;
                break;
            }
			
            if (game_state().hasDied)
            {
                game_mode() = GameMode::Dead;
                break;
            }

            if ( g_core.buttons.pressed(gb_buttons::KEY_MENU)  )
            {
                fade_out();
                game_mode() = GameMode::Menu;
                break;
            }
            break;
        }

        case GameMode::Win:
            // inchangé
            // ...
            break;

		case GameMode::Dead:
		{
			// =========================================================================
			//  ÉTAT : Dead
			// -------------------------------------------------------------------------
			//  Ce mode est activé lorsque game_update() détecte une mort (hasDied = true).
			//  La logique de mort NE DOIT PAS être dans game_update() : c’est ici, dans
			//  la machine à états, que l’on gère les transitions visuelles et temporelles.
			//
			//  Pipeline :
			//      1) Afficher un message de mort
			//      2) Petite pause (freeze dramatique)
			//      3) Recharger le niveau courant (game_load_level)
			//      4) Retour en mode Playing
			//
			//  Note :
			//      game_load_level() vide déjà le rollback → comportement fidèle à Baba.
			// =========================================================================

			// --- 1) Écran de mort ---
			gfx_clear(COLOR_BLACK);
			gfx_text_center(SCREEN_H/2 - 10, "BABA EST MORT", COLOR_WHITE);
			gfx_text_center(SCREEN_H/2 + 10, "Redemarrage...", COLOR_WHITE);
			gfx_flush();

			// --- 2) Petite pause (freeze) ---
			//     400–600 ms donne un bon ressenti. Ici : 500 ms.
			vTaskDelay(pdMS_TO_TICKS(500));

			// --- 3) Recharger le niveau courant ---
			//     Cela réinitialise la grille, les règles, la caméra, la musique,
			//     et vide le rollback (déjà géré dans game_load_level()).
			//     On verra plus tard si on autorise le roll back avant, dans ce cas.
			game_load_level(game_state().currentLevel);

			// --- 4) Retour au gameplay ---
			game_mode() = GameMode::Playing;
			break;
		}


        case GameMode::Menu:
            options_update();
            break;

        case GameMode::Options:
            options_update();
            break;
        }

        if (game_mode() == GameMode::Playing)
        {
            game_draw();
            gfx_flush();
        }

    }
}

} // namespace baba
	