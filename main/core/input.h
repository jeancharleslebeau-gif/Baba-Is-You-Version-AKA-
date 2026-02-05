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
#include <gamebuino.h>



// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------
void input_init();          // Réinitialise l’état des entrées
void input_poll();   // Lit les entrées et met à jour g_keys
bool input_ready();			// anti répétitions


// Helpers
bool isLongPress(gb_buttons k, int key);   // Détection pression longue (~1s)
