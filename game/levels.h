/*
===============================================================================
  levels.h — Déclarations des niveaux
-------------------------------------------------------------------------------
  Rôle :
    - Définir les constantes de dimensions de la carte.
    - Déclarer les tableaux de niveaux (extern).
    - Fournir un accès générique via levels[] et levels_count().
    - Utilisé par load_level() dans levels.cpp pour remplir la Grid.

  Notes :
    - Chaque niveau est codé en dur sous forme de tableau uint8_t.
    - Les valeurs proviennent de defines.h (EMPTY, BABA, FLAG, W_BABA, W_IS…).
    - MAP_WIDTH × MAP_HEIGHT = META_FULL_SIZE.
    - Évolution possible : chargement dynamique depuis fichiers externes.
===============================================================================
*/

#pragma once
#include <cstdint>
#include "core/grid.h"

namespace baba {

// -----------------------------------------------------------------------------
// Dimensions de la carte
// -----------------------------------------------------------------------------
// Dimensions des niveaux META
constexpr int META_WIDTH     = 13;
constexpr int META_HEIGHT    = 10;
constexpr int META_FULL_SIZE = META_WIDTH * META_HEIGHT;


// ----------------------------------------------------------------------------- 
// Structure LevelInfo : associe données et dimensions 
// -----------------------------------------------------------------------------
struct LevelInfo {
    const uint8_t* data;   // tableau brut du niveau
    int width;             // largeur propre au niveau
    int height;            // hauteur propre au niveau
};


// -----------------------------------------------------------------------------
// Déclarations des niveaux (définis dans levels.cpp)
// -----------------------------------------------------------------------------
extern const uint8_t level1[META_FULL_SIZE];
extern const uint8_t level2[META_FULL_SIZE];
extern const uint8_t level3[META_FULL_SIZE];
extern const uint8_t level4[META_FULL_SIZE];
extern const uint8_t level5[META_FULL_SIZE];
extern const uint8_t level6[META_FULL_SIZE];
extern const uint8_t level7[META_FULL_SIZE];
extern const uint8_t level8[META_FULL_SIZE];
extern const uint8_t level9[META_FULL_SIZE];
extern const uint8_t level10[META_FULL_SIZE];
extern const uint8_t level11[META_FULL_SIZE];
extern const uint8_t level12[META_FULL_SIZE];
extern const uint8_t level13[META_FULL_SIZE];
extern const uint8_t level14[META_FULL_SIZE];
extern const uint8_t level15[META_FULL_SIZE];
extern const uint8_t level16[META_FULL_SIZE];
extern const uint8_t level17[META_FULL_SIZE];
extern const uint8_t level18[META_FULL_SIZE];
extern const uint8_t level19[META_FULL_SIZE];
extern const uint8_t level20[META_FULL_SIZE];
extern const uint8_t level21[META_FULL_SIZE];

// Placeholders pour extension
extern const uint8_t level22[META_FULL_SIZE];
extern const uint8_t level23[META_FULL_SIZE];
extern const uint8_t level24[META_FULL_SIZE];
extern const uint8_t level25[META_FULL_SIZE];

// -----------------------------------------------------------------------------
// Tableau global des niveaux
// -----------------------------------------------------------------------------
extern const LevelInfo levels[];


// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

// Nombre de niveaux disponibles
inline int levels_count() {
    // ⚠️ À mettre à jour si tu ajoutes/retires des niveaux
    return 21; 
}

// Charge un niveau dans une Grid
void load_level(int index, Grid& g);

} // namespace baba
