/*
===============================================================================
  input.h — Interface du système d’entrée (AKA Edition)
-------------------------------------------------------------------------------
  Rôle :
    - Définir la structure Keys : état complet des entrées utilisateur.
    - Fournir les fonctions d’initialisation et de polling.
    - Offrir des helpers pour pressions longues et répétitions.

  Notes :
    - input_poll() met à jour g_keys à chaque frame.
    - Les champs pressed/released sont valables uniquement pour la frame courante.
    - Les axes joystick analogiques sont exprimés en [-2000, +2000].
    - Les axes joystick numériques sont exprimés en {-1, 0, +1}.

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#pragma once
#include "gb_common.h"
#include <cstdint>

// -----------------------------------------------------------------------------
// Structure Keys : état complet des entrées utilisateur
// -----------------------------------------------------------------------------
struct Keys {
    // État brut (bits venant de l’expander)
    uint32_t raw = 0;

    // Transitions (valables pour la frame courante)
    uint32_t pressed  = 0;   // bits pressés cette frame
    uint32_t released = 0;   // bits relâchés cette frame

    // Joystick analogique (valeurs brutes mappées en [-2000, +2000])
    int joxx = 0;
    int joxy = 0;

    // Boutons numériques (booléens lisibles)
    bool up = false, down = false, left = false, right = false;
    bool A = false, B = false, C = false, D = false;
    bool RUN = false, MENU = false, R1 = false, L1 = false;

    // Joystick numérique (-1, 0, +1)
    int joyX = 0;
    int joyY = 0;
};

// État global des entrées (mis à jour par input_poll)
extern Keys g_keys;

// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------
void input_init();          // Réinitialise l’état des entrées
void input_poll(Keys& k);   // Lit les entrées et met à jour g_keys
bool input_ready();			// anti répétitions


// Helpers
bool isLongPress(const Keys& k, int key);   // Détection pression longue (~1s)
