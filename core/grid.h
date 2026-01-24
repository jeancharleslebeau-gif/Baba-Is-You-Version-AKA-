/*
===============================================================================
  grid.h — Représentation de la grille de jeu (moteur Baba Is You)
-------------------------------------------------------------------------------
  Rôle :
    - Définir la structure de données centrale du moteur : la grille 2D.
    - Chaque cellule peut contenir plusieurs objets (pile d’objets).
    - Fournir des helpers pour accéder aux cellules et vérifier les limites.

  Notes de conception :
    - Le moteur Baba Is You repose sur une logique multi‑objets par case.
      Exemple : une case peut contenir à la fois "BABA", "TEXT_IS", "TEXT_YOU".
    - La grille est volontairement générique : aucune logique de règles ici.
    - Le moteur de règles et le moteur de mouvement utilisent cette structure.

  Extensions prévues :
    - Support d’un système de couches (sol / objets / mots).
    - Support d’un système de z‑index pour le rendu.
    - Support d’un format de sérialisation pour l’éditeur de niveaux.

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#pragma once
#include <vector>
#include <cstdint>
#include "types.h"   // ObjectType, Properties, PropertyTable

namespace baba {

// -----------------------------------------------------------------------------
//  Constantes globales
// -----------------------------------------------------------------------------
constexpr int TILE_SIZE  = 16;
constexpr int MAP_WIDTH  = 32;
constexpr int MAP_HEIGHT = 24;
constexpr int MAP_SIZE   = MAP_WIDTH * MAP_HEIGHT;

// -----------------------------------------------------------------------------
//  Objet individuel
// -----------------------------------------------------------------------------
struct Object {
    ObjectType type;
};

// -----------------------------------------------------------------------------
//  Cellule de la grille (pile d’objets)
// -----------------------------------------------------------------------------
struct Cell {
    std::vector<Object> objects;
};

// -----------------------------------------------------------------------------
//  Grille complète
// -----------------------------------------------------------------------------
struct Grid {
    int width, height;
    std::vector<Cell> cells;

    Grid(int w = MAP_WIDTH, int h = MAP_HEIGHT);

    Cell&       cell(int x, int y);
    const Cell& cell(int x, int y) const;

    bool in_bounds(int x, int y) const;

    int playMinX = 0, playMinY = 0;
    int playMaxX = 0, playMaxY = 0;

    bool in_play_area(int x, int y) const {
        return (x >= playMinX && x <= playMaxX &&
                y >= playMinY && y <= playMaxY);
    }
};

// -----------------------------------------------------------------------------
//  Rendu d’une cellule
// -----------------------------------------------------------------------------
void draw_cell(int x, int y, const Cell& c, const PropertyTable& props);

} // namespace baba
