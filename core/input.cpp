/*
===============================================================================
  input.cpp — Gestion des entrées (boutons + joystick)
-------------------------------------------------------------------------------
  Rôle :
    - Lire l’état brut des touches via l’expander.
    - Détecter les pressions / relâchements.
    - Mapper les bits vers des booléens lisibles (A, B, UP, MENU…).
    - Lire les axes du joystick analogique.

  Notes :
    - Le module met à jour g_keys, accessible globalement.
    - input_poll() doit être appelé à chaque frame.
    - isLongPress() détecte une pression longue (~1 seconde).

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#include "input.h"
#include "gb_core.h"

// gb_core global défini dans app_main.cpp
extern gb_core g_core;

Keys g_keys;
static uint16_t prev_raw = 0;

void input_init()
{
    prev_raw = 0;
    g_keys = {};
}

void input_poll(Keys& k)
{
    // Lecture brute via lib AKA
	g_core.buttons.update();

    uint16_t raw = g_core.buttons.pressed() | g_core.buttons.state();
    k.raw = raw;

    // Transitions
    k.pressed  = raw & ~prev_raw;
    k.released = prev_raw & ~raw;
    prev_raw   = raw;

    // Mapping boutons (adapté à tes besoins)
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

    // Joystick analogique
    k.joxx = g_core.joystick.get_x();
    k.joxy = g_core.joystick.get_y();

    // Joystick numérique (-1, 0, +1) avec seuils arbitraires
    const int THRESH = 500;
    if (k.joxx < -THRESH)       k.joyX = -1;
    else if (k.joxx > THRESH)   k.joyX = +1;
    else                        k.joyX = 0;

    if (k.joxy < -THRESH)       k.joyY = -1;
    else if (k.joxy > THRESH)   k.joyY = +1;
    else                        k.joyY = 0;

    g_keys = k;
}

bool isLongPress(const Keys& k, int key)
{
    static int pressDuration = 0;

    if (k.raw & key)
    {
        pressDuration++;
        if (pressDuration > 60) // ~1s à 60 FPS
        {
            pressDuration = 0;
            return true;
        }
    }
    else
    {
        pressDuration = 0;
    }
    return false;
}

