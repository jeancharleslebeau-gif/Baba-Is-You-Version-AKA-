/*
===============================================================================
  grid.cpp — Implémentation de la grille de jeu
-------------------------------------------------------------------------------
  Rôle :
    - Fournir les fonctions d’accès aux cellules.
    - Garantir la validité des coordonnées.
    - Initialiser une grille vide (w×h).
    - Servir de base au moteur Baba Is You :
        * rules.cpp (analyse des phrases)
        * movement.cpp (déplacements)
        * game.cpp (gestion du niveau)

  Notes :
    - La grille ne contient aucune logique de gameplay.
    - Chaque cellule contient une pile d’objets (vector<Object>).
    - Le moteur peut donc empiler plusieurs objets dans une même case.

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#include "grid.h"
#include "assets/gfx/atlas.h"
#include "graphics.h"
#include "sprites.h"

namespace baba {

// -----------------------------------------------------------------------------
//  Constructeur : crée une grille w×h avec des cellules vides
// -----------------------------------------------------------------------------
Grid::Grid(int w, int h)
    : width(w), height(h), cells(w * h)
{
    // Rien d’autre à faire : chaque Cell contient un vector<Object> vide.
}

// -----------------------------------------------------------------------------
//  Vérifie si (x,y) est dans les limites de la grille
// -----------------------------------------------------------------------------
bool Grid::in_bounds(int x, int y) const
{
    return x >= 0 && x < width &&
           y >= 0 && y < height;
}

// -----------------------------------------------------------------------------
//  Accès en écriture à une cellule
// -----------------------------------------------------------------------------
Cell& Grid::cell(int x, int y)
{
    return cells[y * width + x];
}

// -----------------------------------------------------------------------------
//  Accès en lecture à une cellule
// -----------------------------------------------------------------------------
const Cell& Grid::cell(int x, int y) const
{
    return cells[y * width + x];
}

// -----------------------------------------------------------------------------
//  draw_cell() — Dessine une cellule
// -----------------------------------------------------------------------------

void draw_cell(int x, int y, const Cell& c)
{
    for (auto& obj : c.objects) {
        draw_sprite(x, y, obj.type); // appel clair et non ambigu
    }
}

} // namespace baba
