#include "graphics.h"

// Lib AKA
#include "gb_graphics.h"
#include "gb_common.h"
#include "gb_ll_lcd.h"
#include "..\lib\font8x8_basic.h"   // font8x8_basic[128][8] (uint8_t)

// Namespace moteur
namespace baba {

// ============================================================================
//  Objet graphique AKA global
// ============================================================================
static gb_graphics g_gfx;


// ============================================================================
//  Initialisation graphique
// ============================================================================
void gfx_init()
{
    // Le LCD et le bus sont déjà initialisés par gb_core::init().
    // Ici on ne fait que régler la luminosité de base.
    g_gfx.set_backlight_percent(80);
}


// ============================================================================
//  Effacement / présentation
// ============================================================================
void gfx_clear(Color color)
{
    lcd_clear(color);
}

void gfx_present()
{
    g_gfx.update();
}

void gfx_flush()
{
    gfx_present();
}


// ============================================================================
//  Pixel
// ============================================================================
void gfx_putpixel(int x, int y, Color color)
{
    lcd_putpixel(x, y, color);
}


// ============================================================================
//  Rectangle plein
// ============================================================================
void gfx_fillRect(int x, int y, int w, int h, Color color)
{
    for (int iy = 0; iy < h; ++iy)
    {
        for (int ix = 0; ix < w; ++ix)
        {
            lcd_putpixel(x + ix, y + iy, color);
        }
    }
}


// ============================================================================
//  Rectangle vide (contour)
// ============================================================================
void gfx_drawRect(int x, int y, int w, int h, Color color)
{
    // Haut / bas
    for (int ix = 0; ix < w; ++ix)
    {
        lcd_putpixel(x + ix, y,         color);
        lcd_putpixel(x + ix, y + h - 1, color);
    }

    // Gauche / droite
    for (int iy = 0; iy < h; ++iy)
    {
        lcd_putpixel(x,         y + iy, color);
        lcd_putpixel(x + w - 1, y + iy, color);
    }
}


// ============================================================================
//  Ligne (Bresenham)
// ============================================================================
void gfx_drawLine(int x0, int y0, int x1, int y1, Color color)
{
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = (y1 > y0) ? (y0 - y1) : (y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true)
    {
        lcd_putpixel(x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0  += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0  += sy;
        }
    }
}


// ============================================================================
//  Cercle vide (Midpoint circle algorithm)
// ============================================================================
void gfx_drawCircle(int cx, int cy, int r, Color color)
{
    int x = r;
    int y = 0;
    int err = 1 - r;

    while (x >= y)
    {
        lcd_putpixel(cx + x, cy + y, color);
        lcd_putpixel(cx + y, cy + x, color);
        lcd_putpixel(cx - y, cy + x, color);
        lcd_putpixel(cx - x, cy + y, color);
        lcd_putpixel(cx - x, cy - y, color);
        lcd_putpixel(cx - y, cy - x, color);
        lcd_putpixel(cx + y, cy - x, color);
        lcd_putpixel(cx + x, cy - y, color);

        y++;
        if (err < 0)
        {
            err += 2 * y + 1;
        }
        else
        {
            x--;
            err += 2 * (y - x + 1);
        }
    }
}


// ============================================================================
//  Cercle plein
// ============================================================================
void gfx_fillCircle(int cx, int cy, int r, Color color)
{
    int x = r;
    int y = 0;
    int err = 1 - r;

    while (x >= y)
    {
        // Dessine des segments horizontaux pour remplir le cercle
        for (int ix = cx - x; ix <= cx + x; ++ix)
        {
            lcd_putpixel(ix, cy + y, color);
            lcd_putpixel(ix, cy - y, color);
        }
        for (int ix = cx - y; ix <= cx + y; ++ix)
        {
            lcd_putpixel(ix, cy + x, color);
            lcd_putpixel(ix, cy - x, color);
        }

        y++;
        if (err < 0)
        {
            err += 2 * y + 1;
        }
        else
        {
            x--;
            err += 2 * (y - x + 1);
        }
    }
}


// ============================================================================
//  Blit d’un bitmap arbitraire (pixels 16 bits RGB565)
//  - utilisé pour les images “pleines” (titre, écrans fixes, etc.)
// ============================================================================
void gfx_blit(const uint16_t* pixels, int w, int h, int x, int y)
{
    for (int iy = 0; iy < h; ++iy)
    {
        const uint16_t* srcLine = pixels + iy * w;
        int dy = y + iy;

        for (int ix = 0; ix < w; ++ix)
        {
            int dx = x + ix;
            lcd_putpixel(dx, dy, srcLine[ix]);
        }
    }
}


// ============================================================================
//  Blit d’une région rectangulaire dans un atlas 16 bits (RGB565)
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
)
{
    for (int y = 0; y < srcH; ++y)
    {
        // Pointeur sur le début de la ligne source dans l’atlas
        const uint16_t* srcLine = atlas + (srcY + y) * atlasW + srcX;
        int dy = dstY + y;

        // Version CPU simple : pixel par pixel
        // (remplaçable facilement par un appel DMA ligne par ligne)
        for (int x = 0; x < srcW; ++x)
        {
            int dx = dstX + x;
            lcd_putpixel(dx, dy, srcLine[x]);
        }
    }
}

/*
    Variante DMA (à brancher plus tard) :

    void gfx_blitRegion(
        const uint16_t* atlas,
        int atlasW,
        int srcX, int srcY,
        int srcW, int srcH,
        int dstX, int dstY
    )
    {
        lcd_startWriteWindow(dstX, dstY, srcW, srcH);

        for (int y = 0; y < srcH; ++y)
        {
            const uint16_t* srcLine = atlas + (srcY + y) * atlasW + srcX;
            lcd_writePixels(srcLine, srcW); // transfert DMA idéal
        }

        lcd_endWriteWindow();
    }
*/


// ============================================================================
//  Texte (font8x8)
// ============================================================================
void gfx_text(int x, int y, const char* text, Color color)
{
    int px = x;

    while (*text)
    {
        unsigned char c = static_cast<unsigned char>(*text++);
        if (c >= 128)
            c = '?'; // fallback simple

        const uint8_t* glyph = reinterpret_cast<const uint8_t*>(font8x8_basic[c]);

        for (int gy = 0; gy < 8; ++gy)
        {
            uint8_t row = glyph[gy];
            for (int gx = 0; gx < 8; ++gx)
            {
                if (row & (1 << gx))
                    lcd_putpixel(px + gx, y + gy, color);
            }
        }

        px += 8; // avance d’un caractère
    }
}

void gfx_text_center(int y, const char* text, Color color)
{
    int len = 0;
    for (const char* p = text; *p; ++p)
        len++;

    int x = (gfx_width() - len * 8) / 2;
    gfx_text(x, y, text, color);
}


// ============================================================================
//  Dimensions écran
// ============================================================================
int gfx_width()
{
    return SCREEN_WIDTH;
}

int gfx_height()
{
    return SCREEN_HEIGHT;
}

} // namespace baba
