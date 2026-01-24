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

// gb_core global défini dans gb_core.cpp
extern gb_core g_core;

// État global des entrées
Keys g_keys;

// Dernier état brut (pour transitions)
static uint16_t prev_raw = 0;

// Compteurs pour pressions longues (un par bit)
static int longPressCounter[16] = {};


// -----------------------------------------------------------------------------
// Initialisation
// -----------------------------------------------------------------------------
void input_init()
{
    prev_raw = 0;
    g_keys = {};
    for (int& c : longPressCounter)
        c = 0;
}


// -----------------------------------------------------------------------------
// Lecture des entrées (à appeler chaque frame)
// -----------------------------------------------------------------------------
void input_poll(Keys& k)
{
    // 1) Lecture brute via lib AKA
    g_core.buttons.update();
    uint16_t raw = g_core.buttons.state() | g_core.buttons.pressed();
    k.raw = raw;

    // 2) Transitions
    k.pressed  = raw & ~prev_raw;
    k.released = prev_raw & ~raw;
    prev_raw   = raw;

    // 3) Mapping boutons
    k.up    = raw & GB_KEY_UP;
    k.down  = raw & GB_KEY_DOWN;
    k.left  = raw & GB_KEY_LEFT;
    k.right = raw & GB_KEY_RIGHT;

    k.A     = raw & GB_KEY_A;
    k.B     = raw & GB_KEY_B;
    k.C     = raw & GB_KEY_C;
    k.D     = raw & GB_KEY_D;

    k.RUN   = raw & GB_KEY_RUN;
    k.MENU  = raw & GB_KEY_MENU;
    k.R1    = raw & GB_KEY_R1;
    k.L1    = raw & GB_KEY_L1;

    // 4) Joystick analogique
    k.joxx = g_core.joystick.get_x();  // [-2000, +2000]
    k.joxy = g_core.joystick.get_y();

    // 5) Joystick numérique
    constexpr int THRESH = 500;

    k.joyX = (k.joxx < -THRESH) ? -1 :
             (k.joxx >  THRESH) ? +1 : 0;

    k.joyY = (k.joxy < -THRESH) ? -1 :
             (k.joxy >  THRESH) ? +1 : 0;

    // 6) Mise à jour globale
    g_keys = k;
}


// -----------------------------------------------------------------------------
// Détection pression longue (~1 seconde à 60 FPS)
// -----------------------------------------------------------------------------
bool isLongPress(const Keys& k, int key)
{
    // Convertit le bit en index 0..15
    int idx = __builtin_ctz(key);

    if (k.raw & key)
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
