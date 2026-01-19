//============================================================================
// PMF Player
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Profoundic Technologies nor the names of its
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL PROFOUNDIC TECHNOLOGIES BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//============================================================================

#include "pmf_player.h"

//============================================================================
// pmf_header : en-tête binaire du fichier PMF
//============================================================================
struct pmf_header
{
  char     signature[4];          // "pmfx" (0x70 0x6d 0x66 0x78)
  uint16_t version;               // version du format (0x1400 pour v1.4)
  uint16_t flags;                 // e_pmf_flags
  uint32_t file_size;             // taille totale du fichier
  uint32_t sample_meta_offs;      // offset table des métadonnées de samples
  uint32_t instrument_meta_offs;  // offset table des métadonnées d’instruments
  uint32_t pattern_meta_offs;     // offset table des métadonnées de patterns
  uint32_t env_data_offs;         // offset des données d’enveloppes
  uint32_t nmap_data_offs;        // offset des note-maps
  uint32_t track_data_offs;       // offset des données de pistes (patterns)
  uint8_t  initial_speed;         // speed (ticks/row) initial
  uint8_t  initial_tempo;         // tempo initial (BPM-ish)
  uint16_t note_period_min;       // borne basse des periods valides
  uint16_t note_period_max;       // borne haute des periods valides
  uint16_t playlist_length;       // nombre d’entrées dans la playlist
  uint8_t  num_channels;          // nombre de canaux dans les patterns
  uint8_t  num_patterns;          // nombre de patterns
  uint8_t  num_instruments;       // nombre d’instruments
  uint8_t  num_samples;           // nombre de samples
  uint8_t  first_playlist_entry;  // début de la playlist (table d’indices)
};

//============================================================================
// PMF format config : offsets et tailles dans le fichier PMF
//============================================================================

enum { pmf_file_version = 0x1400 }; // v1.4

// Offsets dans pmf_header
enum { pmfcfg_offset_signature        = PFC_OFFSETOF(pmf_header, signature) };
enum { pmfcfg_offset_version          = PFC_OFFSETOF(pmf_header, version) };
enum { pmfcfg_offset_flags            = PFC_OFFSETOF(pmf_header, flags) };
enum { pmfcfg_offset_file_size        = PFC_OFFSETOF(pmf_header, file_size) };
enum { pmfcfg_offset_smp_meta_offs    = PFC_OFFSETOF(pmf_header, sample_meta_offs) };
enum { pmfcfg_offset_inst_meta_offs   = PFC_OFFSETOF(pmf_header, instrument_meta_offs) };
enum { pmfcfg_offset_pat_meta_offs    = PFC_OFFSETOF(pmf_header, pattern_meta_offs) };
enum { pmfcfg_offset_env_data_offs    = PFC_OFFSETOF(pmf_header, env_data_offs) };
enum { pmfcfg_offset_nmap_data_offs   = PFC_OFFSETOF(pmf_header, nmap_data_offs) };
enum { pmfcfg_offset_track_data_offs  = PFC_OFFSETOF(pmf_header, track_data_offs) };
enum { pmfcfg_offset_init_speed       = PFC_OFFSETOF(pmf_header, initial_speed) };
enum { pmfcfg_offset_init_tempo       = PFC_OFFSETOF(pmf_header, initial_tempo) };
enum { pmfcfg_offset_note_period_min  = PFC_OFFSETOF(pmf_header, note_period_min) };
enum { pmfcfg_offset_note_period_max  = PFC_OFFSETOF(pmf_header, note_period_max) };
enum { pmfcfg_offset_playlist_length  = PFC_OFFSETOF(pmf_header, playlist_length) };
enum { pmfcfg_offset_num_channels     = PFC_OFFSETOF(pmf_header, num_channels) };
enum { pmfcfg_offset_num_patterns     = PFC_OFFSETOF(pmf_header, num_patterns) };
enum { pmfcfg_offset_num_instruments  = PFC_OFFSETOF(pmf_header, num_instruments) };
enum { pmfcfg_offset_num_samples      = PFC_OFFSETOF(pmf_header, num_samples) };
enum { pmfcfg_offset_playlist         = PFC_OFFSETOF(pmf_header, first_playlist_entry) };

// Métadonnées de patterns
enum { pmfcfg_pattern_metadata_header_size      = 2 };
enum { pmfcfg_pattern_metadata_track_offset_size = 2 };
enum { pmfcfg_offset_pattern_metadata_last_row  = 0 };
enum { pmfcfg_offset_pattern_metadata_track_offsets = 2 };

// Enveloppes
enum { pmfcfg_offset_env_num_points        = 0 };
enum { pmfcfg_offset_env_loop_start        = 1 };
enum { pmfcfg_offset_env_loop_end          = 2 };
enum { pmfcfg_offset_env_sustain_loop_start = 3 };
enum { pmfcfg_offset_env_sustain_loop_end  = 4 };
enum { pmfcfg_offset_env_points            = 6 };
enum { pmfcfg_envelope_point_size          = 4 };
enum { pmfcfg_offset_env_point_tick        = 0 };
enum { pmfcfg_offset_env_point_val         = 2 };

// Note map
enum { pmfcfg_max_note_map_regions         = 8 };
enum { pmfcfg_offset_nmap_num_entries      = 0 };
enum { pmfcfg_offset_nmap_entries          = 1 };
enum { pmfcfg_nmap_entry_size_direct       = 2 };
enum { pmfcfg_nmap_entry_size_range        = 3 };
enum { pmgcfg_offset_nmap_entry_note_idx_offs = 0 };
enum { pmgcfg_offset_nmap_entry_sample_idx = 1 };

// Bit-compression
enum { pmfcfg_num_data_mask_bits   = 4 };
enum { pmfcfg_num_note_bits        = 7 };  // max 10 octaves (0-9) (12*10=120)
enum { pmfcfg_num_instrument_bits  = 6 };  // max 64 instruments
enum { pmfcfg_num_volume_bits      = 6 };  // volume [0,63]
enum { pmfcfg_num_effect_bits      = 4 };  // effets 0-15
enum { pmfcfg_num_effect_data_bits = 8 };

// Flags PMF
enum e_pmf_flags
{
  pmfflag_linear_freq_table = 0x01, // 0 = Amiga, 1 = table linéaire
};

// Notes spéciales
enum { pmfcfg_note_cut = 120 };
enum { pmfcfg_note_off = 121 };

// Effets volume/pan
enum { num_subfx_value_bits = 4 };
enum { subfx_value_mask     = ~(unsigned(-1) << num_subfx_value_bits) };

enum e_pmfx_volslide_type
{
  pmffx_volsldtype_down      = 0x00,
  pmffx_volsldtype_up        = 0x10,
  pmffx_volsldtype_fine_down = 0x20,
  pmffx_volsldtype_fine_up   = 0x30,
  pmffx_volsldtype_mask      = 0x30,
  pmffx_volsldtype_fine_mask = 0x20
};

enum e_pmfx_panslide_type
{
  pmffx_pansldtype_left        = 0x80,
  pmffx_pansldtype_right       = 0xa0,
  pmffx_pansldtype_fine_left   = 0xc0,
  pmffx_pansldtype_fine_right  = 0xe0,
  pmffx_pansldtype_val_mask    = 0x0f,
  pmffx_pansldtype_dir_mask    = 0x20,
  pmffx_pansldtype_fine_mask   = 0x40,
  pmffx_pansldtype_enable_mask = 0x80
};

