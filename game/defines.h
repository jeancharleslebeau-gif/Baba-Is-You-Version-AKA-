/*
===============================================================================
  defines.h — Macros de correspondance pour les niveaux META
-------------------------------------------------------------------------------
  Rôle :
    - Fournir des alias en majuscules (WALL, FLAG, ROCK, etc.) compatibles
      avec les tableaux de niveaux copiés depuis la META.
    - Mapper ces alias vers les valeurs de l'enum ObjectType défini dans grid.h.
    - Permettre de garder les données META inchangées tout en les compilant
      dans le projet AKA.

  Notes :
    - Les tableaux levelX[] sont déclarés comme const uint8_t.
    - Chaque macro convertit un ObjectType en uint8_t via static_cast.
    - Exemple : WALL → ObjectType::Wall → uint8_t.
===============================================================================
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

#define W_IS    static_cast<uint8_t>(ObjectType::Text_Is  )
#define W_YOU   static_cast<uint8_t>(ObjectType::Text_You )
#define W_WIN   static_cast<uint8_t>(ObjectType::Text_Win )
#define W_STOP  static_cast<uint8_t>(ObjectType::Text_Stop)
#define W_PUSH  static_cast<uint8_t>(ObjectType::Text_Push)
#define W_KILL  static_cast<uint8_t>(ObjectType::Text_Kill)
#define W_SINK  static_cast<uint8_t>(ObjectType::Text_Sink)
#define W_SWAP  static_cast<uint8_t>(ObjectType::Text_Swap)
