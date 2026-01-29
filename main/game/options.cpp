/*
===============================================================================
  options.cpp — Menu OPTIONS du moteur Baba
-------------------------------------------------------------------------------
  Rôle :
    - Afficher et gérer le menu OPTIONS.
    - Sous-menus :
        * Audio : volumes, mute global, tests, choix musique
        * Niveau : sélection du niveau (normal + custom)
        * Éditeur : accès à l’éditeur embarqué (à implémenter)
    - Sauvegarder/charger les options via NVS (à implémenter)
===============================================================================
*/

#include <cstdio>
#include <cstring>

#include "options.h"
#include "core/graphics.h"
#include "core/input.h"
#include "core/audio.h"
#include "music_map.h"
#include "game/game.h"
#include "game/levels.h"
#include "esp_timer.h"

namespace baba {

// ============================================================================
//  États internes du menu
// ============================================================================
enum class OptionsPage {
    ROOT,
    AUDIO,
    AUDIO_TEST,
    AUDIO_MUSIC,
    LEVEL_SELECT,
    EDITOR
};

static OptionsPage page = OptionsPage::ROOT;
static int cursor = 0;
static int selectedLevel = 0;
static MusicID forcedMusic = MusicID::NONE;


// ============================================================================
//  Sauvegarde / chargement (NVS)
// ============================================================================
static const char* OPTIONS_FILE = "/babaisyou/options.cfg";

void options_init() {
    FILE* f = fopen(OPTIONS_FILE, "r");
    if (!f) {
        // Valeurs par défaut
        soundEnabled = true;
        g_audio_settings.music_volume = 80;
        g_audio_settings.sfx_volume   = 80;
        forcedMusic = MusicID::NONE;
        return;
    }

    char key[32];
    int value;

    while (fscanf(f, "%31[^=]=%d\n", key, &value) == 2) {

        if (strcmp(key, "sound") == 0)
            soundEnabled = (value != 0);

        else if (strcmp(key, "music") == 0)
            g_audio_settings.music_volume = value;

        else if (strcmp(key, "sfx") == 0)
            g_audio_settings.sfx_volume = value;

        else if (strcmp(key, "forced") == 0)
            forcedMusic = static_cast<MusicID>(value);
    }

    fclose(f);

    // Appliquer immédiatement
    if (soundEnabled) {
        audio_set_music_volume(g_audio_settings.music_volume);
        audio_set_sfx_volume(g_audio_settings.sfx_volume);
    } else {
        audio_set_music_volume(0);
        audio_set_sfx_volume(0);
    }
}

void options_save() {
    FILE* f = fopen(OPTIONS_FILE, "w");
    if (!f) return;

    fprintf(f, "sound=%d\n", soundEnabled ? 1 : 0);
    fprintf(f, "music=%d\n", g_audio_settings.music_volume);
    fprintf(f, "sfx=%d\n",   g_audio_settings.sfx_volume);
    fprintf(f, "forced=%d\n", (int)forcedMusic);

    fclose(f);
}


// ============================================================================
//  Helpers d’affichage
// ============================================================================
static void draw_item(int y, const char* text, bool selected) {
    gfx_text_center(y, text, selected ? COLOR_YELLOW : COLOR_WHITE);
}

// ============================================================================
//  PAGE ROOT
// ============================================================================
static void page_root(const Keys& k) {
    const char* items[] = {
        "Audio",
        "Choisir niveau",
        "Editeur",
        "Retour"
    };
    const int count = 4;

    if (k.up    && input_ready()) cursor = (cursor + count - 1) % count;
    if (k.down  && input_ready()) cursor = (cursor + 1) % count;

    if (k.A && input_ready()) {
        if (cursor == 0) { page = OptionsPage::AUDIO;        cursor = 0; }
        if (cursor == 1) { page = OptionsPage::LEVEL_SELECT; cursor = 0; }
        if (cursor == 2) { page = OptionsPage::EDITOR;       cursor = 0; }
        if (cursor == 3) { game_mode() = GameMode::Playing; 
							page = OptionsPage::ROOT;
							cursor = 0;
						 }

    }

    gfx_clear(COLOR_BLACK);
    gfx_text_center(40, "OPTIONS", COLOR_WHITE);

    for (int i = 0; i < count; i++)
        draw_item(100 + i * 20, items[i], cursor == i);

    gfx_flush();
}

// ============================================================================
//  PAGE AUDIO
// ============================================================================
static void page_audio(const Keys& k) {
    const int count = 6;

    if (k.up    && input_ready()) cursor = (cursor + count - 1) % count;
    if (k.down  && input_ready()) cursor = (cursor + 1) % count;

    // Réglage volumes musique/SFX
    if (cursor <= 1) {
        int* volumes[] = {
            &g_audio_settings.music_volume,
            &g_audio_settings.sfx_volume
        };

        if (k.left  && input_ready() && *volumes[cursor] > 0)
            (*volumes[cursor])--;

        if (k.right && input_ready() && *volumes[cursor] < 100)
            (*volumes[cursor])++;

        if (cursor == 0)
            audio_set_music_volume(*volumes[0]);
        if (cursor == 1)
            audio_set_sfx_volume(*volumes[1]);

        options_save();
    }

    // Toggle son global
    if (cursor == 2 && k.A && input_ready()) {
        soundEnabled = !soundEnabled;
        options_save();

        if (soundEnabled) {
            audio_set_music_volume(g_audio_settings.music_volume);
            audio_set_sfx_volume(g_audio_settings.sfx_volume);
        } else {
            audio_set_music_volume(0);
            audio_set_sfx_volume(0);
        }
    }

    // Navigation sous-pages
    if (k.A && input_ready()) {
        if (cursor == 3) page = OptionsPage::AUDIO_TEST;
        if (cursor == 4) page = OptionsPage::AUDIO_MUSIC;
        if (cursor == 5) page = OptionsPage::ROOT;
    }

    gfx_clear(COLOR_BLACK);
    gfx_text_center(40, "AUDIO", COLOR_WHITE);

    char buf[32];
    std::snprintf(buf, sizeof(buf), "Musique: %d", g_audio_settings.music_volume);
    draw_item(100, buf, cursor == 0);

    std::snprintf(buf, sizeof(buf), "SFX: %d", g_audio_settings.sfx_volume);
    draw_item(120, buf, cursor == 1);

    draw_item(140, soundEnabled ? "Son: ON" : "Son: OFF", cursor == 2);
    draw_item(170, "Tester sons",     cursor == 3);
    draw_item(190, "Choisir musique", cursor == 4);
    draw_item(210, "Retour",          cursor == 5);

    gfx_flush();
}

// ============================================================================
//  PAGE TEST SONS
// ============================================================================
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

// ============================================================================
//  PAGE CHOIX MUSIQUE
// ============================================================================
static void page_audio_music(const Keys& k) {
    const int musicCount = 7;

    if (k.up    && input_ready()) cursor = (cursor + musicCount - 1) % musicCount;
    if (k.down  && input_ready()) cursor = (cursor + 1) % musicCount;

    if (k.A && input_ready()) {
        forcedMusic = static_cast<MusicID>(cursor);
        game_set_forced_music(forcedMusic, true);
        audio_request_music(forcedMusic);
        options_save();
    }

    if (k.B && input_ready()) {
        forcedMusic = MusicID::NONE;
        game_set_forced_music(MusicID::NONE, false);
        page = OptionsPage::AUDIO;
        options_save();
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

// ============================================================================
//  PAGE SELECTION NIVEAU (normal + custom)
// ============================================================================
static void page_level_select(const Keys& k) {

    const int normalLevels = levels_count();
    const int totalLevels = normalLevels + 3; // Custom 1..3

    if (k.left  && input_ready())
        selectedLevel = (selectedLevel + totalLevels - 1) % totalLevels;

    if (k.right && input_ready())
        selectedLevel = (selectedLevel + 1) % totalLevels;

    if (k.A && input_ready()) {
        int index = selectedLevel;

        if (index >= normalLevels)
            index = -(index - normalLevels + 1); // → -1, -2, -3

        game_load_level(index);
        game_mode() = GameMode::Playing;
    }

    if (k.B && input_ready())
        page = OptionsPage::ROOT;

    gfx_clear(COLOR_BLACK);
    gfx_text_center(40, "SELECTION NIVEAU", COLOR_WHITE);

    char buf[32];

    if (selectedLevel < normalLevels)
        std::snprintf(buf, sizeof(buf), "Niveau : %d", selectedLevel);
    else
        std::snprintf(buf, sizeof(buf), "Custom %d", selectedLevel - normalLevels + 1);

    gfx_text_center(120, buf, COLOR_YELLOW);
    gfx_text_center(200, "A = Jouer   B = Retour", COLOR_WHITE);

    gfx_flush();
}

// ============================================================================
//  PAGE EDITEUR (placeholder)
// ============================================================================
static void page_editor(const Keys& k) {
    gfx_clear(COLOR_BLACK);
    gfx_text_center(40, "EDITEUR", COLOR_WHITE);
    gfx_text_center(120, "Aucun editeur implemente", COLOR_YELLOW);
    gfx_text_center(200, "B = Retour", COLOR_WHITE);

    if (k.B && input_ready())
        page = OptionsPage::ROOT;

    gfx_flush();
}

// ============================================================================
//  Fonction principale appelée depuis task_game
// ============================================================================
void options_update() {
    Keys k = g_keys;

    switch (page) {
        case OptionsPage::ROOT:         page_root(k); break;
        case OptionsPage::AUDIO:        page_audio(k); break;
        case OptionsPage::AUDIO_TEST:   page_audio_test(k); break;
        case OptionsPage::AUDIO_MUSIC:  page_audio_music(k); break;
        case OptionsPage::LEVEL_SELECT: page_level_select(k); break;
        case OptionsPage::EDITOR:       page_editor(k); break;
    }
}

} // namespace baba