enum e_pmf_voleffect
{
  pmfvolfx_vol_slide            = 0x40,
  pmfvolfx_vol_slide_down       = 0x40,
  pmfvolfx_vol_slide_up         = 0x50,
  pmfvolfx_vol_slide_fine_down  = 0x60,
  pmfvolfx_vol_slide_fine_up    = 0x70,
  pmfvolfx_note_slide_down      = 0x80,
  pmfvolfx_note_slide_up        = 0x90,
  pmfvolfx_note_slide           = 0xa0,
  pmfvolfx_set_vibrato_speed    = 0xb0,
  pmfvolfx_vibrato              = 0xc0,
  pmfvolfx_set_panning          = 0xd0,
  pmfvolfx_pan_slide_fine_left  = 0xe0,
  pmfvolfx_pan_slide_fine_right = 0xf0,
};

// Tables de formes d’onde pour vibrato/tremolo
static const int8_t PROGMEM s_waveforms[3][32] =
{
  // sinusoïde
  { 6, 19, 31, 43, 54, 65, 76, 85, 94, 102, 109, 115, 120, 123, 126, 127,
    127, 126, 123, 120, 115, 109, 102, 94, 85, 76, 65, 54, 43, 31, 19, 6 },
  // rampe descendante
  { -2, -6, -10, -14, -18, -22, -26, -30, -34, -38, -42, -46, -50, -54, -58, -62,
    -66, -70, -74, -78, -82, -86, -90, -94, -98, -102, -106, -110, -114, -118, -122, -126 },
  // carré
  { 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 },
};

//============================================================================
// Note periods
//============================================================================
enum { note_slide_down_target_period = 32767 };
enum { note_slide_up_target_period   = 1 };

//============================================================================
// Helpers locaux
//============================================================================
namespace
{
  // Lecture de bits dans un flux compressé
  uint8_t read_bits(const uint8_t *&ptr, uint8_t &bit_pos, uint8_t num_bits)
  {
    uint8_t v = pgm_read_byte(ptr) >> bit_pos;
    bit_pos += num_bits;
    if (bit_pos > 7)
    {
      ++ptr;
      if (bit_pos -= 8)
        v |= pgm_read_byte(ptr) << (num_bits - bit_pos);
    }
    return v;
  }

  // Approximation rapide de 2^x (utilisée pour les périodes non linéaires)
  float fast_exp2(float x)
  {
    int adjustment = 0;
    uint8_t int_arg = uint8_t(x);
    x -= int_arg;
    if (x > 0.5f)
    {
      adjustment = 1;
      x -= 0.5f;
    }

    float x2 = x * x;
    float q  = 20.8189237930062f + x2;
    float x_p = x * (7.2152891521493f + 0.0576900723731f * x2);
    float res = (1 << int_arg) * (q + x_p) / (q - x_p);
    if (adjustment)
      res *= 1.4142135623730950488f; // sqrt(2)
    return res;
  }
} // namespace

//============================================================================
// pmf_player : construction / destruction
//============================================================================

pmf_player::pmf_player()
  : m_pmf_file(nullptr)
  , m_sampling_freq(0)
  , m_row_callback(nullptr)
  , m_tick_callback(nullptr)
  , m_speed(0)
{
}

// Arrêt propre à la destruction
pmf_player::~pmf_player()
{
  stop();
}

//============================================================================
// Chargement du fichier PMF
//============================================================================

void pmf_player::load(const void *pmem_pmf_file)
{
  if (!pmem_pmf_file)
  {
    PMF_SERIAL_LOG("Error: Null PMF pointer.\r\n");
    return;
  }

  const uint8_t *pmf_file = static_cast<const uint8_t*>(pmem_pmf_file);

  // Vérifie la signature "pmfx" (0x70 0x6d 0x66 0x78)
  if (pgm_read_dword(pmf_file + pmfcfg_offset_signature) != 0x78666d70)
  {
    PMF_SERIAL_LOG("Error: Invalid PMF file. Please use pmf_converter to generate the file.\r\n");
    return;
  }

  // Vérifie la version (on masque les 4 bits de révision)
  const uint16_t file_ver = pgm_read_word(pmf_file + pmfcfg_offset_version);
  if ((file_ver & 0xfff0) != pmf_file_version)
  {
    PMF_SERIAL_LOG("Error: PMF file version mismatch. Please use matching pmf_converter to generate the file.\r\n");
    return;
  }

  // Lecture des propriétés de base
  m_pmf_file            = pmf_file;
  m_num_pattern_channels = pgm_read_byte(m_pmf_file + pmfcfg_offset_num_channels);
  m_num_instruments      = pgm_read_byte(m_pmf_file + pmfcfg_offset_num_instruments);
  m_num_samples          = pgm_read_byte(m_pmf_file + pmfcfg_offset_num_samples);

  // Limite le nombre de canaux de lecture à pmfplayer_max_channels
  enable_playback_channels(m_num_pattern_channels);

  m_pmf_flags       = pgm_read_word(m_pmf_file + pmfcfg_offset_flags);
  m_note_slide_speed = (m_pmf_flags & pmfflag_linear_freq_table) ? 4 : 2;

  PMF_SERIAL_LOG("PMF file loaded (%i channels)\r\n", m_num_pattern_channels);
}

//============================================================================
// Configuration des callbacks et canaux
//============================================================================

void pmf_player::enable_playback_channels(uint8_t num_channels)
{
  if (!m_pmf_file)
    return;

  // Clamp sur pmfplayer_max_channels
  m_num_playback_channels =
    (num_channels < pmfplayer_max_channels) ? num_channels : pmfplayer_max_channels;
}

void pmf_player::set_row_callback(pmf_row_callback_t callback, void *custom_data)
{
  m_row_callback = callback;
  m_row_callback_custom_data = custom_data;
}

void pmf_player::set_tick_callback(pmf_tick_callback_t callback, void *custom_data)
{
  m_tick_callback = callback;
  m_tick_callback_custom_data = custom_data;
}

//============================================================================
// Accesseurs simples
//============================================================================

uint8_t pmf_player::num_pattern_channels() const
{
  return m_pmf_file ? m_num_pattern_channels : 0;
}

uint8_t pmf_player::num_playback_channels() const
{
  return m_pmf_file ? m_num_playback_channels : 0;
}

uint16_t pmf_player::playlist_length() const
{
  return m_pmf_file ? pgm_read_word(m_pmf_file + pmfcfg_offset_playlist_length) : 0;
}

//============================================================================
// Démarrage / arrêt de la lecture
//============================================================================

