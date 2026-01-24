	/*
	===============================================================================
	  options.cpp — Menu OPTIONS du moteur Baba
	-------------------------------------------------------------------------------
	  Rôle :
		- Afficher et gérer le menu OPTIONS.
		- Sous-menus :
			* Audio : volumes, tests, choix musique
			* Niveau : sélection du niveau
		- Sauvegarder/charger les options via NVS (à implémenter)
	===============================================================================
	*/

	#include <cstdio>           // sprintf
	#include "options.h"
	#include "core/graphics.h"
	#include "core/input.h"
	#include "core/audio.h"
	#include "music_map.h"
	#include "game/game.h"
	#include "game/levels.h"
	#include "esp_timer.h"

	namespace baba {

	// -----------------------------------------------------------------------------
	//  États internes du menu
	// -----------------------------------------------------------------------------
	enum class OptionsPage {
		ROOT,
		AUDIO,
		AUDIO_TEST,
		AUDIO_MUSIC,
		LEVEL_SELECT
	};

	static OptionsPage page = OptionsPage::ROOT;
	static int cursor = 0;
	static int selectedLevel = 0;
	static MusicID forcedMusic = MusicID::NONE;

	// -----------------------------------------------------------------------------
	//  Anti-repeat pour les touches (évite de défiler trop vite)
	// -----------------------------------------------------------------------------
	static uint32_t lastInputTime = 0;
	static const uint32_t INPUT_COOLDOWN_MS = 120;

	static bool input_ready() {
		uint32_t now = esp_timer_get_time() / 1000; // en millisecondes
		if (now - lastInputTime < INPUT_COOLDOWN_MS)
			return false;
		lastInputTime = now;
		return true;
	}

	// -----------------------------------------------------------------------------
	//  Sauvegarde / chargement (NVS)
	// -----------------------------------------------------------------------------
	void options_init() {
		// TODO : charger depuis NVS
	}

	void options_save() {
		// TODO : sauvegarder dans NVS
	}

	// -----------------------------------------------------------------------------
	//  Affichage d’un item centré
	// -----------------------------------------------------------------------------
	static void draw_item(int y, const char* text, bool selected) {
		gfx_text_center(y, text, selected ? COLOR_YELLOW : COLOR_WHITE);
	}

	// -----------------------------------------------------------------------------
	//  Page : menu principal OPTIONS
	// -----------------------------------------------------------------------------
	static void page_root(const Keys& k) {
		const char* items[] = {
			"Audio",
			"Choisir niveau",
			"Retour"
		};
		const int count = 3;

		if (k.up    && input_ready()) cursor = (cursor + count - 1) % count;
		if (k.down  && input_ready()) cursor = (cursor + 1) % count;

		if (k.A && input_ready()) {
			if (cursor == 0) { page = OptionsPage::AUDIO;        cursor = 0; }
			if (cursor == 1) { page = OptionsPage::LEVEL_SELECT; cursor = 0; }
			if (cursor == 2) { game_mode() = GameMode::Playing; }
		}

		gfx_clear(COLOR_BLACK);
		gfx_text_center(40, "OPTIONS", COLOR_WHITE);

		for (int i = 0; i < count; i++)
			draw_item(100 + i * 20, items[i], cursor == i);

		gfx_flush();
	}

	// -----------------------------------------------------------------------------
	//  Page : Audio
	// -----------------------------------------------------------------------------
	static void page_audio(const Keys& k) {
		const int count = 5;

		if (k.up    && input_ready()) cursor = (cursor + count - 1) % count;
		if (k.down  && input_ready()) cursor = (cursor + 1) % count;

		// Réglage des volumes (0–100)
		if (cursor <= 1) {
			int* volumes[] = {
				&g_audio_settings.music_volume,
				&g_audio_settings.sfx_volume
			};

			if (k.left  && input_ready() && *volumes[cursor] > 0)
				(*volumes[cursor])--;

			if (k.right && input_ready() && *volumes[cursor] < 100)
				(*volumes[cursor])++;

			// Applique au backend
			if (cursor == 0)
				audio_set_music_volume(*volumes[0]);
			if (cursor == 1)
				audio_set_sfx_volume(*volumes[1]);
		}

		if (k.A && input_ready()) {
			if (cursor == 2) page = OptionsPage::AUDIO_TEST;
			if (cursor == 3) page = OptionsPage::AUDIO_MUSIC;
			if (cursor == 4) page = OptionsPage::ROOT;
		}

		gfx_clear(COLOR_BLACK);
		gfx_text_center(40, "AUDIO", COLOR_WHITE);

		char buf[32];
		std::snprintf(buf, sizeof(buf), "Musique: %d", g_audio_settings.music_volume);
		draw_item(100, buf, cursor == 0);

		std::snprintf(buf, sizeof(buf), "SFX: %d", g_audio_settings.sfx_volume);
		draw_item(120, buf, cursor == 1);

		draw_item(160, "Tester sons",     cursor == 2);
		draw_item(180, "Choisir musique", cursor == 3);
		draw_item(200, "Retour",          cursor == 4);

		gfx_flush();
	}

	// -----------------------------------------------------------------------------
	//  Page : test des sons
	// -----------------------------------------------------------------------------
	static void page_audio_test(const Keys& k) {
		const char* items[] = {
			"Move",
			"Push",
			"Win",
			"Lose",
			"Retour"
		};
		const int count = 5;

		if (k.up    && input_ready()) cursor = (cursor + count - 1) % count;
		if (k.down  && input_ready()) cursor = (cursor + 1) % count;

		if (k.A && input_ready()) {
			if (cursor == 0) audio_play_move();
			if (cursor == 1) audio_play_push();
			if (cursor == 2) audio_play_win();
			if (cursor == 3) audio_play_lose();
			if (cursor == 4) page = OptionsPage::AUDIO;
		}

		gfx_clear(COLOR_BLACK);
		gfx_text_center(40, "TEST SONS", COLOR_WHITE);

		for (int i = 0; i < count; i++)
			draw_item(100 + i * 20, items[i], cursor == i);

		gfx_flush();
	}

	// -----------------------------------------------------------------------------
	//  Page : choix de la musique
	// -----------------------------------------------------------------------------
	static void page_audio_music(const Keys& k) {
		const int musicCount = 7;

		if (k.up    && input_ready()) cursor = (cursor + musicCount - 1) % musicCount;
		if (k.down  && input_ready()) cursor = (cursor + 1) % musicCount;

		if (k.A && input_ready()) {
			forcedMusic = static_cast<MusicID>(cursor);
			audio_request_music(forcedMusic);   // ✔ thread-safe
		}

		if (k.B && input_ready()) {
			forcedMusic = MusicID::NONE;
			page = OptionsPage::AUDIO;
		}

		gfx_clear(COLOR_BLACK);
		gfx_text_center(40, "CHOIX MUSIQUE", COLOR_WHITE);

		const char* names[] = {
			"Baba Samba",
			"Baba Music 2",
			"Baba Cave",
			"Crystal",
			"Misthart",
			"WF Drago",
			"WF Mages"
		};

		for (int i = 0; i < musicCount; i++)
			draw_item(100 + i * 20, names[i], cursor == i);

		gfx_flush();
	}

		// -----------------------------------------------------------------------------
		//  Page : sélection du niveau
		// -----------------------------------------------------------------------------
		//
		//  Correction :
		//    - Remplacement du "maxLevel = 20" codé en dur.
		//    - Utilisation de levels_count() défini dans level.h.
		//    - Navigation circulaire correcte sur [0 .. levels_count()-1].
		//    - Code commenté pour éviter toute régression future.
		//
		// -----------------------------------------------------------------------------
		static void page_level_select(const Keys& k) {

			// Nombre total de niveaux disponibles (défini dans level.h)
			// Exemple : inline int levels_count() { return 40; }
			const int maxLevel = levels_count() - 1;   // dernier index valide

			// -------------------------------------------------------------------------
			// Navigation gauche/droite
			// -------------------------------------------------------------------------
			//
			// selectedLevel = (selectedLevel + maxLevel) % (maxLevel + 1)
			//   → équivalent à selectedLevel-- avec wrap vers maxLevel
			//
			// selectedLevel = (selectedLevel + 1) % (maxLevel + 1)
			//   → équivalent à selectedLevel++ avec wrap vers 0
			//
			// input_ready() évite les répétitions trop rapides.
			// -------------------------------------------------------------------------

			if (k.left  && input_ready())
				selectedLevel = (selectedLevel + maxLevel) % (maxLevel + 1);

			if (k.right && input_ready())
				selectedLevel = (selectedLevel + 1) % (maxLevel + 1);

			// -------------------------------------------------------------------------
			// Validation : charger le niveau sélectionné
			// -------------------------------------------------------------------------
			if (k.A && input_ready()) {
				game_load_level(selectedLevel);
				game_mode() = GameMode::Playing;
			}

			// Retour au menu principal
			if (k.B && input_ready())
				page = OptionsPage::ROOT;

			// -------------------------------------------------------------------------
			// Affichage
			// -------------------------------------------------------------------------
			gfx_clear(COLOR_BLACK);
			gfx_text_center(40, "SELECTION NIVEAU", COLOR_WHITE);

			char buf[32];
			std::snprintf(buf, sizeof(buf), "Niveau : %d", selectedLevel);
			gfx_text_center(120, buf, COLOR_YELLOW);

			gfx_text_center(200, "A = Jouer   B = Retour", COLOR_WHITE);

			gfx_flush();
		}


	// -----------------------------------------------------------------------------
	//  Fonction principale appelée depuis task_game
	// -----------------------------------------------------------------------------
	void options_update() {
		Keys k = g_keys;

		switch (page) {
			case OptionsPage::ROOT:         page_root(k); break;
			case OptionsPage::AUDIO:        page_audio(k); break;
			case OptionsPage::AUDIO_TEST:   page_audio_test(k); break;
			case OptionsPage::AUDIO_MUSIC:  page_audio_music(k); break;
			case OptionsPage::LEVEL_SELECT: page_level_select(k); break;
		}
	}

	} // namespace baba
