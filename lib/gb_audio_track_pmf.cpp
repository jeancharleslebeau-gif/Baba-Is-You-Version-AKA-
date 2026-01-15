/*
This file is part of the Gamebuino-AKA library,
Copyright (c) Gamebuino 2026

This is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License (LGPL)
as published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License (LGPL) for more details.

You should have received a copy of the GNU Lesser General Public
License (LGPL) along with the library.
If not, see <http://www.gnu.org/licenses/>.

Authors:
 - Jean-Marie Papillon
*/

#include "gb_audio_track_pmf.h"
#include "gb_ll_audio.h"      // GB_AUDIO_SAMPLE_RATE
#include <string.h>           // memset
#include <stdio.h>            // printf (debug)

/*
  ----------------------------------------------------------------------------
  gb_audio_track_pmf
  ----------------------------------------------------------------------------
  Rôle :
    - Adapter pmf_player (lib externe) au moteur audio Gamebuino-AKA.
    - Fournir une interface simple :
        * load_pmf()  : associer un fichier PMF en mémoire (const void*)
        * play_pmf()  : démarrer la lecture
        * stop_playing() / is_playing()
        * play_callback() : remplir un buffer audio 16 bits signé

  Contraintes :
    - Le buffer PMF doit rester valide en mémoire pendant toute la lecture.
    - Le moteur audio AKA appelle play_callback() à cadence fixe, en lui
      fournissant un buffer de u16_sample_count échantillons (mono 16 bits).
    - Aucune allocation dynamique ici : tout est géré dans pmf_player.
*/

// ---------------------------------------------------------------------------
// Chargement du fichier PMF (en mémoire)
// ---------------------------------------------------------------------------

void gb_audio_track_pmf::load_pmf(const void *pmem_pmf_file)
{
    // Sécurité : ne rien faire si pointeur nul
    if (!pmem_pmf_file)
    {
        printf("[PMF] load_pmf: null pointer, ignoring\n");
        return;
    }

    // pmf_player::load vérifie déjà la signature, la version, etc.
    _pmf_player.load(pmem_pmf_file);
}

// ---------------------------------------------------------------------------
// Démarrage de la lecture (sur PMF déjà chargé)
// ---------------------------------------------------------------------------

void gb_audio_track_pmf::play_pmf()
{
    // Si aucun fichier PMF valide n’a été chargé, start() ne fera rien.
    // On ne teste pas ici l’état interne : pmf_player gère sa propre cohérence.
    _pmf_player.start(GB_AUDIO_SAMPLE_RATE);
}

// ---------------------------------------------------------------------------
// Chargement + démarrage en une seule étape
// ---------------------------------------------------------------------------

void gb_audio_track_pmf::play_pmf(const void *pmem_pmf_file)
{
    if (!pmem_pmf_file)
    {
        printf("[PMF] play_pmf: null pointer, ignoring\n");
        return;
    }

    _pmf_player.load(pmem_pmf_file);
    _pmf_player.start(GB_AUDIO_SAMPLE_RATE);
}

// ---------------------------------------------------------------------------
// État de lecture
// ---------------------------------------------------------------------------

bool gb_audio_track_pmf::is_playing()
{
    return _pmf_player.is_playing();
}

// ---------------------------------------------------------------------------
// Arrêt de la lecture
// ---------------------------------------------------------------------------

void gb_audio_track_pmf::stop_playing()
{
    _pmf_player.stop();
}

// ---------------------------------------------------------------------------
// Callback de remplissage du buffer audio
// ---------------------------------------------------------------------------
// Contrat :
//   - pi16_buffer : buffer mono 16 bits signé, taille u16_sample_count
//   - u16_sample_count : nombre d’échantillons à produire
//   - Retour :
//       0  = le morceau est en cours de lecture (buffer rempli)
//      -1  = rien à jouer (silence, buffer laissé tel quel ou mis à zéro)
//
// Remarques :
//   - On efface le buffer avant de mixer pour éviter des résidus si le
//     mixeur interne ne remplit pas tout (par design, il remplit tout).
//   - Si le player n’est pas en lecture, on renvoie -1 pour que le moteur
//     puisse éventuellement enchaîner sur autre chose.
//
int gb_audio_track_pmf::play_callback(int16_t *pi16_buffer,
                                      uint16_t u16_sample_count)
{
    if (!pi16_buffer || u16_sample_count == 0)
        return -1;

    if (_pmf_player.is_playing())
    {
        // On part d’un buffer silencieux
        memset(pi16_buffer, 0, sizeof(int16_t) * u16_sample_count);

        // pmf_player::mix() mélange directement dans le buffer fourni
        _pmf_player.mix(pi16_buffer, u16_sample_count);

        return 0; // playing
    }

    // Pas de lecture en cours → silence (le moteur peut décider quoi faire)
    memset(pi16_buffer, 0, sizeof(int16_t) * u16_sample_count);
    return -1;
}

// ===========================================================================
// Implémentation "plateforme" de pmf_player pour Gamebuino-AKA
// ===========================================================================
//
// Ici, on adapte les hooks abstraits de pmf_player à notre backend.
// Dans ce backend, c’est le moteur audio AKA qui pilote la cadence et
// appelle pmf_player::mix(), donc :
//   - get_sampling_freq() : fixe la fréquence à GB_AUDIO_SAMPLE_RATE
//   - start_playback() / stop_playback() : pas d’action (pilotage externe)
//   - get_mixer_buffer() : non utilisé (on passe par mix())
//   - mix_buffer() : délègue au mixeur interne templatisé
//

uint32_t pmf_player::get_sampling_freq(uint32_t sampling_freq_) const
{
  (void)sampling_freq_;
  // Le moteur tourne déjà à une fréquence fixe connue (I2S).
  // On force pmf_player à utiliser cette fréquence pour tous ses calculs.
  return GB_AUDIO_SAMPLE_RATE;
}

void pmf_player::start_playback(uint32_t sampling_freq_)
{
  // Dans ce backend, le démarrage réel est géré par le moteur audio AKA.
  // pmf_player::start() initialise son état interne, puis appelle ici.
  (void)sampling_freq_;
}

void pmf_player::stop_playback()
{
  // Idem : l’arrêt “physique” (I2S, DMA…) est géré ailleurs.
  // Ici, on ne fait que respecter l’interface.
}

pmf_mixer_buffer pmf_player::get_mixer_buffer()
{
  // Backend “pull” : on ne passe pas par update()+get_mixer_buffer(),
  // mais par mix(out_buffer, num_samples).
  pmf_mixer_buffer buf = { 0, 0 };
  return buf;
}

void pmf_player::mix_buffer(pmf_mixer_buffer &buf_, unsigned num_samples_)
{
  // Implémentation générique fournie par la lib Profoundic.
  // On garde la config : int16_t, pas de clipping hard, 16 bits de headroom.
  mix_buffer_impl<int16_t, false, 16>(buf_, num_samples_);
}
