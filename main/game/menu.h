/*
===============================================================================
  menu.h — Menu principal du moteur Baba
-------------------------------------------------------------------------------
  Rôle :
    - Afficher et gérer le menu principal.
    - Navigation :
        * Jouer
        * Options
        * Quitter / Retour
    - Déléguer les actions à game.cpp et options.cpp.
===============================================================================
*/

#pragma once
#include "core/input.h"

namespace baba {

void menu_init();
void menu_update();
void menu_draw();

} // namespace baba
