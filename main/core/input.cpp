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

// gb_core global défini dans gb_core.cpp
extern gb_core g_core;

// État global des entrées
Keys g_keys;

// Dernier état brut (pour transitions)
static uint16_t prev_raw = 0;

// Compteurs pour pressions longues (un par bit)
static int longPressCounter[16] = {};

// -----------------------------------------------------------------------------
// Helper front montant pour B (optionnel mais pratique)
// -----------------------------------------------------------------------------
static inline bool pressed_B(const Keys& k)
{
    return k.pressed & GB_KEY_B;
}


// ============================================================================
//  Anti-repeat
// ============================================================================

static uint32_t lastInputTime = 0;
static const uint32_t INPUT_COOLDOWN_MS = 120;

static uint32_t get_time_ms() {
    return esp_timer_get_time() / 1000;
}

bool input_ready() {
    uint32_t now = get_time_ms();
    if (now - lastInputTime < INPUT_COOLDOWN_MS)
        return false;
    lastInputTime = now;
    return true;
}


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
	k.joxx = g_core.joystick.get_x();  // [0, 4095]
	k.joxy = g_core.joystick.get_y();  // [0, 4095]

	// 5) Joystick numérique (centré)
	constexpr int CENTER_X = 1915;
	constexpr int CENTER_Y = 1950;
	constexpr int DEADZONE = 400;   // tolérance autour du centre

	int dx = k.joxx - CENTER_X;
	int dy = k.joxy - CENTER_Y;

	// X
	if (dx < -DEADZONE)      k.joyX = -1;
	else if (dx > DEADZONE)  k.joyX = +1;
	else                     k.joyX = 0;

	// Y 
	if (dy < -DEADZONE)      k.joyY = +1;   // bas
	else if (dy > DEADZONE)  k.joyY = -1;   // haut
	else                     k.joyY = 0;

    // 6) Mise à jour globale
    g_keys = k;
	g_keys.raw = k.raw;
	g_keys.A |= k.A;
	g_keys.B |= k.B;
	g_keys.C |= k.C;
	g_keys.D |= k.D;
	g_keys.pressed |= k.pressed;

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
