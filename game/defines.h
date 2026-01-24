/*
===============================================================================
  defines.h — Macros de correspondance pour les niveaux META
-------------------------------------------------------------------------------
*/

#pragma once
#include "core/grid.h"

// -----------------------------------------------------------------------------
// Objets physiques
// -----------------------------------------------------------------------------
#define EMPTY static_cast<uint8_t>(ObjectType::Empty)
#define WALL  static_cast<uint8_t>(ObjectType::Wall )
#define FLAG  static_cast<uint8_t>(ObjectType::Flag )
#define ROCK  static_cast<uint8_t>(ObjectType::Rock )
#define BABA  static_cast<uint8_t>(ObjectType::Baba )
#define LAVA  static_cast<uint8_t>(ObjectType::Lava )
#define GOOP  static_cast<uint8_t>(ObjectType::Goop )
#define LOVE  static_cast<uint8_t>(ObjectType::Love )

// Nouveaux objets simples
#define KEY   static_cast<uint8_t>(ObjectType::Key  )
#define DOOR  static_cast<uint8_t>(ObjectType::Door )
#define WATER static_cast<uint8_t>(ObjectType::Water)
#define ICE   static_cast<uint8_t>(ObjectType::Ice  )
#define BOX   static_cast<uint8_t>(ObjectType::Box  )

// -----------------------------------------------------------------------------
// Mots (TEXT_*)
// -----------------------------------------------------------------------------
#define W_EMPTY static_cast<uint8_t>(ObjectType::Text_Empty)
#define W_BABA  static_cast<uint8_t>(ObjectType::Text_Baba )
#define W_FLAG  static_cast<uint8_t>(ObjectType::Text_Flag )
#define W_ROCK  static_cast<uint8_t>(ObjectType::Text_Rock )
#define W_WALL  static_cast<uint8_t>(ObjectType::Text_Wall )
#define W_LAVA  static_cast<uint8_t>(ObjectType::Text_Lava )
#define W_GOOP  static_cast<uint8_t>(ObjectType::Text_Goop )
#define W_LOVE  static_cast<uint8_t>(ObjectType::Text_Love )

// Nouveaux mots (TEXT_* objets)
#define W_KEY   static_cast<uint8_t>(ObjectType::Text_Key  )
#define W_DOOR  static_cast<uint8_t>(ObjectType::Text_Door )
#define W_WATER static_cast<uint8_t>(ObjectType::Text_Water)
#define W_ICE   static_cast<uint8_t>(ObjectType::Text_Ice  )
#define W_BOX   static_cast<uint8_t>(ObjectType::Text_Box  )

// Verbes / propriétés
#define W_IS    static_cast<uint8_t>(ObjectType::Text_Is  )
#define W_YOU   static_cast<uint8_t>(ObjectType::Text_You )
#define W_WIN   static_cast<uint8_t>(ObjectType::Text_Win )
#define W_STOP  static_cast<uint8_t>(ObjectType::Text_Stop)
#define W_PUSH  static_cast<uint8_t>(ObjectType::Text_Push)
#define W_KILL  static_cast<uint8_t>(ObjectType::Text_Kill)
#define W_SINK  static_cast<uint8_t>(ObjectType::Text_Sink)
#define W_SWAP  static_cast<uint8_t>(ObjectType::Text_Swap)
#define W_HOT   static_cast<uint8_t>(ObjectType::Text_Hot )
#define W_MELT  static_cast<uint8_t>(ObjectType::Text_Melt)
#define W_MOVE  static_cast<uint8_t>(ObjectType::Text_Move)
#define W_OPEN  static_cast<uint8_t>(ObjectType::Text_Open)
#define W_SHUT  static_cast<uint8_t>(ObjectType::Text_Shut)
#define W_FLOAT static_cast<uint8_t>(ObjectType::Text_Float)

// Nouvelle propriété simple
#define W_PULL  static_cast<uint8_t>(ObjectType::Text_Pull)
