/*
===============================================================================
  sprites.cpp — Implémentation de l’atlas de sprites
-------------------------------------------------------------------------------
  Rôle :
    - Mapper chaque ObjectType vers un index dans l’atlas.
    - Convertir cet index en coordonnées source (x,y,w,h).
    - Dessiner une cellule via gfx_blitRegion().

  Notes :
    - L’atlas utilisé fait 256×32 px (16 colonnes × 2 lignes).
    - Chaque sprite fait 16×16 px.
    - Les indices ci-dessous correspondent à l’atlas minimal blanc.

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#include "sprites.h"
#include "core/graphics.h"
#include "assets/gfx/atlas.h"
#include <cstdio>

namespace baba {

// -----------------------------------------------------------------------------
//  Constantes liées à l’atlas
// -----------------------------------------------------------------------------
static constexpr int ATLAS_TILE_W = 16;
static constexpr int ATLAS_TILE_H = 16;
static constexpr int ATLAS_COLS   = 16;
static constexpr int ATLAS_WIDTH  = 256;
static constexpr int ATLAS_HEIGHT = 32;

// -----------------------------------------------------------------------------
//  Accès aux pixels de l’atlas
// -----------------------------------------------------------------------------
const uint16_t* getAtlasPixels()
{
    return atlas_pixels;
}

// -----------------------------------------------------------------------------
//  Table de correspondance ObjectType → index dans l’atlas
// -----------------------------------------------------------------------------
static uint16_t g_spriteIndex[(size_t)ObjectType::Count];

// -----------------------------------------------------------------------------
//  Initialisation des indices de sprites
// -----------------------------------------------------------------------------
void sprites_init()
{
    // Valeur par défaut : EMPTY (index 7)
    for (size_t i = 0; i < (size_t)ObjectType::Count; i++)
        g_spriteIndex[i] = 7;

    // -------------------------------------------------------------------------
    // Ligne 0 : objets
    // -------------------------------------------------------------------------
    g_spriteIndex[(size_t)ObjectType::Baba] = 0;
    g_spriteIndex[(size_t)ObjectType::Wall] = 1;
    g_spriteIndex[(size_t)ObjectType::Rock] = 2;
    g_spriteIndex[(size_t)ObjectType::Flag] = 3;
    g_spriteIndex[(size_t)ObjectType::Lava] = 4;
    g_spriteIndex[(size_t)ObjectType::Goop] = 5;
    g_spriteIndex[(size_t)ObjectType::Love] = 6;
    g_spriteIndex[(size_t)ObjectType::Empty] = 7;

    // -------------------------------------------------------------------------
    // Ligne 1 : mots
    // -------------------------------------------------------------------------
    g_spriteIndex[(size_t)ObjectType::Text_Baba]  = 16;
    g_spriteIndex[(size_t)ObjectType::Text_Wall]  = 17;
    g_spriteIndex[(size_t)ObjectType::Text_Rock]  = 18;
    g_spriteIndex[(size_t)ObjectType::Text_Flag]  = 19;
    g_spriteIndex[(size_t)ObjectType::Text_Lava]  = 20;
    g_spriteIndex[(size_t)ObjectType::Text_Goop]  = 21;
    g_spriteIndex[(size_t)ObjectType::Text_Love]  = 22;
    g_spriteIndex[(size_t)ObjectType::Text_Empty] = 23;

    g_spriteIndex[(size_t)ObjectType::Text_Is]    = 24;
    g_spriteIndex[(size_t)ObjectType::Text_Push]  = 25;
    g_spriteIndex[(size_t)ObjectType::Text_Stop]  = 26;
    g_spriteIndex[(size_t)ObjectType::Text_Win]   = 27;
    g_spriteIndex[(size_t)ObjectType::Text_You]   = 28;
    g_spriteIndex[(size_t)ObjectType::Text_Sink]  = 29;
    g_spriteIndex[(size_t)ObjectType::Text_Kill]  = 30;
    g_spriteIndex[(size_t)ObjectType::Text_Swap]  = 31;
}

// -----------------------------------------------------------------------------
//  Calcule le rectangle source dans l’atlas pour un ObjectType donné
// -----------------------------------------------------------------------------
SpriteRect sprite_rect_for(ObjectType t)
{
    uint16_t idx = g_spriteIndex[(size_t)t];
    int col = idx % ATLAS_COLS;
    int row = idx / ATLAS_COLS;

    return {
        col * ATLAS_TILE_W,
        row * ATLAS_TILE_H,
        ATLAS_TILE_W,
        ATLAS_TILE_H
    };
}

// -----------------------------------------------------------------------------
//  Dessine une cellule à l’écran
// -----------------------------------------------------------------------------
void draw_sprite(int x, int y, ObjectType t)
{
    if ((int)t < 0 || (int)t >= (int)ObjectType::Count) {
        printf("INVALID TYPE: %d\n", (int)t);
        return;
    }

    SpriteRect r = sprite_rect_for(t);

    // Nouvelle version : blit rectangulaire générique
    gfx_blitRegion(
        getAtlasPixels(),
        ATLAS_WIDTH,   // stride réel de l’atlas (256 px)
        r.x, r.y,
        r.w, r.h,
        x, y
    );
}

} // namespace baba
