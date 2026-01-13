/*
===============================================================================
  music_map.h — Table de correspondance niveaux → musiques
-------------------------------------------------------------------------------
  Rôle :
    - Définir un identifiant pour chaque musique disponible
    - Associer chaque niveau à une musique
    - Fournir une fonction utilitaire pour récupérer la musique d’un niveau

  Notes :
    - Ajoutez simplement une entrée dans levelMusicMap pour assigner une musique
      à un niveau existant ou futur.
    - Les musiques sont déclarées dans assets/music/(*).h
===============================================================================
*/

#pragma once
#include "assets/music/baba_samba_la_baba.h"
#include "assets/music/baba_music_2.h"
#include "assets/music/baba_cave_newer_short.h"
#include "assets/music/CRYSTAL.h"
#include "assets/music/MISTHART.h"
#include "assets/music/WF-DRAGO.h"
#include "assets/music/WF-MAGES.h"

enum class MusicID {
    BABA_SAMBA,
    BABA_MUSIC_2,
    BABA_CAVE,
    CRYSTAL,
    MISTHART,
    WF_DRAGO,
    WF_MAGES,
    NONE
};

struct LevelMusic {
    int level;
    MusicID music;
};

// Tableau extensible : tu peux modifier ou ajouter des lignes librement
static const LevelMusic levelMusicMap[] = {
    { 0, MusicID::BABA_SAMBA },
    { 1, MusicID::BABA_MUSIC_2 },
    { 2, MusicID::BABA_CAVE },
    { 3, MusicID::CRYSTAL },
    { 4, MusicID::MISTHART },
    { 5, MusicID::WF_DRAGO },
    { 6, MusicID::WF_MAGES },
    { 7, MusicID::BABA_SAMBA },
    { 8, MusicID::BABA_MUSIC_2 },
    { 9, MusicID::BABA_CAVE },
    { 10, MusicID::CRYSTAL },
    { 11, MusicID::MISTHART },
    { 12, MusicID::WF_DRAGO },
    { 13, MusicID::WF_MAGES },
    { 14, MusicID::BABA_SAMBA },
    { 15, MusicID::BABA_MUSIC_2 },
    { 16, MusicID::BABA_CAVE },
    { 17, MusicID::CRYSTAL },
    { 18, MusicID::MISTHART },
    { 19, MusicID::WF_DRAGO },
    { 20, MusicID::WF_MAGES },
};

inline MusicID getMusicForLevel(int level)
{
    for (auto& lm : levelMusicMap)
        if (lm.level == level)
            return lm.music;
    return MusicID::NONE;
}