void pmf_player::start(uint32_t sampling_freq, uint16_t playlist_pos)
{
  if (!m_pmf_file)
    return;

  // Reset complet des canaux
  memset(m_channels, 0, sizeof(m_channels));

  const uint16_t playlist_len =
    pgm_read_word(m_pmf_file + pmfcfg_offset_playlist_length);

  // Initialisation des canaux audio
  for (unsigned ci = 0; ci < m_num_playback_channels; ++ci)
  {
    audio_channel &chl = m_channels[ci];
    chl.sample_panning =
      pgm_read_byte(m_pmf_file + pmfcfg_offset_playlist + playlist_len + ci);
    chl.fxmem_vol_slide_spd = pmfvolfx_vol_slide_down | 0x01;
    chl.vol_env.value   = 0xffff;
    chl.pitch_env.value = 0x8000;
  }

  // État de lecture
  m_sampling_freq = get_sampling_freq(sampling_freq);
  m_num_processed_pattern_channels =
    min(m_num_pattern_channels, m_num_playback_channels);

  const uint16_t clamped_pos =
    (playlist_pos < playlist_len) ? playlist_pos : 0;
  init_pattern(clamped_pos);

  m_speed           = pgm_read_byte(m_pmf_file + pmfcfg_offset_init_speed);
  m_note_period_min = pgm_read_word(m_pmf_file + pmfcfg_offset_note_period_min);
  m_note_period_max = pgm_read_word(m_pmf_file + pmfcfg_offset_note_period_max);

  const uint8_t init_tempo =
    pgm_read_byte(m_pmf_file + pmfcfg_offset_init_tempo);
  m_num_batch_samples =
    (m_sampling_freq * 125) / long(init_tempo * 50);
  m_num_batch_samples /= 2;
  m_current_row_tick = m_speed ? (m_speed - 1) : 0;
  m_arpeggio_counter = 0;
  m_pattern_delay    = 1;

  // Démarre le backend audio
  m_batch_pos = 0;
  start_playback(sampling_freq);

  PMF_SERIAL_LOG("PMF playback started (%i channels)\r\n", m_num_playback_channels);
}

void pmf_player::stop()
{
  if (m_speed)
    stop_playback();
  m_speed = 0;
}

//============================================================================
// Boucle de mise à jour (backend "pull" classique)
//============================================================================

void pmf_player::update()
{
  // Si le player n’est pas initialisé correctement, on sort
  if (!m_note_slide_speed || !m_pmf_file || !m_speed)
    return;

  pmf_mixer_buffer subbuffer = get_mixer_buffer();
  if (!subbuffer.num_samples)
    return;

  do
  {
    const uint16_t batch_left = m_num_batch_samples - m_batch_pos;
    const unsigned num_samples =
      min(subbuffer.num_samples, batch_left);

    mix_buffer(subbuffer, num_samples);
    m_batch_pos += num_samples;

    if (m_batch_pos == m_num_batch_samples)
    {
      // Tick suivant
      if (++m_current_row_tick == m_speed)
      {
        if (!--m_pattern_delay)
        {
          m_pattern_delay = 1;
          process_pattern_row();
        }
        m_current_row_tick = 0;
      }
      else
      {
        apply_channel_effects();
      }

      if (m_num_instruments)
        evaluate_envelopes();

      if (m_tick_callback)
        (*m_tick_callback)(m_tick_callback_custom_data);

      m_batch_pos = 0;
    }
  } while (subbuffer.num_samples);
}

//============================================================================
// mix() : version Gamebuino-AKA, appelée par le callback audio
//============================================================================

void pmf_player::mix(int16_t *out_buffer, unsigned num_samples)
{
  // Sécurité : si le player n’est pas en état de lecture, on ne touche pas au buffer
  if (!m_note_slide_speed || !m_pmf_file || !m_speed || !out_buffer || !num_samples)
    return;

  pmf_mixer_buffer subbuffer;
  subbuffer.begin       = out_buffer;
  subbuffer.num_samples = num_samples;

  do
  {
    const uint16_t batch_left = m_num_batch_samples - m_batch_pos;
    const unsigned chunk =
      (subbuffer.num_samples < batch_left) ? subbuffer.num_samples : batch_left;

    if (chunk == 0)
      break;

    // Mix interne (implémenté via mix_buffer_impl)
    mix_buffer(subbuffer, chunk);
    m_batch_pos += chunk;

    // Fin de batch → avance la musique (ticks, rows, effets…)
    if (m_batch_pos == m_num_batch_samples)
    {
      if (++m_current_row_tick == m_speed)
      {
        if (!--m_pattern_delay)
        {
          m_pattern_delay = 1;
          process_pattern_row();
        }
        m_current_row_tick = 0;
      }
      else
      {
        apply_channel_effects();
      }

      if (m_num_instruments)
        evaluate_envelopes();

      if (m_tick_callback)
        (*m_tick_callback)(m_tick_callback_custom_data);

      m_batch_pos = 0;
    }
  } while (subbuffer.num_samples);
}

//============================================================================
// État de lecture / infos de canaux
//============================================================================

bool pmf_player::is_playing() const
{
  return m_speed != 0;
}

uint8_t pmf_player::playlist_pos() const
{
  return m_speed ? m_current_pattern_playlist_pos : 0;
}

uint8_t pmf_player::pattern_row() const
{
  return m_speed ? m_current_pattern_row_idx : 0;
}

uint8_t pmf_player::pattern_speed() const
{
  return m_speed ? m_speed : 0;
}

pmf_channel_info pmf_player::channel_info(uint8_t channel_idx) const
{
  pmf_channel_info info;

  if (channel_idx < m_num_playback_channels)
  {
    const audio_channel &chl = m_channels[channel_idx];
    info.base_note   = chl.base_note_idx;
    info.volume      = chl.sample_volume;
    info.effect      = chl.effect;
    info.effect_data = chl.effect_data;
    info.note_hit    = chl.note_hit;
  }
  else
  {
    // Canal invalide → info neutre
    info.base_note   = 0xff;
    info.volume      = 0;
    info.effect      = 0xff;
    info.effect_data = 0;
    info.note_hit    = 0;
  }
  return info;
}

//============================================================================
// Effets de canal (volume / pitch / vibrato / etc.)
//============================================================================

void pmf_player::apply_channel_effect_volume_slide(audio_channel &chl)
{
  // Slide de volume vers le haut ou le bas selon fxmem_vol_slide_spd
  const int8_t vdelta = (chl.fxmem_vol_slide_spd & 0x0f) << 2;
  int16_t v = int16_t(chl.sample_volume) +
              (((chl.fxmem_vol_slide_spd & pmffx_volsldtype_mask) == pmffx_volsldtype_down)
                 ? -vdelta
                 :  vdelta);
  if (v < 0)   v = 0;
  if (v > 255) v = 255;
  chl.sample_volume = uint8_t(v);
}

void pmf_player::apply_channel_effect_note_slide(audio_channel &chl)
{
  // Slide de la période vers la cible, clampée
  if (!chl.sample_speed)
    return;

  int16_t note_period     = chl.note_period;
  const int16_t target_prd = chl.fxmem_note_slide_prd;
  const int16_t slide_spd  =
    (note_period < target_prd ? chl.fxmem_note_slide_spd : -chl.fxmem_note_slide_spd)
    * m_note_slide_speed;

  note_period += slide_spd;
  // Si on dépasse la cible, on clamp
  if ( (slide_spd > 0) ^ (note_period < target_prd) )
    note_period = target_prd;

  chl.note_period = note_period;

  if (note_period < m_note_period_min || note_period > m_note_period_max)
  {
    chl.sample_speed = 0;
  }
  else
  {
    chl.sample_speed = get_sample_speed(chl.note_period, (chl.sample_speed >= 0));
  }
}

