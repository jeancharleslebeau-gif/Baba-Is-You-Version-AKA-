/*
===============================================================================
  movement.h — Moteur de déplacement (résolution des mouvements)
-------------------------------------------------------------------------------
  Rôle :
    - Appliquer les déplacements des objets ayant la propriété YOU.
    - Gérer STOP, PUSH et les chaînes de PUSH.
    - Détecter WIN, KILL, SINK après mouvement.
    - Retourner un MoveResult indiquant victoire ou mort.

  Notes :
    - Le moteur est volontairement minimal pour un prototype propre.
    - Extensions possibles :
        * MOVE (déplacement automatique)
        * SHIFT (glissement)
        * TELE (téléportation)
        * PULL (tirer au lieu de pousser)
        * etc.

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#pragma once
#include "types.h"
#include "grid.h"
#include "rules.h"

namespace baba {

/*
===============================================================================
  MoveResult — Résultat d’un déplacement
-------------------------------------------------------------------------------
  Rôle :
    - Indiquer si le mouvement a entraîné une victoire ou une mort.
===============================================================================
*/
struct MoveResult {
    bool hasWon  = false;
    bool hasDied = false;
};

/*
===============================================================================
  Helpers pour tester les propriétés
-------------------------------------------------------------------------------
  Rôle :
    - Simplifier l’accès aux champs de Properties.
    - Utilisés par movement.cpp et potentiellement ailleurs.
===============================================================================
*/
inline bool isYou(const Properties& p)   { return p.you; }
inline bool isPush(const Properties& p)  { return p.push; }
inline bool isStop(const Properties& p)  { return p.stop; }
inline bool isWin(const Properties& p)   { return p.win; }
inline bool isSink(const Properties& p)  { return p.sink; }
inline bool isKill(const Properties& p)  { return p.kill; }

/*
===============================================================================
  step()
-------------------------------------------------------------------------------
  Rôle :
    - Appliquer un déplacement dx/dy à tous les objets YOU.
    - Résoudre les PUSH.
    - Appliquer les effets post-mouvement.

  Paramètres :
    - grid  : grille de jeu à modifier.
    - props : propriétés dynamiques (YOU, PUSH, STOP, WIN…).
    - dx,dy : direction du mouvement.

  Retour :
    - MoveResult indiquant victoire ou mort.
===============================================================================
*/
MoveResult step(Grid& grid, const PropertyTable& props, const TransformSetTable& transforms, int dx, int dy);

} // namespace baba
