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
#include <array>
#include "types.h"
#include "grid.h"

namespace baba {

// -----------------------------------------------------------------------------
//  Fonctions exposées par rules.cpp
// -----------------------------------------------------------------------------
void rules_reset(PropertyTable& table);
void rules_parse(const Grid& g, PropertyTable& table);

} // namespace baba
