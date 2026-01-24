/*
===============================================================================
  options.h — Menu OPTIONS du moteur Baba
-------------------------------------------------------------------------------
  Rôle :
    - Gérer l’affichage et la navigation dans le menu OPTIONS.
    - Permettre :
        * réglage des volumes audio
        * test des sons
        * choix manuel de la musique
        * sélection d’un niveau
        * sauvegarde/chargement des options
===============================================================================
*/

#pragma once
#include <cstdint>

namespace baba {

void options_init();      // charge les options depuis NVS
void options_save();      // sauvegarde dans NVS
void options_update();    // logique + rendu du menu OPTIONS

} // namespace baba
