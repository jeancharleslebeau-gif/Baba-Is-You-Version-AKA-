/*
===============================================================================
  audio.cpp — Moteur audio AKA (PMF + SFX + WAV)
-------------------------------------------------------------------------------
  Rôle :
    - Centraliser la gestion audio du moteur BabaIsU
    - Fournir une API thread-safe pour demander un changement de musique
    - Garantir que seul task_audio manipule réellement le player PMF
    - Préserver les réglages utilisateur (volume, etc.)

  Architecture :
    - g_player : moteur audio AKA (PMF + SFX + WAV)
    - g_track_music : piste PMF (musique)
    - g_track_tone  : piste tonale (bips, bruitages)
    - g_track_wav   : piste WAV (samples)
    - g_requested_music : commande asynchrone (écrite par le jeu)
    - g_current_music   : musique réellement jouée (mise à jour par task_audio)
    - g_audio_cmd_mutex : protège les commandes audio (thread-safe)

  Important :
    - audio_play_music_internal() NE DOIT PAS être appelé depuis le jeu.
      Seule task_audio l’utilise, après avoir lu une commande.
===============================================================================
*/

#include "audio.h"

// Lib AKA
#include "gb_audio_player.h"
#include "gb_audio_track_tone.h"
#include "gb_audio_track_fx.h"
#include "gb_audio_track_wav.h"
#include "gb_audio_track_pmf.h"
#include "gb_common.h"
#include "music_map.h"

// Déclarations des musiques PMF (définies dans assets/music/*.h)
extern const uint8_t baba_samba_la_baba_pmf[];
extern const uint8_t baba_music_2_pmf[];
extern const uint8_t baba_cave_newer_short_pmf[];
extern const uint8_t CRYSTAL_pmf[];
extern const uint8_t MISTHART_pmf[];
extern const uint8_t WF_DRAGO_pmf[];
extern const uint8_t WF_MAGES_pmf[];

namespace baba {

// ============================================================================
//  Objets audio AKA (globaux mais encapsulés dans le namespace)
// ============================================================================
AudioSettings g_audio_settings;

static gb_audio_player g_player;

static gb_audio_track_pmf  g_track_music;
static gb_audio_track_tone g_track_tone;
static gb_audio_track_wav  g_track_wav;

// Commande asynchrone (écrite par le jeu, lue par task_audio)
MusicID g_requested_music = MusicID::NONE;

// Musique réellement jouée (mise à jour par task_audio)
MusicID g_current_music   = MusicID::NONE;

// Mutex protégeant les commandes audio
SemaphoreHandle_t g_audio_cmd_mutex = nullptr;


// ============================================================================
//  Sélection musique selon MusicID
// ============================================================================
static const uint8_t* get_music_data(MusicID id)
{
    switch(id)
    {
        case MusicID::BABA_SAMBA:   return baba_samba_la_baba_pmf;
        case MusicID::BABA_MUSIC_2: return baba_music_2_pmf;
        case MusicID::BABA_CAVE:    return baba_cave_newer_short_pmf;
        case MusicID::CRYSTAL:      return CRYSTAL_pmf;
        case MusicID::MISTHART:     return MISTHART_pmf;
        case MusicID::WF_DRAGO:     return WF_DRAGO_pmf;
        case MusicID::WF_MAGES:     return WF_MAGES_pmf;
        default: return nullptr;
    }
}


// ============================================================================
//  Initialisation audio
// ============================================================================
void audio_init()
{
    /*
    ---------------------------------------------------------------------------
      Initialisation du moteur audio AKA
      - Création du mutex de commande (sécurise les changements de musique)
      - Configuration du volume maître selon les préférences utilisateur
      - Enregistrement des pistes (PMF, TONE, WAV) dans le player
    ---------------------------------------------------------------------------
    */

    if (!g_audio_cmd_mutex)
        g_audio_cmd_mutex = xSemaphoreCreateMutex();

    // Volume maître : on respecte le réglage utilisateur existant
    uint8_t vol = static_cast<uint8_t>(g_audio_settings.music_volume * 2.55f);
    g_player.set_master_volume(vol);

    // Enregistrement des pistes dans le player
    g_player.add_track(&g_track_music);
    g_player.add_track(&g_track_tone);
    g_player.add_track(&g_track_wav);
}


// ============================================================================
//  Update audio (appelé uniquement par task_audio)
// ============================================================================
void audio_update()
{
    // Avance la musique PMF, les SFX, etc.
    g_player.pool();
}


// ============================================================================
//  API thread-safe : demande de musique (appelée par le jeu)
// ============================================================================
void audio_request_music(MusicID id)
{
    if (!g_audio_cmd_mutex) return;

    xSemaphoreTake(g_audio_cmd_mutex, portMAX_DELAY);
    g_requested_music = id;
    xSemaphoreGive(g_audio_cmd_mutex);
}


// ============================================================================
//  API interne : changement réel de musique (appelé uniquement par task_audio)
// ============================================================================
void audio_play_music_internal(MusicID id)
{
    const uint8_t* data = get_music_data(id);
    if (!data)
        return;

    g_track_music.stop_playing();
    g_track_music.load_pmf(data);
    g_track_music.play_pmf();
}


// ============================================================================
//  SFX : ton simple
// ============================================================================
void audio_play_tone(float freq, float volume, uint16_t duration_ms)
{
    g_track_tone.play_tone(freq, volume, duration_ms);
}


// ============================================================================
//  SFX : bruit (noise)
// ============================================================================
void audio_play_noise(float volume, uint16_t duration_ms)
{
    g_track_tone.play_tone(
        440, 440, volume, 0.0f, duration_ms,
        gb_audio_track_tone::NOISE
    );
}


// ============================================================================
//  SFX de haut niveau (wrappers)
// ============================================================================
void audio_play_move() { audio_play_tone(600.0f, 0.5f, 80); }
void audio_play_push() { audio_play_tone(300.0f, 0.6f, 120); }
void audio_play_win()  { audio_play_tone(800.0f, 0.7f, 200); }
void audio_play_lose() { audio_play_tone(200.0f, 0.7f, 250); }


// ============================================================================
//  WAV
// ============================================================================
void audio_play_wav(const char* path)
{
    g_track_wav.play_wav(path);
}

bool audio_wav_is_playing()
{
    return g_track_wav.is_playing();
}


// ============================================================================
//  Réglage du volume musique
// ============================================================================
void audio_set_music_volume(int v)
{
    if (v < 0) v = 0;
    if (v > 100) v = 100;

    g_audio_settings.music_volume = v;

    uint8_t vol = static_cast<uint8_t>(v * 2.55f);
    g_player.set_master_volume(vol);
}


// ============================================================================
//  Réglage du volume SFX
// ============================================================================
void audio_set_sfx_volume(int v)
{
    if (v < 0) v = 0;
    if (v > 100) v = 100;

    g_audio_settings.sfx_volume = v;
}

} // namespace baba
