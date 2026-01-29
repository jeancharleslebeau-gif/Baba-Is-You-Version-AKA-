/*
===============================================================================
  task_input.cpp — Tâche input (100 Hz)
-------------------------------------------------------------------------------
  Rôle :
    - Interroger le module d’entrée (input_poll).
    - Mettre à jour l’état global des touches (g_keys).
    - Fournir une fréquence d’échantillonnage élevée (100 Hz) pour garantir
      une excellente réactivité, même si la boucle de jeu tourne à 60 FPS.

  Notes :
    - Cette tâche tourne en parallèle du moteur de jeu.
    - input_poll() lit l’état matériel des boutons et du joystick.
    - Le debug RAW peut être activé pour visualiser les états bruts.

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#include "task_input.h"
#include "core/input.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace baba {

void task_input(void* param)
{
    const TickType_t period = pdMS_TO_TICKS(16); // ~60 Hz
    TickType_t last_wake = xTaskGetTickCount();

    printf("[InputTask] started\n");

    while (true)
    {
        // Lire l’état des touches
        input_poll(g_keys);

        // Debug optionnel
        /* if (g_keys.raw != 0) {
            printf("[RAW] %04lX  A=%d B=%d MENU=%d  U=%d D=%d L=%d R=%d\n",
                   (unsigned long)g_keys.raw,
                   g_keys.A, g_keys.B, g_keys.MENU,
                   g_keys.up, g_keys.down, g_keys.left, g_keys.right);
        } */

        // Attendre précisément la période définie
        vTaskDelayUntil(&last_wake, period);
    }
}

} // namespace baba
