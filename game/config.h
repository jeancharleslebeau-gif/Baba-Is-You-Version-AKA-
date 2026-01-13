/*
===============================================================================
  config.h — Configuration globale du moteur BabaIsU
-------------------------------------------------------------------------------
  Rôle :
    - Définir les constantes de dimensions et de grille.
    - Centraliser les paramètres communs (taille des tiles, écran).
    - Faciliter l’évolution future (changement de résolution, zoom, etc.).
===============================================================================
*/

#pragma once

#ifndef USE_FRAMEBUFFER
#define USE_FRAMEBUFFER 1
#endif


// Dimensions de l’écran
constexpr int SCREEN_W = 320;
constexpr int SCREEN_H = 240;

// Dimensions de la grille
constexpr int TILE_SIZE = 16;                        // taille d’une case
constexpr int GRID_COLS = SCREEN_W / TILE_SIZE;      // nombre de colonnes
constexpr int GRID_ROWS = SCREEN_H / TILE_SIZE;      // nombre de lignes

// Origine de la grille (pixels)
constexpr int GRID_X0 = 0;
constexpr int GRID_Y0 = 0;

// Tolérances d’alignement
constexpr int CENTER_EPS = 2;   // tolérance pour être centré
constexpr int SNAP_EPS   = 3;   // distance sous laquelle on "snap" au centre

// Mode debug (0 = off, 1 = on)
extern int debug;
