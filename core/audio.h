#pragma once
#include <stdint.h>
#include "music_map.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace baba {

// ============================================================================
//  Paramètres audio globaux
// ============================================================================
struct AudioSettings {
    int music_volume = 100;   // 0–100
    int sfx_volume   = 100;   // 0–100
};

extern AudioSettings g_audio_settings;


// ============================================================================
//  Variables internes (utilisées par task_audio uniquement)
//  - exposées ici car task_audio en a besoin
//  - NE PAS utiliser ailleurs dans le jeu
// ============================================================================
extern SemaphoreHandle_t g_audio_cmd_mutex;
extern MusicID g_requested_music;
extern MusicID g_current_music;


// ============================================================================
//  Initialisation du système audio
// ============================================================================
void audio_init();

// Mise à jour audio (appelée uniquement par task_audio)
void audio_update();


// ============================================================================
//  API publique : demande de musique (thread-safe)
// ============================================================================
void audio_request_music(MusicID id);


// ============================================================================
//  API interne (réservée à task_audio)
// ============================================================================
void audio_play_music_internal(MusicID id);


// ============================================================================
//  SFX : ton simple / bruit
// ============================================================================
void audio_play_tone(float freq, float volume, uint16_t duration_ms);
void audio_play_noise(float volume, uint16_t duration_ms);


// ============================================================================
//  WAV
// ============================================================================
void audio_play_wav(const char* path);
bool audio_wav_is_playing();


// ============================================================================
//  SFX de haut niveau
// ============================================================================
void audio_play_move();
void audio_play_push();
void audio_play_win();
void audio_play_lose();


// ============================================================================
//  Réglage du volume
// ============================================================================
void audio_set_music_volume(int v);
void audio_set_sfx_volume(int v);

} // namespace baba
