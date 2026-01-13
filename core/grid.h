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

namespace baba {

// -----------------------------------------------------------------------------
//  Constantes globales (taille par défaut de la grille)
// -----------------------------------------------------------------------------
constexpr int TILE_SIZE  = 16;   // Taille d’un sprite en pixels
constexpr int MAP_WIDTH  = 32;   // Largeur logique de la grille
constexpr int MAP_HEIGHT = 24;   // Hauteur logique de la grille
constexpr int MAP_SIZE   = MAP_WIDTH * MAP_HEIGHT;


// -----------------------------------------------------------------------------
//  Types d’objets du jeu
//  (objets physiques + mots textuels)
// -----------------------------------------------------------------------------
enum class ObjectType : uint8_t {
    Empty = 0,

    // Objets physiques
    Baba,
    Wall,
    Rock,
    Flag,
    Lava,
    Goop,
    Love,

    // Mots (noms)
    Text_Baba,
    Text_Wall,
    Text_Rock,
    Text_Flag,
    Text_Lava,
    Text_Goop,
    Text_Love,
    Text_Empty,

    // Mots (verbes / propriétés)
    Text_Is,
    Text_Push,
    Text_Stop,
    Text_Win,
    Text_You,
    Text_Sink,
    Text_Kill,
    Text_Swap,
    Text_Hot,
    Text_Melt,
    Text_Move,
    Text_Open,
    Text_Shut,
    Text_Float,

    Count
};


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

    // Accès sécurisé aux cellules
    Cell&       cell(int x, int y);
    const Cell& cell(int x, int y) const;

    // Vérifie si une coordonnée est dans la grille
    bool in_bounds(int x, int y) const;
	
	
    int playMinX = 0, playMinY = 0;
    int playMaxX = 0, playMaxY = 0; // inclusifs

    bool in_play_area(int x, int y) const {
        return (x >= playMinX && x <= playMaxX && y >= playMinY && y <= playMaxY);
    }
};


// ----------------------------------------------------------------------------- 
// Fonctions utilitaires de rendu 
// -----------------------------------------------------------------------------
void draw_cell(int x, int y, const Cell& c);


} // namespace baba
