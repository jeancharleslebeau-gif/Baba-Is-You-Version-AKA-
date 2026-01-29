/*
===============================================================================
  levels.cpp — Chargement des niveaux
-------------------------------------------------------------------------------
  Rôle :
    - Charger un niveau défini dans levels_data.cpp.
    - Chaque niveau possède ses propres dimensions (width / height).
    - La grille n'est plus fixe : elle est créée dynamiquement à la taille
      exacte du niveau (Grid(width, height)).
    - Aucun centrage n'est effectué ici : la caméra se charge de centrer
      automatiquement le niveau à l'écran via update_camera().

  Notes :
    - Les données brutes des niveaux (level1 … level21) sont définies dans
      levels_data.cpp.
    - Les macros (WALL, FLAG, ROCK, etc.) sont définies dans defines.h et
      mappent vers ObjectType.
    - La zone jouable (playMinX/Y, playMaxX/Y) est initialisée ici, puis
      éventuellement raffinée par rules_parse().
===============================================================================
*/


#include "levels.h"
#include "defines.h"
#include "core/grid.h"
#include "core/types.h"

namespace baba {

/*
===============================================================================
  load_level()
-------------------------------------------------------------------------------
  Rôle :
    - Charger un niveau identifié par son index.
    - Créer une grille dynamique de la taille exacte du niveau.
    - Copier les objets dans la grille sans centrage.
    - Initialiser la zone jouable (playMinX/Y, playMaxX/Y).

  Notes :
    - Le centrage visuel est effectué plus tard par update_camera().
===============================================================================
*/

void load_level(int index, Grid& g)
{
    const LevelInfo& info = levels[index];

    // Créer une grille de la bonne taille
    g = Grid(info.width, info.height);

    // Copier les objets
    for (int y = 0; y < info.height; ++y)
        for (int x = 0; x < info.width; ++x) {

            uint8_t raw = info.data[y * info.width + x];
            ObjectType type = static_cast<ObjectType>(raw);

            if (type != ObjectType::Empty)
                g.cell(x, y).objects.push_back({ type });
        }

    // === Recalcul automatique de la zone jouable ===
    int minX = g.width;
    int maxX = -1;
    int minY = g.height;
    int maxY = -1;

    for (int y = 0; y < g.height; ++y)
        for (int x = 0; x < g.width; ++x)
            if (!g.cell(x, y).objects.empty()) {
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }

    // Niveau vide (rare) → fallback
    if (maxX == -1) {
        minX = 0;
        maxX = g.width  - 1;
        minY = 0;
        maxY = g.height - 1;
    }

    g.playMinX = minX;
    g.playMaxX = maxX;
    g.playMinY = minY;
    g.playMaxY = maxY;
}

void load_level_from_text(const char* text, Grid& out)
{
    std::vector<std::vector<ObjectType>> rows;

    std::string line;
    std::stringstream ss(text);

    while (std::getline(ss, line)) {
        if (line.empty()) continue;

        std::vector<ObjectType> row;
        std::stringstream ls(line);
        std::string token;

        while (std::getline(ls, token, ',')) {
            while (!token.empty() && token[0] == ' ') token.erase(0, 1);
            while (!token.empty() && token.back() == ' ') token.pop_back();
            row.push_back(parse_object_type(token.c_str()));
        }

        rows.push_back(row);
    }

    int h = rows.size();
    int w = rows[0].size();

    out = Grid(w, h);

    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            if (rows[y][x] != ObjectType::Empty)
                out.cell(x, y).objects.push_back({ rows[y][x] });

    // === Recalcul automatique de la zone jouable ===
    int minX = w;
    int maxX = -1;
    int minY = h;
    int maxY = -1;

    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            if (!out.cell(x, y).objects.empty()) {
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }

    if (maxX == -1) {
        minX = 0;
        maxX = w - 1;
        minY = 0;
        maxY = h - 1;
    }

    out.playMinX = minX;
    out.playMaxX = maxX;
    out.playMinY = minY;
    out.playMaxY = maxY;
}

std::string export_level_to_text(const LevelInfo& info)
{
    std::string out;

    for (int y = 0; y < info.height; ++y) {
        for (int x = 0; x < info.width; ++x) {

            uint8_t raw = info.data[y * info.width + x];
            ObjectType type = static_cast<ObjectType>(raw);

            out += object_type_to_text(type);

            if (x + 1 < info.width)
                out += ", ";
        }
        out += "\n";
    }

    return out;
}



} // namespace baba
