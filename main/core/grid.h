/*
===============================================================================
  grid.h — Grille de jeu dynamique
-------------------------------------------------------------------------------
  Rôle :
    - Représenter la grille logique d’un niveau.
    - Stocker une matrice w×h de Cell, chaque Cell contenant plusieurs objets.
    - Fournir un accès sécurisé aux cellules (lecture/écriture).
    - Maintenir une zone jouable (playMinX/Y, playMaxX/Y) utilisée par :
        * rules_parse()      → détection des limites du niveau
        * update_camera()    → centrage caméra
        * game_draw()        → optimisation du rendu
    - Supporter des niveaux de tailles variables (niveaux normaux + custom).
===============================================================================
*/

#pragma once

#include <vector>
#include "core/types.h"

namespace baba {

// -----------------------------------------------------------------------------
//  Un objet dans une cellule (type uniquement, pas de position locale)
// -----------------------------------------------------------------------------
struct Object {
    ObjectType type;
};

// -----------------------------------------------------------------------------
//  Une cellule contient une pile d’objets superposés
// -----------------------------------------------------------------------------
struct Cell {
    std::vector<Object> objects;
};

// -----------------------------------------------------------------------------
//  Grille dynamique : structure centrale du moteur
// -----------------------------------------------------------------------------
class Grid {
public:

    // -------------------------------------------------------------------------
    //  Constructeur par défaut
    //  Utilisé lors de l'initialisation de GameState (g_state.grid)
    // -------------------------------------------------------------------------
    Grid();

    // -------------------------------------------------------------------------
    //  Constructeur dynamique (w×h)
    //  Crée une grille vide de dimensions réelles width × height
    // -------------------------------------------------------------------------
    Grid(int w, int h);

    // -------------------------------------------------------------------------
    //  Vérifie si (x,y) est dans les limites réelles du niveau
    // -------------------------------------------------------------------------
    bool in_bounds(int x, int y) const;

    // -------------------------------------------------------------------------
    //  Accès aux cellules (lecture/écriture)
    // -------------------------------------------------------------------------
    Cell& cell(int x, int y);
    const Cell& cell(int x, int y) const;

    // -------------------------------------------------------------------------
    //  Zone jouable (mise à jour par rules_parse)
    //  Permet à la caméra et au rendu d’ignorer les marges vides
    // -------------------------------------------------------------------------
    bool in_play_area(int x, int y) const;

    // Dimensions réelles du niveau
    int width;
    int height;

    // Zone jouable (bornes min/max détectées par rules_parse)
    int playMinX;
    int playMaxX;
    int playMinY;
    int playMaxY;

    // -------------------------------------------------------------------------
    //  Contenu de la grille (public car utilisé par movement.cpp, rules.cpp,
    //  game_draw(), etc.)
    // -------------------------------------------------------------------------
    std::vector<Cell> cells;
};

// -----------------------------------------------------------------------------
//  draw_cell() — Dessine une cellule + effet WIN
//  Implémentation dans grid.cpp
// -----------------------------------------------------------------------------
void draw_cell(int x, int y, const Cell& c, const PropertyTable& props);
void draw_cell_scaled(int x, int y, const Cell& c, const PropertyTable& props, int scale_fp);


} // namespace baba
