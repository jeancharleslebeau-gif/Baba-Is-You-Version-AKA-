/*
===============================================================================
  levels.h — Déclarations des niveaux
-------------------------------------------------------------------------------
*/

#pragma once
#include <cstdint>
#include <sstream>
#include "core/grid.h"

namespace baba {

// -----------------------------------------------------------------------------
// Dimensions META (uniquement pour les 21 premiers niveaux)
// -----------------------------------------------------------------------------
constexpr int META_WIDTH     = 13;
constexpr int META_HEIGHT    = 10;
constexpr int META_FULL_SIZE = META_WIDTH * META_HEIGHT;


// ----------------------------------------------------------------------------- 
// Structure LevelInfo : associe données et dimensions 
// -----------------------------------------------------------------------------
struct LevelInfo {
    const uint8_t* data;
    int width;
    int height;
};


// -----------------------------------------------------------------------------
// Déclarations des niveaux (définis dans levels.cpp)
// -----------------------------------------------------------------------------
// Niveaux META (fixes 13×10)
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

// -----------------------------------------------------------------------------
// Niveaux 22 → 40 (tailles variables)
// -----------------------------------------------------------------------------
extern const uint8_t level22[];
extern const uint8_t level23[];
extern const uint8_t level24[];
extern const uint8_t level25[];
extern const uint8_t level26[];
extern const uint8_t level27[];
extern const uint8_t level28[];
extern const uint8_t level29[];
extern const uint8_t level30[];
extern const uint8_t level31[];
extern const uint8_t level32[];
extern const uint8_t level33[];
extern const uint8_t level34[];
extern const uint8_t level35[];
extern const uint8_t level36[];
extern const uint8_t level37[];
extern const uint8_t level38[];
extern const uint8_t level39[];
extern const uint8_t level40[];


// -----------------------------------------------------------------------------
// Tableau global des niveaux
// -----------------------------------------------------------------------------
extern const LevelInfo levels[];


// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
inline int levels_count() {
    return 40;
}

void load_level(int index, Grid& g);
void load_level_from_text(const char* text, Grid& out);
std::string export_level_to_text(const LevelInfo& info);

} // namespace baba
