/*
===============================================================================
  sprites.cpp — Implémentation de l’atlas de sprites
-------------------------------------------------------------------------------
  Rôle :
    - Mapper chaque ObjectType vers un index dans l’atlas.
    - Convertir cet index en coordonnées source (x,y,w,h).
    - Dessiner une cellule via gfx_blitRegion().

  Notes :
    - Atlas recommandé : 256×64 px (16 colonnes × 4 lignes).
    - Chaque sprite fait 16×16 px.
    - Ligne 0 : objets physiques
    - Ligne 1 : TEXT_* (noms)
    - Ligne 2 : TEXT_* (propriétés)
    - Ligne 3 : réserve future
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

// ⚠️ Atlas étendu : 4 lignes (64 px)
static constexpr int ATLAS_WIDTH  = 256;
static constexpr int ATLAS_HEIGHT = 64;

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
    // Valeur par défaut : EMPTY
    for (size_t i = 0; i < (size_t)ObjectType::Count; i++)
        g_spriteIndex[i] = 7;

    // -------------------------------------------------------------------------
    // Ligne 0 : objets physiques
    // -------------------------------------------------------------------------
    g_spriteIndex[(size_t)ObjectType::Baba]  = 0;
    g_spriteIndex[(size_t)ObjectType::Wall]  = 1;
    g_spriteIndex[(size_t)ObjectType::Rock]  = 2;
    g_spriteIndex[(size_t)ObjectType::Flag]  = 3;
    g_spriteIndex[(size_t)ObjectType::Lava]  = 4;
    g_spriteIndex[(size_t)ObjectType::Goop]  = 5;
    g_spriteIndex[(size_t)ObjectType::Love]  = 6;
    g_spriteIndex[(size_t)ObjectType::Empty] = 7;

    // Nouveaux objets
    g_spriteIndex[(size_t)ObjectType::Key]   = 8;
    g_spriteIndex[(size_t)ObjectType::Door]  = 9;
    g_spriteIndex[(size_t)ObjectType::Water] = 10;
    g_spriteIndex[(size_t)ObjectType::Ice]   = 11;
    g_spriteIndex[(size_t)ObjectType::Box]   = 12;

    // -------------------------------------------------------------------------
    // Ligne 1 : TEXT_* (noms)
    // -------------------------------------------------------------------------
    g_spriteIndex[(size_t)ObjectType::Text_Baba]  = 16;
    g_spriteIndex[(size_t)ObjectType::Text_Wall]  = 17;
    g_spriteIndex[(size_t)ObjectType::Text_Rock]  = 18;
    g_spriteIndex[(size_t)ObjectType::Text_Flag]  = 19;
    g_spriteIndex[(size_t)ObjectType::Text_Lava]  = 20;
    g_spriteIndex[(size_t)ObjectType::Text_Goop]  = 21;
    g_spriteIndex[(size_t)ObjectType::Text_Love]  = 22;
    g_spriteIndex[(size_t)ObjectType::Text_Empty] = 23;

    g_spriteIndex[(size_t)ObjectType::Text_Key]   = 24;
    g_spriteIndex[(size_t)ObjectType::Text_Door]  = 25;
    g_spriteIndex[(size_t)ObjectType::Text_Water] = 26;
    g_spriteIndex[(size_t)ObjectType::Text_Ice]   = 27;
    g_spriteIndex[(size_t)ObjectType::Text_Box]   = 28;

    // -------------------------------------------------------------------------
    // Ligne 2 : TEXT_* (propriétés)
    // -------------------------------------------------------------------------
    g_spriteIndex[(size_t)ObjectType::Text_Is]    = 32;
    g_spriteIndex[(size_t)ObjectType::Text_Push]  = 33;
    g_spriteIndex[(size_t)ObjectType::Text_Stop]  = 34;
    g_spriteIndex[(size_t)ObjectType::Text_Win]   = 35;
    g_spriteIndex[(size_t)ObjectType::Text_You]   = 36;
    g_spriteIndex[(size_t)ObjectType::Text_Sink]  = 37;
    g_spriteIndex[(size_t)ObjectType::Text_Kill]  = 38;
    g_spriteIndex[(size_t)ObjectType::Text_Swap]  = 39;

    g_spriteIndex[(size_t)ObjectType::Text_Hot]   = 40;
    g_spriteIndex[(size_t)ObjectType::Text_Melt]  = 41;
    g_spriteIndex[(size_t)ObjectType::Text_Move]  = 42;
    g_spriteIndex[(size_t)ObjectType::Text_Open]  = 43;
    g_spriteIndex[(size_t)ObjectType::Text_Shut]  = 44;
    g_spriteIndex[(size_t)ObjectType::Text_Float] = 45;
    g_spriteIndex[(size_t)ObjectType::Text_Pull]  = 46;

    // Ligne 3 : libre pour extensions futures
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
//  Dessine un sprite à l’écran
// -----------------------------------------------------------------------------
void draw_sprite(int x, int y, ObjectType t)
{
    if ((int)t < 0 || (int)t >= (int)ObjectType::Count) {
        printf("INVALID TYPE: %d\n", (int)t);
        return;
    }

    SpriteRect r = sprite_rect_for(t);

    gfx_blitRegion(
        getAtlasPixels(),
        ATLAS_WIDTH,
        r.x, r.y,
        r.w, r.h,
        x, y
    );
}

} // namespace baba