void pmf_player::apply_channel_effect_vibrato(audio_channel &chl)
{
  if (!chl.sample_speed)
    return;

  const uint8_t wave_idx = chl.fxmem_vibrato_wave & 3;
  int8_t vibrato_pos     = chl.fxmem_vibrato_pos;

  int8_t wave_sample =
    (vibrato_pos < 0)
      ? -int8_t(pgm_read_byte(&s_waveforms[wave_idx][~vibrato_pos]))
      :  int8_t(pgm_read_byte(&s_waveforms[wave_idx][ vibrato_pos ]));

  const int16_t offset =
    int16_t(wave_sample * chl.fxmem_vibrato_depth) >> 8;

  chl.sample_speed =
    get_sample_speed(chl.note_period + offset, (chl.sample_speed >= 0));

  if ( (chl.fxmem_vibrato_pos += chl.fxmem_vibrato_spd) > 31 )
    chl.fxmem_vibrato_pos -= 64;
}

void pmf_player::apply_channel_effects()
{
  if (++m_arpeggio_counter == 3)
    m_arpeggio_counter = 0;

  for (unsigned ci = 0; ci < m_num_playback_channels; ++ci)
  {
    audio_channel &chl = m_channels[ci];
    chl.note_hit = 0;

    // Effet de volume (colonne volume)
    switch (chl.vol_effect)
    {
      case pmfvolfx_vol_slide: apply_channel_effect_volume_slide(chl); break;
      case pmfvolfx_note_slide: apply_channel_effect_note_slide(chl);  break;
      case pmfvolfx_vibrato:    apply_channel_effect_vibrato(chl);     break;
      default: break;
    }

    // Effet principal (colonne effet)
    switch (chl.effect)
    {
      case pmffx_arpeggio:
      {
        if (!chl.sample_speed)
          break;
        const uint8_t base_note_idx = chl.base_note_idx & 127;
        const uint16_t note_period =
          get_note_period(base_note_idx +
                          ((chl.fxmem_arpeggio >> (4 * m_arpeggio_counter)) & 0x0f),
                          chl.sample_finetune);
        chl.sample_speed = get_sample_speed(note_period, (chl.sample_speed >= 0));
      } break;

      case pmffx_note_slide:
        apply_channel_effect_note_slide(chl);
        break;

      case pmffx_note_vol_slide:
      {
        if (chl.fxmem_note_slide_spd < 0xe0)
          apply_channel_effect_note_slide(chl);
        if (!(chl.fxmem_vol_slide_spd & pmffx_volsldtype_fine_mask))
          apply_channel_effect_volume_slide(chl);
      } break;

      case pmffx_volume_slide:
        apply_channel_effect_volume_slide(chl);
        break;

      case pmffx_vibrato:
        apply_channel_effect_vibrato(chl);
        break;

      case pmffx_vibrato_vol_slide:
      {
        apply_channel_effect_vibrato(chl);
        if (!(chl.fxmem_vol_slide_spd & pmffx_volsldtype_fine_mask))
          apply_channel_effect_volume_slide(chl);
      } break;

      case pmffx_retrig_vol_slide:
      {
        if (!--chl.fxmem_retrig_count)
        {
          uint8_t effect_data = chl.effect_data;
          chl.fxmem_retrig_count = effect_data & 0x0f;
          int vol = chl.sample_volume;
          effect_data >>= 4;
          switch (effect_data)
          {
            case  6: vol = (vol + vol) / 3; break;
            case  7: vol >>= 1;             break;
            case 14: vol = (vol * 3) / 2;   break;
            case 15: vol += vol;            break;
            default:
            {
              const uint8_t delta = 2 << (effect_data & 7);
              vol += (effect_data & 8) ? +delta : -delta;
            } break;
          }
          if (vol < 0)   vol = 0;
          if (vol > 255) vol = 255;
          chl.sample_volume = uint8_t(vol);
          chl.sample_pos    = 0;
          chl.note_hit      = 1;
        }
      } break;

      case pmffx_subfx | (pmfsubfx_note_cut << pmfcfg_num_effect_bits):
      {
        // Coupe la note après N ticks
        if (!--chl.effect_data)
        {
          chl.sample_speed = 0;
          chl.effect       = 0xff;
        }
      } break;

      case pmffx_subfx | (pmfsubfx_note_delay << pmfcfg_num_effect_bits):
      {
        // Déclenche la note après N ticks
        if (!--chl.effect_data)
        {
          hit_note(chl, chl.fxmem_note_delay_idx, 0, true);
          chl.effect = 0xff;
        }
      } break;

      case pmffx_panning:
      {
        const uint8_t pan_spd =
          (chl.fxmem_panning_spd & pmffx_pansldtype_val_mask) * 4;
        const bool dir_right =
          (chl.fxmem_panning_spd & pmffx_pansldtype_dir_mask) != 0;

        int pan = chl.sample_panning;
        pan = dir_right ? min(127, pan + int(pan_spd))
                        : max(-127, pan - int(pan_spd));
        chl.sample_panning = int8_t(pan);
      } break;

      default:
        break;
    }
  }
}

//============================================================================
// Initialisation des effets (vol slide / note slide / vibrato)
//============================================================================

bool pmf_player::init_effect_volume_slide(audio_channel &chl, uint8_t effect_data)
{
  // Si des données sont présentes, on met à jour la mémoire d’effet
  if (effect_data & 0x0f)
    chl.fxmem_vol_slide_spd = effect_data;
  effect_data = chl.fxmem_vol_slide_spd;

  if (effect_data & pmffx_volsldtype_fine_mask)
  {
    // Fine slide appliqué immédiatement
    const uint8_t fx_type = effect_data & pmffx_volsldtype_mask;
    const int8_t vdelta   = (effect_data & 0x0f) << 2;
    int16_t v = int16_t(chl.sample_volume) +
                (fx_type == pmffx_volsldtype_fine_down ? -vdelta : vdelta);
    if (v < 0)   v = 0;
    if (v > 255) v = 255;
    chl.sample_volume = uint8_t(v);
    return false; // rien à faire aux ticks suivants
  }
  return true; // slide "normal" à appliquer sur les ticks
}

bool pmf_player::init_effect_note_slide(audio_channel &chl,
                                        uint8_t slide_speed,
                                        uint16_t target_note_period)
{
  // Mémorise la vitesse de slide si fournie
  if (slide_speed)
    chl.fxmem_note_slide_spd = slide_speed;
  else
    slide_speed = chl.fxmem_note_slide_spd;

  // Mémorise la cible si fournie
  if (target_note_period)
    chl.fxmem_note_slide_prd = target_note_period;

  // Slide "normal" (non fine)
  if (slide_speed < 0xe0)
    return true;

  // Fine / extra-fine slide appliqué immédiatement
  if (!chl.sample_speed)
    return false;

  int16_t note_period = chl.note_period;
  int16_t slide_spd   =
    (slide_speed >= 0xf0) ? (slide_speed - 0xf0) * 4
                          : (slide_speed - 0xe0);

  if (note_period > target_note_period)
    slide_spd = -slide_spd;

  note_period += slide_spd;
  if ( (slide_spd > 0) ^ (note_period < target_note_period) )
    note_period = target_note_period;

  chl.note_period = note_period;
  if (note_period < m_note_period_min || note_period > m_note_period_max)
    chl.sample_speed = 0;

  return false;
}

