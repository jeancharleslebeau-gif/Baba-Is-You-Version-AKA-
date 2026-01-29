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
static Keys s_prevKeys{};

// Cooldown pour les déplacements (en ms)
static uint32_t s_lastMoveTimeMs = 0;
static constexpr uint32_t MOVE_DELAY_MS = 120;

// Détection front montant
static inline bool pressed_A(const Keys &now)     { return now.A    && !s_prevKeys.A; }
static inline bool pressed_MENU(const Keys &now)  { return now.MENU && !s_prevKeys.MENU; }
static inline bool pressed_UP(const Keys &now)    { return now.up    && !s_prevKeys.up; }
static inline bool pressed_DOWN(const Keys &now)  { return now.down  && !s_prevKeys.down; }
static inline bool pressed_LEFT(const Keys &now)  { return now.left  && !s_prevKeys.left; }
static inline bool pressed_RIGHT(const Keys &now) { return now.right && !s_prevKeys.right; }

static inline bool pressed_any_dir(const Keys& now)
{
    return pressed_UP(now) || pressed_DOWN(now) || pressed_LEFT(now) || pressed_RIGHT(now);
}

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

    const TickType_t frame_period = pdMS_TO_TICKS(25); // ~40 FPS
    TickType_t last_wake = xTaskGetTickCount();

    while (true)
    {
        vTaskDelayUntil(&last_wake, frame_period);

        g_time += 0.025f;

        Keys k = g_keys;

        if (game_mode() != s_prevMode)
        {
            on_enter_mode(game_mode());
            s_prevMode = game_mode();
        }

        switch (game_mode())
        {
        case GameMode::Title:
            if (pressed_A(k))
            {
                game_load_level(0);
                game_mode() = GameMode::Playing;
            }
            break;

        case GameMode::Playing:
        {
            uint32_t nowMs = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
            bool dirPressed = pressed_any_dir(k);

            if (dirPressed)
            {
                if (nowMs - s_lastMoveTimeMs >= MOVE_DELAY_MS)
                {
                    game_update();
                    s_lastMoveTimeMs = nowMs;
                }
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

            if (pressed_MENU(k))
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
            // inchangé
            // ...
            break;

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

        s_prevKeys = k;
    }
}

} // namespace baba
