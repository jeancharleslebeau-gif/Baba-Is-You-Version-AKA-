#pragma once
#include <stdint.h>
#include "music_map.h"

namespace baba {

// ============================================================================
//  Initialisation du système audio
// ----------------------------------------------------------------------------
//  - Configure gb_audio_player et les pistes (PMF, tone, WAV)
//  - Initialise le pipeline I2S et l’ampli via gb_core
//  - Doit être appelé après gb_core.init()
// ============================================================================
void audio_init();


// ============================================================================
//  Mise à jour audio
// ----------------------------------------------------------------------------
//  - À appeler régulièrement (ex: dans la boucle principale du jeu)
//  - Permet au mixeur et aux pistes de progresser (PMF, SFX, WAV)
// ============================================================================
void audio_update();


// ============================================================================
//  Lecture de musiques PMF
// ----------------------------------------------------------------------------
//  - audio_play_music_id : lance une musique identifiée par MusicID
//  - audio_stop_music    : stoppe la musique en cours
//  - audio_music_is_playing : indique si une musique PMF est active
//
//  Les données PMF sont définies dans assets/music/*.cpp
//  et référencées dans music_map.h
// ============================================================================
void audio_play_music_id(MusicID id);
void audio_stop_music();
bool audio_music_is_playing();


// ============================================================================
//  Effets sonores (SFX)
// ----------------------------------------------------------------------------
//  - audio_play_tone  : génère un son simple (sinus / square selon backend)
//  - audio_play_noise : bruit aléatoire (utile pour collisions, explosions)
// ============================================================================
void audio_play_tone(float freq, float volume, uint16_t duration_ms);
void audio_play_noise(float volume, uint16_t duration_ms);


// ============================================================================
//  Lecture de fichiers WAV (optionnel)
// ----------------------------------------------------------------------------
//  - audio_play_wav : joue un fichier WAV depuis la SD
//  - audio_wav_is_playing : indique si un WAV est en cours
//
//  Le support dépend du backend gb_audio_track_wav
// ============================================================================
void audio_play_wav(const char* path);
bool audio_wav_is_playing();


// ============================================================================
//  SFX de haut niveau (mappés sur les sons du jeu)
// ----------------------------------------------------------------------------
//  - Utilisés par le menu OPTIONS et potentiellement par le gameplay.
// ============================================================================
void audio_play_move();
void audio_play_push();
void audio_play_win();
void audio_play_lose();


// ============================================================================
//  Réglage du volume musique (0–100)
// ----------------------------------------------------------------------------
//  - Met à jour g_audio_settings.music_volume
//  - Applique le volume au backend audio (PMF / mixeur)
// ============================================================================
void audio_set_music_volume(int v);

// Réglage du volume SFX (0–100)
void audio_set_sfx_volume(int v);


// ============================================================================
//  Paramètres audio globaux
// ----------------------------------------------------------------------------
//  - music_volume : volume musique (0–100)
//  - sfx_volume   : volume SFX (0–100)
//
//  Ces valeurs sont utilisées par les menus (options.cpp)
//  et peuvent être sauvegardées via persist.
// ============================================================================
struct AudioSettings {
    int music_volume = 100;
    int sfx_volume   = 100;
};

extern AudioSettings g_audio_settings;

} // namespace baba