void pmf_player::init_effect_vibrato(audio_channel &chl,
                                     uint8_t vibrato_depth,
                                     uint8_t vibrato_speed)
{
  if (vibrato_depth)
    chl.fxmem_vibrato_depth = vibrato_depth << 3;
  if (vibrato_speed)
    chl.fxmem_vibrato_spd = vibrato_speed;
}

//============================================================================
// Enveloppes
//============================================================================

void pmf_player::evaluate_envelope(envelope_state &env,
                                   uint16_t env_data_offs,
                                   bool is_note_off)
{
  const uint8_t *envelope =
    m_pmf_file + pgm_read_dword(m_pmf_file + pmfcfg_offset_env_data_offs)
    + env_data_offs;

  const uint8_t *env_span_data =
    envelope + pmfcfg_offset_env_points + env.pos * pmfcfg_envelope_point_size;

  uint16_t env_span_tick_end =
    pgm_read_word(env_span_data + pmfcfg_envelope_point_size +
                  pmfcfg_offset_env_point_tick);

  // Avance le temps dans l’enveloppe
  if (++env.tick >= env_span_tick_end)
  {
    uint8_t env_pnt_start_idx;
    uint8_t env_pnt_end_idx;

    if (is_note_off)
    {
      env_pnt_start_idx = pgm_read_byte(envelope + pmfcfg_offset_env_loop_start);
      env_pnt_end_idx   = pgm_read_byte(envelope + pmfcfg_offset_env_loop_end);
    }
    else
    {
      env_pnt_start_idx =
        pgm_read_byte(envelope + pmfcfg_offset_env_sustain_loop_start);
      env_pnt_end_idx   =
        pgm_read_byte(envelope + pmfcfg_offset_env_sustain_loop_end);
    }

    const uint8_t env_last_pnt_idx =
      pgm_read_byte(envelope + pmfcfg_offset_env_num_points) - 1;

    env_pnt_start_idx = min(env_pnt_start_idx, env_last_pnt_idx);
    env_pnt_end_idx   = min(env_pnt_end_idx,   env_last_pnt_idx);

    // Fin de segment / boucle
    if (++env.pos == env_pnt_end_idx)
    {
      if (env_pnt_start_idx < env_pnt_end_idx)
        env.pos = env_pnt_start_idx;
      else
        env.pos = env_pnt_start_idx - 1;

      env.tick =
        pgm_read_word(envelope + pmfcfg_offset_env_points +
                      env_pnt_start_idx * pmfcfg_envelope_point_size +
                      pmfcfg_offset_env_point_tick);
    }

    env_span_data =
      envelope + pmfcfg_offset_env_points + env.pos * pmfcfg_envelope_point_size;
    env_span_tick_end =
      pgm_read_word(env_span_data + pmfcfg_envelope_point_size +
                    pmfcfg_offset_env_point_tick);
  }

  // Interpolation linéaire entre deux points de l’enveloppe
  const uint16_t env_span_tick_start =
    pgm_read_word(env_span_data + pmfcfg_offset_env_point_tick);
  const uint16_t env_span_val_start =
    pgm_read_word(env_span_data + pmfcfg_offset_env_point_val);
  const uint16_t env_span_val_end =
    pgm_read_word(env_span_data + pmfcfg_envelope_point_size +
                  pmfcfg_offset_env_point_val);

  const float span_pos =
    float(env.tick - env_span_tick_start) /
    float(env_span_tick_end - env_span_tick_start);

  env.value = env_span_val_start +
              int32_t(span_pos *
                      (int32_t(env_span_val_end) - int32_t(env_span_val_start)));
}

//------------------------------------------------

void pmf_player::evaluate_envelopes()
{
  // evaluate channel envelopes
  for(uint8_t ci=0; ci<m_num_playback_channels; ++ci)
  {
    audio_channel &chl=m_channels[ci];

    // 🔒 Si aucun instrument n’a encore été assigné, on saute ce canal
    if (!chl.inst_metadata)
      continue;

    bool is_note_off=(chl.base_note_idx&0x80)!=0;
	//==============================================================
	// FIX: inst_metadata null-protection
	//==============================================================
	uint16_t vol_env_offset = 0xffff;   // sentinel = no envelope
	if (chl.inst_metadata)              // avoid null pointer deref
	{
		vol_env_offset = pgm_read_word(
			chl.inst_metadata + pmfcfg_offset_inst_vol_env
		);
	}

    if(vol_env_offset!=0xffff)
      evaluate_envelope(chl.vol_env, vol_env_offset, is_note_off);

    // pitch env commenté chez toi, donc rien à changer ici

    if(is_note_off)
    {
      chl.vol_env.value=(chl.vol_env.value>>8)*(chl.vol_fadeout>>8);
      uint16_t fadeout_speed=pgm_read_word(chl.inst_metadata+pmfcfg_offset_inst_fadeout_speed);
      chl.vol_fadeout=chl.vol_fadeout>fadeout_speed?chl.vol_fadeout-fadeout_speed:0;
    }
  }
}

//----------------------------------------------------------------------------

uint16_t pmf_player::get_note_period(uint8_t note_idx_, int16_t finetune_)
{
  if(m_pmf_flags&pmfflag_linear_freq_table)
    return 7680-note_idx_*64-finetune_/2;
  return uint16_t(27392.0f/fast_exp2((note_idx_*128+finetune_)/(12.0f*128.0f))+0.5f);
}
//----

int16_t pmf_player::get_sample_speed(uint16_t note_period_, bool forward_)
{
  int16_t speed;
  if(m_pmf_flags&pmfflag_linear_freq_table)
    speed=int16_t((8363.0f*8.0f/m_sampling_freq)*fast_exp2(float(7680-note_period_)/768.0f)+0.5f);
  else
    speed=int16_t((7093789.2f*256.0f/m_sampling_freq)/note_period_+0.5f);
  return forward_?speed:-speed;
}
//----

