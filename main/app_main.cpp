//===============================================================================
// app_main.cpp — Point d’entrée du moteur BabaIsU (AKA Edition)
//-------------------------------------------------------------------------------
//  Rôle :
//    - Initialiser l’ensemble du hardware (LCD, audio, input, SD, expander).
//    - Charger les ressources globales (sprites, paramètres audio, niveaux).
//    - Lancer les tâches FreeRTOS (jeu, input, audio si nécessaire).
//    - Fournir une boucle idle propre et stable.
//
//  Contexte :
//    Ce projet est une réécriture moderne du moteur "Baba Is You" adaptée à
//    l’AKA. Il s’appuie sur :
//      - un moteur de grille multi-objets,
//      - un moteur de règles dynamique (SUBJECT IS PROPERTY),
//      - un moteur de mouvement complet,
//      - un rendu basé sur un atlas 16×16,
//      - un pipeline DMA LCD optimisé,
//      - un système audio I2S propre et indépendant du framerate.
//
//  Objectifs :
//    - Architecture claire, modulaire, extensible.
//    - Séparation stricte des responsabilités :
//        * app_main.cpp : initialisation + lancement des tâches
//        * task_game.cpp : boucle de jeu (40 FPS)
//        * core/* : moteur logique (grille, règles, mouvement)
//        * game/* : gestion des niveaux, état global
//        * assets/* : sprites, audio, données
//    - Code lisible, documenté, facile à maintenir.
//
//  Auteur : Jean-Charles LEBEAU (Jicehel)
//
//  Création : 01/2026
//===============================================================================

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// -----------------------------------------------------------------------------
//  Hardware AKA (lib Gamebuino AKA)
// -----------------------------------------------------------------------------
#include "gb_core.h"
#include "gb_graphics.h"

// -----------------------------------------------------------------------------
//  Core du moteur
// -----------------------------------------------------------------------------
#include "core/graphics.h"
#include "core/input.h"
#include "core/audio.h"
#include "core/sprites.h"
#include "core/persist.h"

// -----------------------------------------------------------------------------
//  Logique de jeu
// -----------------------------------------------------------------------------
#include "game/game.h"

// -----------------------------------------------------------------------------
//  Tâches FreeRTOS
// -----------------------------------------------------------------------------
#include "tasks/task_game.h"
#include "tasks/task_input.h"
#include "tasks/task_audio.h"

gb_core g_core;


void hardware_init()
{
    printf("\n=== HARDWARE INIT (AKA Edition) ===\n");

    // ------------------------------------------------------------
    // 1) Init Gamebuino AKA core (timers, ADC, I2C, expander, SD, LCD, audio)
    //    Cette fonction appelle :
    //      - gb_ll_system_init()
    //      - gb_ll_adc_init()
    //      - gb_ll_i2c_init()
    //      - gb_ll_expander_init()
    //      - gb_ll_sd_init()
    //      - gb_ll_lcd_init()
    //      - gb_ll_audio_init()
    // ------------------------------------------------------------
    printf("[HW] gb_core.init()...\n");
    g_core.init();

    // ------------------------------------------------------------
    // 2) Audio haut-niveau BabaIsU (wrap lib AKA) remplacé par l'init dans la tâche
    // ------------------------------------------------------------
    // printf("[HW] baba::audio_init()...\n");
    // baba::audio_init();

    // ------------------------------------------------------------
    // 3) Graphics haut-niveau BabaIsU (wrap gb_graphics / LCD)
    // ------------------------------------------------------------
    printf("[HW] baba::gfx_init()...\n");
    baba::gfx_init();

    // ------------------------------------------------------------
    // 4) Input (wrap expander + joystick via core/input.h)
    // ------------------------------------------------------------
    printf("[HW] input_init()...\n");
    input_init();

    // ------------------------------------------------------------
    // 5) Sprites + atlas
    // ------------------------------------------------------------
    printf("[HW] sprites_init()...\n");
    baba::sprites_init();

    // ------------------------------------------------------------
    // 6) Persist (sauvegardes)
    // ------------------------------------------------------------
    printf("[HW] persist_init()...\n");
    baba::persist_init();

    printf("=== HARDWARE INIT DONE ===\n\n");
}


// ============================================================================
//  POINT D’ENTRÉE AKA
// ============================================================================
extern "C" void app_main(void)
{
    printf("\n=============================================\n");
    printf("  BabaIsU — Moteur Puzzle AKA Edition\n");
    printf("  (c) Jean-Charles — Architecture modulaire\n");
    printf("=============================================\n\n");

    hardware_init();

    // -------------------------------------------------------------------------
    //  Création des tâches FreeRTOS
    // -------------------------------------------------------------------------

    // Tâche audio (cadence stable, init + pool)
    xTaskCreatePinnedToCore(
        task_audio,
        "AudioTask",
        4096,
        nullptr,
        4,
        nullptr,
        1 // Core 0 : idéal pour l’audio (I2S + DMA)
    );


    // Tâche principale du jeu (40 FPS)
    xTaskCreatePinnedToCore(
        baba::task_game,
        "GameTask",
        8192,
        nullptr,
        6,
        nullptr,
        1 // Core 1 : gameplay + rendu
    );

    printf("[BabaIsU] Tâches lancées. Entrée en idle loop.\n");

    // -------------------------------------------------------------------------
    //  Boucle idle
    // -------------------------------------------------------------------------
    while (true)
        vTaskDelay(pdMS_TO_TICKS(1000));
}

