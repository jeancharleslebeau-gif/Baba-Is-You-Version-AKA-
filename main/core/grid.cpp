/*
===============================================================================
  grid.cpp — Implémentation de la grille de jeu
-------------------------------------------------------------------------------
*/

#include <cmath>
#include "grid.h"
#include "rules.h"
#include "assets/gfx/atlas.h"
#include "core/graphics.h"
#include "game/config.h"   
#include "game/game.h"     
#include "core/sprites.h"

namespace baba {

// -----------------------------------------------------------------------------
//  Constructeur par défaut
// -----------------------------------------------------------------------------
Grid::Grid()
    : width(0), height(0),
      playMinX(0), playMaxX(0),
      playMinY(0), playMaxY(0),
      cells()
{
}

// -----------------------------------------------------------------------------
//  Constructeur : crée une grille w×h avec des cellules vides
// -----------------------------------------------------------------------------
Grid::Grid(int w, int h)
    : width(w), height(h),
      playMinX(0), playMaxX(w - 1),
      playMinY(0), playMaxY(h - 1),
      cells(w * h)
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
//  Zone jouable (mise à jour par rules_parse)
// -----------------------------------------------------------------------------
bool Grid::in_play_area(int x, int y) const
{
    return x >= playMinX && x <= playMaxX &&
           y >= playMinY && y <= playMaxY;
}

// -----------------------------------------------------------------------------
//  Effet WIN procédural (inchangé)
// -----------------------------------------------------------------------------
static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) |
           ((g & 0xFC) << 3) |
           (b >> 3);
}

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

        float alpha = 0.6f + 0.4f * sinf(t * 5.0f);

        uint8_t r = static_cast<uint8_t>(255 * alpha);
        uint8_t g = static_cast<uint8_t>(240 * alpha);
        uint8_t b = static_cast<uint8_t>(120 * alpha);

        uint16_t col = rgb565(r, g, b);

        gfx_fillRect(px2 - 1, py2 - 1, 2, 2, col);
    }
}

// -----------------------------------------------------------------------------
//  draw_cell() — Version 1: dessin normal (sans zoom)
// -----------------------------------------------------------------------------
void draw_cell(int x, int y, const Cell& c, const PropertyTable& props)
{
    bool hasYou = false;
    bool hasWin = false;

    for (auto& obj : c.objects) {
        draw_sprite(x, y, obj.type);

        const Properties& pr = props[(int)obj.type];
        if (pr.you) hasYou = true;
        if (pr.win) hasWin = true;
    }

    if (hasYou && hasWin) {
        extern float g_time;
        draw_win_effect(x, y, g_time);
    }
}

// -----------------------------------------------------------------------------
//  draw_cell_scaled() — Version 2: dessin avec zoom Q8.8
// -----------------------------------------------------------------------------
void draw_cell_scaled(int x, int y, const Cell& c, const PropertyTable& props, int scale_fp)
{
    bool hasYou = false;
    bool hasWin = false;

    // Taille finale d’un tile en pixels
    int tile_px = (TILE_SIZE * scale_fp) >> 8;

    for (auto& obj : c.objects)
    {
        // Récupération du sprite dans l’atlas
        SpriteRect r = sprite_rect_for(obj.type);

        // Blit zoomé
        gfx_blitRegionScaled(
            getAtlasPixels(),
            256,               // largeur atlas
            r.x, r.y,
            r.w, r.h,
            x, y,
            scale_fp
        );

        const Properties& pr = props[(int)obj.type];
        if (pr.you) hasYou = true;
        if (pr.win) hasWin = true;
    }

    // Effet WIN (adapté au zoom)
    if (hasYou && hasWin) {
        extern float g_time;

        // On applique le zoom au centre de la cellule
        float cx = x + tile_px * 0.5f;
        float cy = y + tile_px * 0.5f;

        const int count = 12;

        for (int i = 0; i < count; i++) {
            float t = g_time * 4.0f + i * 0.7f;

            float angle  = fmodf(t * 2.3f + i * 1.1f, 6.28318f);
            float radius = (2.0f + 3.0f * (0.5f + 0.5f * sinf(t * 3.0f)))
                           * (scale_fp / 256.0f);

            float px2 = cx + cosf(angle) * radius;
            float py2 = cy + sinf(angle) * radius;

            float alpha = 0.6f + 0.4f * sinf(t * 5.0f);

            uint8_t r = static_cast<uint8_t>(255 * alpha);
            uint8_t g = static_cast<uint8_t>(240 * alpha);
            uint8_t b = static_cast<uint8_t>(120 * alpha);

            uint16_t col = rgb565(r, g, b);

            gfx_fillRect(px2 - 1, py2 - 1, 2, 2, col);
        }
    }
}

} // namespace baba