void pmf_player::set_instrument(audio_channel &chl_, uint8_t inst_idx_, uint8_t note_idx_)
{
  uint8_t inst_vol=0xff;
  int8_t panning=-128;
  if(m_num_instruments)
  {
    // setup instrument metadata and check for note mapping
    const uint8_t *inst_metadata=m_pmf_file+pgm_read_dword(m_pmf_file+pmfcfg_offset_inst_meta_offs)+inst_idx_*pmfcfg_instrument_metadata_size;
    chl_.inst_metadata=inst_metadata;
    inst_vol=pgm_read_byte(inst_metadata+pmfcfg_offset_inst_volume);
    panning=pgm_read_byte(inst_metadata+pmfcfg_offset_inst_panning);
    uint16_t sample_idx=pgm_read_word(inst_metadata+pmfcfg_offset_inst_sample_idx);
    uint8_t note_idx_offs=0;
    if(sample_idx>=m_num_samples)
    {
      // access note map and check for range vs direct map
      uint8_t nidx=note_idx_!=0xff?note_idx_:chl_.base_note_idx&127;
      const uint8_t *nmap=m_pmf_file+pgm_read_dword(m_pmf_file+pmfcfg_offset_nmap_data_offs)+sample_idx-m_num_samples;
      uint8_t num_entries=pgm_read_byte(nmap+pmfcfg_offset_nmap_num_entries);
      if(num_entries<120)
      {
        // find note map range for given note index
        nmap+=pmfcfg_offset_nmap_entries;
        while(true)
        {
          uint8_t range_max=pgm_read_byte(nmap);
          if(nidx<=range_max)
          {
            ++nmap;
            break;
          }
          nmap+=pmfcfg_nmap_entry_size_range;
        }
      }
      else
        nmap+=pmfcfg_offset_nmap_entries+nidx*pmfcfg_nmap_entry_size_direct;

      // get note offset and sample index
      note_idx_offs=pgm_read_byte(nmap+pmgcfg_offset_nmap_entry_note_idx_offs);
      sample_idx=pgm_read_byte(nmap+pmgcfg_offset_nmap_entry_sample_idx);
    }
    chl_.inst_note_idx_offs=note_idx_offs;
    inst_idx_=uint8_t(sample_idx);
  }

  // update sample for the channel
  const uint8_t *smp_metadata=m_pmf_file+pgm_read_dword(m_pmf_file+pmfcfg_offset_smp_meta_offs)+inst_idx_*pmfcfg_sample_metadata_size;
  if(chl_.smp_metadata!=smp_metadata)
  {
    chl_.sample_pos=0;
    if(chl_.sample_speed)
      chl_.sample_speed=get_sample_speed(chl_.note_period, true);
  }
  chl_.smp_metadata=smp_metadata;
  chl_.sample_volume=(uint16_t(inst_vol)*uint16_t(pgm_read_byte(chl_.smp_metadata+pmfcfg_offset_smp_volume)))>>8;
  chl_.sample_finetune=pgm_read_word(chl_.smp_metadata+pmfcfg_offset_smp_finetune);

  // setup panning
  if(panning==-128)
    panning=pgm_read_byte(smp_metadata+pmfcfg_offset_smp_loop_length_and_panning+3);
  if(panning!=-128)
    chl_.sample_panning=panning;
}
//----
 
void pmf_player::hit_note(audio_channel &chl_, uint8_t note_idx_, uint8_t sample_start_pos_, bool reset_sample_pos_)
{
  if(!chl_.smp_metadata)
    return;
  chl_.note_period=get_note_period(note_idx_, chl_.sample_finetune);
  chl_.base_note_idx=note_idx_;
  if(reset_sample_pos_)
    chl_.sample_pos=sample_start_pos_*65536;
  chl_.sample_speed=get_sample_speed(chl_.note_period, true);
  chl_.note_hit=reset_sample_pos_;
  if(!(chl_.fxmem_vibrato_wave&0x4))
    chl_.fxmem_vibrato_pos=0;
}
//----

