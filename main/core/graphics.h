#pragma once
#include <stdint.h>

namespace baba {

// ============================================================================
//  Type Color — Couleur 16 bits (format RGB565)
// ============================================================================
using Color = uint16_t;


// ============================================================================
//  Initialisation du système graphique
// ----------------------------------------------------------------------------
//  - Configure gb_graphics (backlight, pipeline LCD)
//  - Doit être appelé après gb_core.init()
// ============================================================================
void gfx_init();


// ============================================================================
//  Effacement / Présentation
// ============================================================================
void gfx_clear(Color color);     // Efface l’écran
void gfx_present();              // Rafraîchit le LCD
void gfx_flush();                // Alias pratique


// ============================================================================
//  Pixel et primitives de base
// ============================================================================
void gfx_putpixel(int x, int y, Color color);

// Rectangle plein
void gfx_fillRect(int x, int y, int w, int h, Color color);

// Rectangle vide (contour)
void gfx_drawRect(int x, int y, int w, int h, Color color);

// Ligne (algorithme de Bresenham)
void gfx_drawLine(int x0, int y0, int x1, int y1, Color color);

// Cercle vide (Midpoint circle)
void gfx_drawCircle(int cx, int cy, int r, Color color);

// Cercle plein
void gfx_fillCircle(int cx, int cy, int r, Color color);


// ============================================================================
//  Blit d’un bitmap arbitraire (ex: 16×16, 320×240…)
// ============================================================================
void gfx_blit(const uint16_t* pixels, int w, int h, int x, int y);


// ============================================================================
//  Blit d’une région rectangulaire dans un atlas 16 bits (RGB565)
// ----------------------------------------------------------------------------
//  - atlas = image brute (ex: 256×32) rangée ligne par ligne
//  - srcX, srcY, srcW, srcH = rectangle source dans l’atlas
//  - dstX, dstY = position de destination à l’écran
//  - implémentation “DMA-friendly” : une ligne source = bloc contigu
// ============================================================================
void gfx_blitRegion(
    const uint16_t* atlas,
    int atlasW,
    int srcX, int srcY,
    int srcW, int srcH,
    int dstX, int dstY
);

// ============================================================================
//  Blit d’une région rectangulaire avec scaling (Q8.8)
// ----------------------------------------------------------------------------
//  - Identique à gfx_blitRegion(), mais applique un zoom nearest-neighbor.
//  - scale_fp = facteur de zoom en Q8.8 (256 = 1.0).
//  - Utilisé pour le zoom de la grille (BabaIsU).
// ============================================================================
void gfx_blitRegionScaled(
    const uint16_t* atlas,
    int atlasW,
    int srcX, int srcY,
    int srcW, int srcH,
    int dstX, int dstY,
    int scale_fp
);


// ============================================================================
//  Blit d’un bitmap arbitraire avec scaling (Q8.8)
// ----------------------------------------------------------------------------
//  - Version zoomée de gfx_blit().
//  - pixels = image brute RGB565 (srcW × srcH).
//  - scale_fp = facteur de zoom en Q8.8 (256 = 1.0).
// ============================================================================
void gfx_blitScaled(
    const uint16_t* pixels,
    int srcW, int srcH,
    int dstX, int dstY,
    int scale_fp
);



// ============================================================================
//  Texte (font8x8)
// ============================================================================
void gfx_text(int x, int y, const char* text, Color color);
void gfx_text_center(int y, const char* text, Color color);


// ============================================================================
//  Dimensions écran
// ============================================================================
int gfx_width();
int gfx_height();


// ============================================================================
//  Couleurs RGB565 standard
// ============================================================================
static constexpr Color COLOR_BLACK  = 0x0000;
static constexpr Color COLOR_WHITE  = 0xFFFF;
static constexpr Color COLOR_YELLOW = 0xFFE0;
static constexpr Color COLOR_RED    = 0xF800;
static constexpr Color COLOR_GREEN  = 0x07E0;
static constexpr Color COLOR_BLUE   = 0x001F;

} // namespace baba
