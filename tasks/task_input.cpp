/*
============================================================
  task_input.cpp — Tâche input (100 Hz)
------------------------------------------------------------
Cette tâche exécute :
 - input_poll(g_keys)

Elle met à jour l’état global des touches 100 fois par seconde,
ce qui garantit une excellente réactivité même si le jeu tourne
à 60 FPS.
============================================================
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
    const TickType_t period = pdMS_TO_TICKS(16); // 60 Hz
    TickType_t last_wake = xTaskGetTickCount();
	printf("[InputTask] started\n");

    while (true)
    {
        input_poll(g_keys);
		
		if (g_keys.raw != 0) {
			printf("[RAW] %04lX  A=%d B=%d MENU=%d  U=%d D=%d L=%d R=%d\n",
				   (unsigned long)g_keys.raw,
				   g_keys.A, g_keys.B, g_keys.MENU,
				   g_keys.up, g_keys.down, g_keys.left, g_keys.right);
		}

        // Attendre précisément 10 ms (100 Hz)
        vTaskDelayUntil(&last_wake, period);
    }
}

} // namespace baba