void pmf_player::process_pattern_row()
{
  // store current track positions
  const uint8_t *current_track_poss[pmfplayer_max_channels];
  uint8_t current_track_bit_poss[pmfplayer_max_channels];
  for(uint8_t ci=0; ci<m_num_processed_pattern_channels; ++ci)
  {
    audio_channel &chl=m_channels[ci];
    current_track_poss[ci]=chl.track_pos;
    current_track_bit_poss[ci]=chl.track_bit_pos;
    chl.note_hit=0;
  }

  // parse row in the music pattern
  bool loop_pattern=false;
  uint8_t num_skip_rows=0;
  for(uint8_t ci=0; ci<m_num_playback_channels; ++ci)
  {
    // get note, instrument, volume and effect for the channel
    audio_channel &chl=m_channels[ci];
    uint8_t note_idx=0xff, inst_idx=0xff, volume=0xff, effect=0xff, effect_data, sample_start_pos=0;
    bool reset_sample_pos=true;
    if(ci<m_num_processed_pattern_channels)
      process_track_row(chl, note_idx, inst_idx, volume, effect, effect_data);
    if(m_row_callback)
    {
      // apply custom track data
      uint8_t custom_note_idx=0xff, custom_inst_idx=0xff, custom_volume=0xff, custom_effect=0xff, custom_effect_data;
      (*m_row_callback)(m_row_callback_custom_data, ci, custom_note_idx, custom_inst_idx, custom_volume, custom_effect, custom_effect_data);
      if(custom_note_idx<12*10 || custom_note_idx==pmfcfg_note_cut || custom_note_idx==pmfcfg_note_off)
        note_idx=custom_note_idx;
      if(custom_inst_idx<pgm_read_byte(m_pmf_file+pmfcfg_offset_num_instruments))
        inst_idx=custom_inst_idx;
      if(custom_volume!=0xff)
        volume=custom_volume;
      if(custom_effect!=0xff)
      {
        effect=custom_effect;
        effect_data=custom_effect_data;
      }
    }

    if(note_idx!=0xff)
    {
      if(note_idx==pmfcfg_note_cut)
      {
        // stop sample playback
        chl.sample_speed=0;
        note_idx=0xff;
      }
      else if(note_idx==pmfcfg_note_off)
      {
        // release note
        if(!(chl.base_note_idx&128))
        {
          chl.base_note_idx|=128;
          chl.vol_fadeout=65535;
        }
        note_idx=0xff;
      }
      else
      {
        // reset envelopes
        chl.vol_env.tick=uint16_t(-1);
        chl.vol_env.pos=-1;
        chl.vol_env.value=0xffff;
        chl.pitch_env.tick=uint16_t(-1);
        chl.pitch_env.pos=-1;
        chl.pitch_env.value=0x8000;
      }
    }

    // check for instrument
    if(inst_idx!=0xff)
    {
      set_instrument(chl, inst_idx, note_idx);
      if(note_idx==0xff && chl.sample_speed)
      {
        note_idx=chl.base_note_idx&127;
        reset_sample_pos=false;
      }
    }
    if(note_idx!=0xff)
      note_idx+=chl.inst_note_idx_offs;

    // check for volume or volume effect
    chl.vol_effect=0xff;
    bool update_sample_speed=true;
    if(volume!=0xff)
    {
      if(volume<(1<<pmfcfg_num_volume_bits))
        chl.sample_volume=(volume<<2)|(volume>>4);
      else
      {
        // initialize volume effect
        uint8_t volfx_data=volume&0xf;
        switch(volume&0xf0)
        {
          // volume slide
          case pmfvolfx_vol_slide_down:
          case pmfvolfx_vol_slide_up:
          case pmfvolfx_vol_slide_fine_down:
          case pmfvolfx_vol_slide_fine_up:
          {
            if(init_effect_volume_slide(chl, volume&0x3f))
              chl.vol_effect=pmfvolfx_vol_slide;
          } break;

          // note slide down
          case pmfvolfx_note_slide_down:
          {
            // init note slide down and disable note retrigger
            if(init_effect_note_slide(chl, volfx_data, note_slide_down_target_period))
              chl.vol_effect=pmfvolfx_note_slide;
          } break;
          
          // note slide up
          case pmfvolfx_note_slide_up:
          {
            // init note slide up and disable note retrigger
            if(init_effect_note_slide(chl, volfx_data, note_slide_up_target_period))
              chl.vol_effect=pmfvolfx_note_slide;
          } break;

          // note slide
          case pmfvolfx_note_slide:
          {
            // init note slide and disable note retrigger
            if(init_effect_note_slide(chl, volfx_data, note_idx!=0xff?get_note_period(note_idx, chl.sample_finetune):0))
              chl.vol_effect=pmfvolfx_note_slide;
            note_idx=0xff;
          } break;

          // set vibrato speed
          case pmfvolfx_set_vibrato_speed:
          {
            if(volfx_data)
              chl.fxmem_vibrato_spd=volfx_data;
          } break;

          // vibrato
          case pmfvolfx_vibrato:
          {
            init_effect_vibrato(chl, volfx_data, 0);
            chl.vol_effect=pmfvolfx_vibrato;
            update_sample_speed=false;
          } break;

          // set panning
          case pmfvolfx_set_panning:
          {
            chl.sample_panning=volfx_data?(volfx_data|(volfx_data<<4))-128:-127;
          } break;

          // fine panning slide left
          case pmfvolfx_pan_slide_fine_left:
          {
            if(chl.sample_panning==-128)
              break;
            chl.sample_panning=int8_t(max(-127, int(chl.sample_panning)-volfx_data*4));
          } break;

          // fine panning slide right
          case pmfvolfx_pan_slide_fine_right:
          {
            if(chl.sample_panning==-128)
              break;
            chl.sample_panning=int8_t(min(127, int(chl.sample_panning)+volfx_data*4));
          } break;
        }
      }
    }

    // get effect
    chl.effect=0xff;
    if(effect!=0xff)
    {
      // setup effect
      switch(effect)
      {
        case pmffx_set_speed_tempo:
        {
          if(effect_data<32)
            m_speed=effect_data;
          else
            m_num_batch_samples=(m_sampling_freq*125)/long(effect_data*50);
        } break;

        case pmffx_position_jump:
        {
          m_current_pattern_playlist_pos=effect_data-1;
          m_current_pattern_row_idx=m_current_pattern_last_row;
        } break;

        case pmffx_pattern_break:
        {
          m_current_pattern_row_idx=m_current_pattern_last_row;
          num_skip_rows=effect_data;
        } break;

        case pmffx_volume_slide:
        {
          if(init_effect_volume_slide(chl, effect_data))
            chl.effect=pmffx_volume_slide;
        } break;

        case pmffx_note_slide_down:
        {
          if(init_effect_note_slide(chl, effect_data, note_slide_down_target_period))
            chl.effect=pmffx_note_slide;
        } break;

        case pmffx_note_slide_up:
        {
          if(init_effect_note_slide(chl, effect_data, note_slide_up_target_period))
            chl.effect=pmffx_note_slide;
        } break;

        case pmffx_note_slide:
        {
          // init note slide and disable retrigger
          if(init_effect_note_slide(chl, effect_data, note_idx!=0xff?get_note_period(note_idx, chl.sample_finetune):0))
            chl.effect=pmffx_note_slide;
          note_idx=0xff;
        } break;

        case pmffx_arpeggio:
        {
          chl.effect=pmffx_arpeggio;
          if(effect_data)
            chl.fxmem_arpeggio=effect_data;
        } break;

        case pmffx_vibrato:
        {
          // update vibrato attributes
          init_effect_vibrato(chl, effect_data&0x0f, effect_data>>4);
          chl.effect=pmffx_vibrato;
          update_sample_speed=false;
        } break;

        case pmffx_tremolo:
        {
          /*todo*/
        } break;

        case pmffx_note_vol_slide:
        {
          init_effect_note_slide(chl, 0, note_idx!=0xff?get_note_period(note_idx, chl.sample_finetune):0);
          init_effect_volume_slide(chl, effect_data);
          chl.effect=pmffx_note_vol_slide;
          note_idx=0xff;
        } break;

        case pmffx_vibrato_vol_slide:
        {
          init_effect_vibrato(chl, 0, 0);
          init_effect_volume_slide(chl, effect_data);
          chl.effect=pmffx_vibrato_vol_slide;
          update_sample_speed=false;
        } break;

        case pmffx_retrig_vol_slide:
        {
          chl.sample_pos=0;
          chl.fxmem_retrig_count=effect_data&0xf;
          chl.effect=pmffx_retrig_vol_slide;
          chl.effect_data=effect_data;
          chl.note_hit=1;
        } break;

        case pmffx_set_sample_offset:
        {
          sample_start_pos=effect_data;
          chl.sample_pos=effect_data*long(65536);
        } break;

        case pmffx_subfx:
        {
          switch(effect_data>>4)
          {
            case pmfsubfx_set_glissando:
            {
              /*todo*/
            } break;

            case pmfsubfx_set_finetune:
            {
              if(inst_idx!=0xff)
              {
                /*todo: should move C4hz values of instruments to RAM*/
              }
            } break;

            case pmfsubfx_set_vibrato_wave:
            {
              uint8_t wave=effect_data&3;
              chl.fxmem_vibrato_wave=(wave<3?wave:m_batch_pos%3)|(effect_data&4);
            } break;

            case pmfsubfx_set_tremolo_wave:
            {
              /*todo*/
            } break;

            case pmfsubfx_pattern_delay:
            {
              m_pattern_delay=(effect_data&0xf)+1;
            } break;

            case pmfsubfx_pattern_loop:
            {
              effect_data&=0xf;
              if(effect_data)
              {
                if(m_pattern_loop_cnt)
                  --m_pattern_loop_cnt;
                else
                  m_pattern_loop_cnt=effect_data;
                if(m_pattern_loop_cnt)
                  loop_pattern=true;
              }
              else
              {
                // set loop start
                m_pattern_loop_row_idx=m_current_pattern_row_idx;
                for(unsigned ci=0; ci<m_num_processed_pattern_channels; ++ci)
                {
                  audio_channel &chl=m_channels[ci];
                  chl.track_loop_pos=current_track_poss[ci];
                  chl.track_loop_bit_pos=current_track_bit_poss[ci];
                  memcpy(chl.track_loop_decomp_buf, chl.decomp_buf, sizeof(chl.track_loop_decomp_buf));
                }
              }
            } break;

            case pmfsubfx_note_cut:
            {
              chl.effect=pmffx_subfx|(pmfsubfx_note_cut<<pmfcfg_num_effect_bits);
              chl.effect_data=effect_data&0xf;
            } break;

            case pmfsubfx_note_delay:
            {
              if(note_idx==0xff)
                break;
              chl.effect=pmffx_subfx|(pmfsubfx_note_delay<<pmfcfg_num_effect_bits);
              chl.effect_data=effect_data&0xf;
              chl.fxmem_note_delay_idx=note_idx;
              note_idx=0xff;
            } break;
          }
        } break;

        case pmffx_panning:
        {
          // check for panning slide
          if(effect_data&pmffx_pansldtype_enable_mask)
          {
            // check valid panning state for sliding and update effect memory
            if(chl.sample_panning==-128)
              break;
            uint8_t panning_spd=effect_data&pmffx_pansldtype_val_mask;
            if(panning_spd)
              chl.fxmem_panning_spd=effect_data;
            else
            {
              effect_data=chl.fxmem_panning_spd;
              panning_spd=effect_data&pmffx_pansldtype_val_mask;
            }

            // apply fine slide or setup normal slide
            if(effect_data&pmffx_pansldtype_fine_mask)
            {
              panning_spd*=4;
              chl.sample_panning=int8_t(effect_data&pmffx_pansldtype_dir_mask?min(127, int(chl.sample_panning)+panning_spd):max(-127, int(chl.sample_panning)-panning_spd));
            }
            else
              chl.effect=pmffx_panning;
          }
          else
            chl.sample_panning=effect_data<<1;
        } break;
      }
    }

    // check for note hit
    if(note_idx!=0xff)
      hit_note(chl, note_idx, sample_start_pos, reset_sample_pos);
    else if(update_sample_speed && chl.sample_speed)
      chl.sample_speed=get_sample_speed(chl.note_period, chl.sample_speed>=0);
  }

  // check for pattern loop
  if(loop_pattern)
  {
    for(unsigned ci=0; ci<m_num_processed_pattern_channels; ++ci)
    {
      audio_channel &chl=m_channels[ci];
      chl.track_pos=chl.track_loop_pos;
      chl.track_bit_pos=chl.track_loop_bit_pos;
      memcpy(chl.decomp_buf, chl.track_loop_decomp_buf, sizeof(chl.decomp_buf));
    }
    m_current_pattern_row_idx=m_pattern_loop_row_idx-1;
  }

  // advance pattern
  if(m_current_pattern_row_idx++==m_current_pattern_last_row)
  {
    // proceed to the next pattern
    if(++m_current_pattern_playlist_pos==pgm_read_word(m_pmf_file+pmfcfg_offset_playlist_length))
      m_current_pattern_playlist_pos=0;
    init_pattern(m_current_pattern_playlist_pos, num_skip_rows);
  }
}
//----

