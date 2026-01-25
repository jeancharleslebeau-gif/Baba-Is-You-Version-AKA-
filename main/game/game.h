/*
===============================================================================
  game.h — Gestion de l’état global du jeu
-------------------------------------------------------------------------------
  Rôle :
    - Définir la structure GameState, qui contient l’état complet du jeu :
        * la grille (Grid)
        * les propriétés dynamiques (PropertyTable)
        * les transformations d’objets (TransformTable)
        * les flags de victoire / mort
    - Déclarer les fonctions principales du moteur de jeu :
        * game_init()       → initialisation globale
        * game_load_level() → chargement d’un niveau + analyse des règles
        * game_update()     → logique par frame (déplacements + règles)
        * game_draw()       → rendu graphique
    - Déclarer les fonctions de gestion d’état (mode, transitions).
    - Servir d’interface entre la boucle principale (app_main)
      et le moteur logique BabaIsU.

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#pragma once
#include "types.h"
#include "core/grid.h"
#include "core/rules.h"
#include "music_map.h"

namespace baba {

MusicID game_get_forced_music();
void    game_set_forced_music(MusicID id, bool persistAcrossLevels);

/*
===============================================================================
  GameState — Structure contenant l’état complet du jeu
===============================================================================
*/
struct GameState {
    Grid grid;                    // Grille de jeu (objets et mots)
    PropertyTable props;          // Propriétés dynamiques (YOU, PUSH, STOP, HOT…)
    TransformSetTable transforms; // Transformations d’objets (ROCK→WALL, EMPTY→ROCK…)
    bool hasWon  = false;         // Flag de victoire
    bool hasDied = false;         // Flag de mort
    int currentLevel = 0;         // Niveau courant (pour restart/advance)
};

/*
===============================================================================
  Système d’états du jeu (Title / Playing / Win / Dead / Menu)
===============================================================================
*/
enum class GameMode {
    Title,
    Playing,
    Win,
    Dead,
    Menu
};

/*
===============================================================================
  Fonctions principales du moteur de jeu
===============================================================================
*/

// Initialise l’état global du jeu et charge le premier niveau
void game_init();

// Charge un niveau par index et recalcule les propriétés + transformations
void game_load_level(int index);

// Met à jour la logique du jeu (déplacements, règles, états)
void game_update();

// Dessine la grille complète à l’écran
void game_draw();

// Retourne une référence vers l’état global du jeu
GameState& game_state();

// Retourne une référence modifiable vers l'état courant du jeu 
GameMode& game_mode();

// Affiche l’écran de titre
void game_show_title();

// Transition visuelle (fondu)
void fade_in(int delayMs = 30, int steps = 10); 
void fade_out(int delayMs = 30, int steps = 10);

// Helpers de progression 
void game_win_continue();        // avance au niveau suivant 
void game_restart_after_death(); // relance le niveau courant

} // namespace baba
