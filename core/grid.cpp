/*
===============================================================================
  grid.cpp — Implémentation de la grille de jeu
-------------------------------------------------------------------------------
*/

#include <cmath>    // pour sinf, cosf, fmodf
#include "grid.h"
#include "rules.h"
#include "assets/gfx/atlas.h"
#include "core/graphics.h"
#include "sprites.h"

namespace baba {

// -----------------------------------------------------------------------------
//  Helper RGB565 (remplace rgba() inexistant)
// -----------------------------------------------------------------------------
static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) |
           ((g & 0xFC) << 3) |
           (b >> 3);
}

// -----------------------------------------------------------------------------
//  Constructeur : crée une grille w×h avec des cellules vides
// -----------------------------------------------------------------------------
Grid::Grid(int w, int h)
    : width(w), height(h), cells(w * h)
{
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
//  Effet WIN procédural (mini feu d’artifice)
// -----------------------------------------------------------------------------
static void draw_win_effect(int px, int py, float time)
{
    float cx = px + 8.0f;
    float cy = py + 8.0f;

    const int count = 12;

    for (int i = 0; i < count; i++) {
        float t = time * 4.0f + i * 0.7f;

        float angle  = fmodf(t * 2.3f + i * 1.1f, 6.28318f);
        float radius = 2.0f + 3.0f * (0.5f + 0.5f * sinf(t * 3.0f));

        float px2 = cx + cosf(angle) * radius;
        float py2 = cy + sinf(angle) * radius;

        // Alpha simulé en modulant la luminosité
        float alpha = 0.6f + 0.4f * sinf(t * 5.0f);

        uint8_t r = static_cast<uint8_t>(255 * alpha);
        uint8_t g = static_cast<uint8_t>(240 * alpha);
        uint8_t b = static_cast<uint8_t>(120 * alpha);

        uint16_t col = rgb565(r, g, b);

        gfx_fillRect(px2 - 1, py2 - 1, 2, 2, col);
    }
}

// -----------------------------------------------------------------------------
//  draw_cell() — Dessine une cellule + effet WIN
// -----------------------------------------------------------------------------
void draw_cell(int x, int y, const Cell& c, const PropertyTable& props)
{
    bool hasYou = false;
    bool hasWin = false;

    // 1) Dessiner les objets
    for (auto& obj : c.objects) {
        draw_sprite(x, y, obj.type);

        const Properties& pr = props[(int)obj.type];
        if (pr.you) hasYou = true;
        if (pr.win) hasWin = true;
    }

    // 2) Effet WIN superposé
    if (hasYou && hasWin) {
        extern float g_time;
        draw_win_effect(x, y, g_time);
    }
}

} // namespace baba