void pmf_player::process_track_row(audio_channel &chl_, uint8_t &note_idx_, uint8_t &inst_idx_, uint8_t &volume_, uint8_t &effect_, uint8_t &effect_data_)
{
  // get data mask
  if(chl_.track_pos==m_pmf_file)
    return;
  uint8_t data_mask=0;
  bool read_dmask=false;
  switch(chl_.decomp_type&0x03)
  {
    case 0x0: read_dmask=true; break;
    case 0x1: read_dmask=read_bits(chl_.track_pos, chl_.track_bit_pos, 1)&1; break;
    case 0x2:
    {
      switch(read_bits(chl_.track_pos, chl_.track_bit_pos, 2)&3)
      {
        case 0x1: read_dmask=true; break;
        case 0x2: data_mask=chl_.decomp_buf[5][0]; break;
        case 0x3: data_mask=chl_.decomp_buf[5][1]; break;
      }
    } break;
  }
  if(read_dmask)
  {
    data_mask=read_bits(chl_.track_pos, chl_.track_bit_pos, chl_.decomp_type&0x4?8:4)&(chl_.decomp_type&4?0xff:0x0f);
    chl_.decomp_buf[5][1]=chl_.decomp_buf[5][0];
    chl_.decomp_buf[5][0]=data_mask;
  }

  // get note
  switch(data_mask&0x11)
  {
    case 0x01:
    {
      note_idx_=read_bits(chl_.track_pos, chl_.track_bit_pos, pmfcfg_num_note_bits)&((1<<pmfcfg_num_note_bits)-1);
      chl_.decomp_buf[0][1]=chl_.decomp_buf[0][0];
      chl_.decomp_buf[0][0]=note_idx_;
    } break;
    case 0x10: note_idx_=chl_.decomp_buf[0][0]; break;
    case 0x11: note_idx_=chl_.decomp_buf[0][1]; break;
  }

  // get instrument
  switch(data_mask&0x22)
  {
    case 0x02:
    {
      inst_idx_=read_bits(chl_.track_pos, chl_.track_bit_pos, pmfcfg_num_instrument_bits)&((1<<pmfcfg_num_instrument_bits)-1);
      chl_.decomp_buf[1][1]=chl_.decomp_buf[1][0];
      chl_.decomp_buf[1][0]=inst_idx_;
    } break;
    case 0x20: inst_idx_=chl_.decomp_buf[1][0]; break;
    case 0x22: inst_idx_=chl_.decomp_buf[1][1]; break;
  }

  // get volume
  switch(data_mask&0x44)
  {
    case 0x04:
    {
      uint8_t num_volume_bits=chl_.decomp_type&0x8?pmfcfg_num_volume_bits+2:pmfcfg_num_volume_bits;
      volume_=read_bits(chl_.track_pos, chl_.track_bit_pos, num_volume_bits)&((1<<num_volume_bits)-1);
      chl_.decomp_buf[2][1]=chl_.decomp_buf[2][0];
      chl_.decomp_buf[2][0]=volume_;
    } break;
    case 0x40: volume_=chl_.decomp_buf[2][0]; break;
    case 0x44: volume_=chl_.decomp_buf[2][1]; break;
  }

  // get effect
  switch(data_mask&0x88)
  {
    case 0x08:
    {
      effect_=read_bits(chl_.track_pos, chl_.track_bit_pos, pmfcfg_num_effect_bits)&((1<<pmfcfg_num_effect_bits)-1);
      effect_data_=read_bits(chl_.track_pos, chl_.track_bit_pos, pmfcfg_num_effect_data_bits)&((1<<pmfcfg_num_effect_data_bits)-1);
      chl_.decomp_buf[3][1]=chl_.decomp_buf[3][0];
      chl_.decomp_buf[3][0]=effect_;
      chl_.decomp_buf[4][1]=chl_.decomp_buf[4][0];
      chl_.decomp_buf[4][0]=effect_data_;
    } break;
    case 0x80: effect_=chl_.decomp_buf[3][0]; effect_data_=chl_.decomp_buf[4][0]; break;
    case 0x88: effect_=chl_.decomp_buf[3][1]; effect_data_=chl_.decomp_buf[4][1]; break;
  }
}
//----

void pmf_player::init_pattern(uint8_t playlist_pos_, uint8_t row_)
{
  // set state
  m_current_pattern_playlist_pos=playlist_pos_;
  m_current_pattern_row_idx=row_;
  m_pattern_loop_cnt=0;
  m_pattern_loop_row_idx=0;

  // initialize pattern at given playlist location and pattern row
  const uint8_t *pattern=m_pmf_file+pgm_read_dword(m_pmf_file+pmfcfg_offset_pat_meta_offs)+pgm_read_byte(m_pmf_file+pmfcfg_offset_playlist+playlist_pos_)*(pmfcfg_pattern_metadata_header_size+pmfcfg_pattern_metadata_track_offset_size*m_num_pattern_channels);
  m_current_pattern_last_row=pgm_read_byte(pattern+pmfcfg_offset_pattern_metadata_last_row);
  for(unsigned ci=0; ci<m_num_processed_pattern_channels; ++ci)
  {
    // init audio track
    audio_channel &chl=m_channels[ci];
    uint16_t track_offs=pgm_read_word(pattern+pmfcfg_offset_pattern_metadata_track_offsets+ci*pmfcfg_pattern_metadata_track_offset_size);
    chl.track_pos=m_pmf_file+track_offs;
    chl.track_bit_pos=0;
    chl.track_loop_pos=chl.track_pos;
    chl.track_loop_bit_pos=chl.track_bit_pos;
    if(track_offs)
      chl.decomp_type=read_bits(chl.track_pos, chl.track_bit_pos, 4)&15;

    // skip to given row
    uint8_t note_idx, inst_idx, volume, effect, effect_data;
    for(unsigned ri=0; ri<row_; ++ri)
      process_track_row(chl, note_idx, inst_idx, volume, effect, effect_data);
  }
}
//---------------------------------------------------------------------------
