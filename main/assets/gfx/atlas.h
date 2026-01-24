/*
===============================================================================
  atlas.h — Atlas de sprites
-------------------------------------------------------------------------------
  Rôle :
    - Déclarer le tableau global contenant les pixels de l’atlas de sprites.
    - Format : RGB565 (16 bits par pixel).
    - Dimensions : 256 × 64 (4 lignes de 16 sprites 16×16).

  Notes :
    - La définition réelle est dans atlas.cpp.
    - Utilisé par sprites.cpp et graphics.cpp pour le rendu des objets.

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#pragma once
#include <cstdint>

// Tableau global des pixels de l’atlas de sprites (RGB565, 256×64)
extern const uint16_t atlas_pixels[256 * 64];
