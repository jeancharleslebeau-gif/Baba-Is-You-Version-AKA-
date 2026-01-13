/*
===============================================================================
  levels.cpp — Chargement des niveaux
-------------------------------------------------------------------------------
  Rôle :
    - Charger un niveau défini dans levels_data.cpp.
    - Chaque niveau a ses propres dimensions (width/height).
    - Les données sont placées dans la grille logique (MAP_WIDTH × MAP_HEIGHT).
    - Si le niveau est plus petit que la grille logique, il est centré automatiquement.

  Notes :
    - Les données brutes des niveaux (level1 … level21) sont définies dans levels_data.cpp.
    - Les macros (WALL, FLAG, ROCK, etc.) sont définies dans defines.h et mappent vers ObjectType.
    - La grille logique est définie dans grid.h (MAP_WIDTH = 32, MAP_HEIGHT = 24).
===============================================================================
*/

#include "levels.h"
#include "defines.h"
#include "core/grid.h"

namespace baba {

/*
===============================================================================
  load_level()
-------------------------------------------------------------------------------
  Rôle :
    - Charger un niveau identifié par son index.
    - Utiliser ses dimensions propres (width/height).
    - Placer les objets dans la grille logique avec centrage automatique.

  Paramètres :
    - index : numéro du niveau à charger.
    - g     : référence vers la grille à remplir.

  Étapes :
    1. Crée une grille logique vide (MAP_WIDTH × MAP_HEIGHT).
    2. Récupère les données du niveau (LevelInfo).
    3. Calcule le décalage pour centrer le niveau.
    4. Copie les objets dans la grille logique.
===============================================================================
*/

void load_level(int index, Grid& g)
{
	// Grille logique (32×24)
    g = Grid(MAP_WIDTH, MAP_HEIGHT);

    const LevelInfo& info = levels[index];

	// Calcul du décalage pour centrer le niveau
    int offsetX = (MAP_WIDTH  - info.width)  / 2;
    int offsetY = (MAP_HEIGHT - info.height) / 2;

    // Définir la zone jouable
    g.playMinX = offsetX;
    g.playMinY = offsetY;
    g.playMaxX = offsetX + info.width  - 1;
    g.playMaxY = offsetY + info.height - 1;

    for (int y = 0; y < info.height; ++y) {
        for (int x = 0; x < info.width; ++x) {
            uint8_t code = info.data[y * info.width + x];
            if (code == EMPTY) continue;
            g.cell(offsetX + x, offsetY + y).objects.push_back({static_cast<ObjectType>(code)});
        }
    }
}


} // namespace baba
