/*
===============================================================================
  input.cpp — Gestion des entrées (boutons + joystick)
-------------------------------------------------------------------------------
  Rôle :
    - Lire l’état brut des touches via l’expander.
    - Détecter pressions / relâchements.
    - Mapper les bits vers des booléens lisibles.
    - Lire les axes du joystick analogique.
    - Fournir des helpers (long press).

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#include "core/input.h"
#include "gb_core.h"
#include "gb_common.h"
#include "esp_timer.h"
#include <stdint.h>
#include <chrono>
#include <gamebuino.h>
// gb_core global défini dans gb_core.cpp
extern gb_core g_core;


// Dernier état brut (pour transitions)
//static uint16_t prev_raw = 0;

// Compteurs pour pressions longues (un par bit)
static int longPressCounter[16] = {};

// -----------------------------------------------------------------------------
// Helper front montant pour B (optionnel mais pratique)
// -----------------------------------------------------------------------------

// ============================================================================
//  Anti-repeat
// ============================================================================

static uint32_t lastInputTime = 0;
static const uint32_t INPUT_COOLDOWN_MS = 120;

static uint32_t get_time_ms() {
    return esp_timer_get_time() / 1000;
}

bool input_ready() {
    return true;
}


// -----------------------------------------------------------------------------
// Initialisation
// -----------------------------------------------------------------------------
void input_init()
{
//    prev_raw = 0;
    for (int& c : longPressCounter)
        c = 0;
}





// -----------------------------------------------------------------------------
// Détection pression longue (~1 seconde à 60 FPS)
// -----------------------------------------------------------------------------
bool isLongPress( gb_buttons k, int key)
{
    // Convertit le bit en index 0..15
    int idx = __builtin_ctz(key);

    if ( g_core.buttons.state()  & key)
    {
        if (++longPressCounter[idx] > 60)
        {
            longPressCounter[idx] = 0;
            return true;
        }
    }
    else
    {
        longPressCounter[idx] = 0;
    }

    return false;
}
