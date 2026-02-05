/*

===============================================================================
  sprites.h — Gestion de l’atlas de sprites 16×16
-------------------------------------------------------------------------------
  Rôle :
    - Associer chaque ObjectType à une tuile dans l’atlas.
    - Fournir sprite_rect_for() pour obtenir la zone source dans l’image.
    - Fournir draw_sprite() pour dessiner un objet unique.
    - La fonction draw_cell() (dans grid.cpp) utilise draw_sprite() pour
      dessiner tous les objets d’une cellule.

  Notes :
    - L’atlas est une image unique (ex : tileset_16x16.png).
    - Chaque sprite fait 16×16 pixels.
    - Le moteur de rendu utilise gfx_drawAtlas() (backend AKA).

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#pragma once
#include "grid.h"

namespace baba {

// Rectangle source dans l’atlas
struct SpriteRect {
    int x, y, w, h;
};

// Initialise la table de correspondance ObjectType → sprite
void sprites_init();

// Accès direct aux pixels de l’atlas (RGB565)
const uint16_t* getAtlasPixels();

// Retourne le rectangle source pour un ObjectType
SpriteRect sprite_rect_for(ObjectType type);

// Dessine un sprite unique 
void draw_sprite(int x, int y, ObjectType t);

} // namespace baba
