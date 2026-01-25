/*
===============================================================================
  rules.h — Analyse des règles et transformations (Baba-style)
-------------------------------------------------------------------------------
  Rôle :
    - Scanner la grille pour détecter les règles de type :
        * SUBJECT IS STATUS   (BABA IS YOU, ROCK IS PUSH, LAVA IS HOT…)
        * SUBJECT IS SUBJECT  (ROCK IS WALL, LAVA IS FLAG, EMPTY IS ROCK…)
    - Remplir :
        * PropertyTable      → propriétés logiques (YOU, PUSH, STOP, HOT…)
        * TransformSetTable  → transformations multiples :
              - ROCK IS WALL + ROCK IS FLAG → ROCK devient WALL + FLAG
              - chaînes : ROCK→WALL→FLAG
              - cycles : A→B→C→A (gérés proprement)
    - Fournir une fonction d’application des transformations sur la grille :
        * apply_transformations(Grid&, TransformSetTable&)
          → applique les transformations d’objets avant les déplacements.
===============================================================================
*/

#pragma once
#include "types.h"
#include "core/grid.h"

namespace baba {

/*
===============================================================================
  Analyse des règles
===============================================================================
*/

// Réinitialise les propriétés et les transformations à partir de zéro.
void rules_reset(PropertyTable& props, TransformSetTable& sets);

// Analyse la grille et remplit :
//   - props : propriétés logiques (YOU, PUSH, STOP, HOT…)
//   - sets  : transformations multiples (ROCK→WALL + ROCK→FLAG, etc.)
void rules_parse(const Grid& g,
                 PropertyTable& props,
                 TransformSetTable& sets);

/*
===============================================================================
  Application des transformations
===============================================================================
*/

// Applique les transformations d’objets à la grille, en utilisant
// TransformSetTable :
//   - gère les transformations multiples (duplication d’objets)
//   - gère les chaînes (ROCK→WALL→FLAG)
//   - gère les cycles (A→B→C→A) via une résolution sûre
//   - gère EMPTY IS X (remplissage des cases vides)
void apply_transformations(Grid& g,
                           const TransformSetTable& sets);

} // namespace baba
