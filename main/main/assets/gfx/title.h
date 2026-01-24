/*
===============================================================================
  title.h — Données graphiques de l’écran titre
-------------------------------------------------------------------------------
  Rôle :
    - Déclarer le tableau global contenant les pixels de l’écran titre.
    - Format : RGB565 (16 bits par pixel).
    - Dimensions : 320 × 240.

  Notes :
    - La définition réelle est dans title.cpp.
    - Utilisé par game_show_title() pour afficher l’écran d’accueil.

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#pragma once
#include <cstdint>

// Tableau global des pixels de l’écran titre (RGB565, 320×240)
extern const uint16_t title_pixels[320 * 240];
