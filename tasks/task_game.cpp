/*
===============================================================================
  task_game.cpp — Boucle de jeu principale (40 FPS)
-------------------------------------------------------------------------------
  Rôle :
	- Exécuter la logique du jeu selon l’état courant (GameMode).
	- Appeler :
		* game_show_title()   → écran de titre
		* game_update()       → logique de déplacement + règles
		* game_draw()         → rendu de la grille
	- Gérer les transitions :
		* Title → Playing
		* Playing → Win / Dead
		* Win / Dead → Restart
		* Menu → retour vers Playing
	- Maintenir une cadence stable (~40 FPS).

  Notes :
	- Les entrées sont lues dans task_input.cpp et stockées dans g_keys.
	- Cette tâche tourne sur le core 1 (défini dans app_main.cpp).
	- Le moteur audio est indépendant (I2S + DMA).

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#include <cstdio> // pour printf, sprintf
#include <cstring>
#include "task_game.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "core/input.h"
#include "core/graphics.h"
#include "core/audio.h"

#include "game/game.h" // game_state(), game_mode(), game_update(), game_draw(), game_load_level()
#include "game/config.h"
#include "game/options.h"

namespace baba
{

	// -----------------------------------------------------------------------------
	//  Variables internes pour gérer les transitions et les appuis de touches
	// -----------------------------------------------------------------------------
	static GameMode s_prevMode = GameMode::Title;
	static Keys s_prevKeys{};

	// Fonctions utilitaires pour détecter les appuis (front montant)
	static inline bool pressed_A(const Keys &now) { return now.A && !s_prevKeys.A; }
	static inline bool pressed_B(const Keys &now) { return now.B && !s_prevKeys.B; }
	static inline bool pressed_MENU(const Keys &now) { return now.MENU && !s_prevKeys.MENU; }

	static inline bool pressed_UP(const Keys &now) { return now.up && !s_prevKeys.up; }
	static inline bool pressed_DOWN(const Keys &now) { return now.down && !s_prevKeys.down; }
	static inline bool pressed_LEFT(const Keys &now) { return now.left && !s_prevKeys.left; }
	static inline bool pressed_RIGHT(const Keys &now) { return now.right && !s_prevKeys.right; }

	// -----------------------------------------------------------------------------
	//  Actions à effectuer lors de l’entrée dans un nouvel état
	// -----------------------------------------------------------------------------
	static void on_enter_mode(GameMode m)
	{
		switch (m)
		{
		case GameMode::Title:
			gfx_clear(COLOR_BLACK);
			game_show_title();
			gfx_text_center(200, "Press A to start", COLOR_WHITE);
			gfx_flush();
			break;

		case GameMode::Playing:
			gfx_clear(COLOR_BLACK);
			gfx_flush();
			break;

		case GameMode::Win:
			break;

		case GameMode::Dead:
			break;

		case GameMode::Menu:
			// Rien de spécial à l’entrée, ou initialisation fade/menu
			break;
		}
	}

	// -----------------------------------------------------------------------------
	//  Tâche de jeu principale (cadence ~40 FPS)
	// -----------------------------------------------------------------------------
	void task_game(void *)
	{
		printf("[GameTask] Démarrage de la boucle de jeu.\n");

		// Démarre en mode Title
		game_mode() = GameMode::Title;
		on_enter_mode(game_mode());
		s_prevMode = game_mode();

		while (true)
		{
			Keys k = g_keys;

			// Détecte changement d’état et applique les actions d’entrée
			if (game_mode() != s_prevMode)
			{
				on_enter_mode(game_mode());
				s_prevMode = game_mode();
			}

			// Routage selon l’état courant
			switch (game_mode())
			{
			case GameMode::Title:
				// Attente appui sur A pour lancer le jeu
				if (pressed_A(k))
				{
					game_load_level(0);
					game_mode() = GameMode::Playing;
				}
				break;

			case GameMode::Playing:
				// Logique de jeu
				game_update();

				if (game_state().hasWon)
				{
					game_mode() = GameMode::Win;
					break;
				}
				if (game_state().hasDied)
				{
					game_mode() = GameMode::Dead;
					break;
				}

				if (pressed_MENU(k))
				{
					fade_out();
					game_mode() = GameMode::Menu;
					break;
				}
				break;

			case GameMode::Win:
			{
				const char* msg = "YOU WIN!";
				const int y_text = 100;               // position verticale du texte
				const int pad_x = 8;                  // marge horizontale autour du texte
				const int pad_y = 6;                  // marge verticale autour du texte

				// Calcul largeur texte (font 8x8)
				int tw = (int)strlen(msg) * 8;
				int rw = tw + pad_x * 2;
				int rx = (SCREEN_W - rw) / 2;
				if (rx < 0) rx = 0;
				if (rx + rw > SCREEN_W) rw = SCREEN_W - rx;

				int rh = 8 + pad_y * 2;               // 8px hauteur police + padding
				int ry = y_text - (8 / 2) - pad_y;    // centrer verticalement le rectangle sous le texte

				// Dessin one‑shot : rectangle sombre + texte par‑dessus
				gfx_fillRect(rx, ry, rw, rh, COLOR_BLACK);
				gfx_text_center(y_text, msg, COLOR_WHITE);
				gfx_text_center(140, "Press A to restart", COLOR_WHITE);
				gfx_flush(); // envoie une fois au LCD

				// Attente non bloquante de l'appui sur A
				while (!g_keys.A) {
					vTaskDelay(pdMS_TO_TICKS(25));
				}

				// Action après appui
				game_win_continue();
				game_mode() = GameMode::Playing;
				break;
			}

			// --- Écran Dead : rectangle sombre sous le texte, one‑shot ---
			case GameMode::Dead:
			{
				const char* msg = "YOU DIED!";
				const int y_text = 100;
				const int pad_x = 8;
				const int pad_y = 6;

				int tw = (int)strlen(msg) * 8;
				int rw = tw + pad_x * 2;
				int rx = (SCREEN_W - rw) / 2;
				if (rx < 0) rx = 0;
				if (rx + rw > SCREEN_W) rw = SCREEN_W - rx;

				int rh = 8 + pad_y * 2;
				int ry = y_text - (8 / 2) - pad_y;

				gfx_fillRect(rx, ry, rw, rh, COLOR_BLACK);
				gfx_text_center(y_text, msg, COLOR_RED);
				gfx_text_center(140, "Press A to return to title", COLOR_WHITE);
				gfx_flush();

				while (!g_keys.A) {
					vTaskDelay(pdMS_TO_TICKS(25));
				}

				game_mode() = GameMode::Title;
				break;
			}


			case GameMode::Menu:
			{
				// Menu OPTIONS complet (audio, test sons, musique, sélection de niveau) 
				options_update();
				break;
			}
			} // fin du switch

			// Rendu uniquement en mode Playing
			if (game_mode() == GameMode::Playing)
			{
				game_draw();
				gfx_flush(); // assure que le rendu de la grille est affiché à chaque frame
			}

			// Sauvegarde des touches pour détection front montant
			s_prevKeys = k;
			
			// Mise à jour du système 
			audio_update();

			// Cadence stable (~40 FPS)
			vTaskDelay(pdMS_TO_TICKS(25));
		}
	}

} // namespace baba
