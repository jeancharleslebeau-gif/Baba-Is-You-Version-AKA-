/*
===============================================================================
  rules.h — Table des propriétés dynamiques (moteur Baba Is You)
-------------------------------------------------------------------------------
  Rôle :
    - Définir la structure Properties avec les attributs (YOU, PUSH, STOP…).
    - Définir PropertyTable = tableau de Properties par ObjectType.
    - Fournir les fonctions de parsing dans rules.cpp.
===============================================================================
*/

#pragma once
#include "grid.h"
#include <array>

namespace baba {

// -----------------------------------------------------------------------------
//  Ensemble de propriétés pour un type d’objet
// -----------------------------------------------------------------------------
struct Properties {
    bool isYou   = false;
    bool isPush  = false;
    bool isStop  = false;
    bool isWin   = false;
    bool isSink  = false;
    bool isKill  = false;
    bool isHot   = false;
    bool isMelt  = false;
    bool isMove  = false;
    bool isOpen  = false;
    bool isShut  = false;
    bool isFloat = false;
};

// -----------------------------------------------------------------------------
//  Table complète : une entrée par ObjectType
// -----------------------------------------------------------------------------
using PropertyTable = std::array<Properties, (int)ObjectType::Count>;

// -----------------------------------------------------------------------------
//  Fonctions exposées par rules.cpp
// -----------------------------------------------------------------------------
void rules_reset(PropertyTable& table);
void rules_parse(const Grid& g, PropertyTable& table);

} // namespace baba
