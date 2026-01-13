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
    // Volume max par défaut
    g_player.set_master_volume(255);

    // Enregistrement des pistes
    g_player.add_track(&g_track_music);
    g_player.add_track(&g_track_tone);
    g_player.add_track(&g_track_wav);
}


// ============================================================================
//  Update audio (à appeler dans la boucle principale)
// ============================================================================
void audio_update()
{
    g_player.pool();
}


// ============================================================================
//  Musique PMF
// ============================================================================
void audio_play_music_id(MusicID id)
{
    const uint8_t* data = get_music_data(id);
    if (!data)
        return;

    g_track_music.stop_playing();
    g_track_music.load_pmf(data);
    g_track_music.play_pmf();
}

void audio_stop_music()
{
    g_track_music.stop_playing();
}

bool audio_music_is_playing()
{
    return g_track_music.is_playing();
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
void audio_play_move()
{
    // Petit bip de déplacement
    audio_play_tone(600.0f, 0.5f, 80);
}

void audio_play_push()
{
    // Ton plus grave pour un push
    audio_play_tone(300.0f, 0.6f, 120);
}

void audio_play_win()
{
    // On peut soit jouer un jingle court en PMF, soit un ton plus "positif"
    // Ici : simple exemple en attendant un vrai jingle
    audio_play_tone(800.0f, 0.7f, 200);
}

void audio_play_lose()
{
    // Ton plus grave, un peu plus long
    audio_play_tone(200.0f, 0.7f, 250);
}
	


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

    // Conversion 0–100 → 0–255
    float vol = v / 100.0f;
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
