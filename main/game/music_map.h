/*
===============================================================================
  music_map.h — Table indexée niveaux → musiques (0 → 39)
-------------------------------------------------------------------------------
  Rôle :
    - Associer chaque niveau à une musique via un tableau indexé
    - Lookup O(1), aucune boucle, aucune ambiguïté
    - Compatible avec levels_count() = 40
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

// -----------------------------------------------------------------------------
//  Tableau indexé : musicForLevel[level] → MusicID
//  40 niveaux (0 → 39), conforme à levels_count() = 40
// -----------------------------------------------------------------------------
static const MusicID musicForLevel[40] = {
    /*  0 */ MusicID::BABA_SAMBA,
    /*  1 */ MusicID::BABA_MUSIC_2,
    /*  2 */ MusicID::BABA_CAVE,
    /*  3 */ MusicID::CRYSTAL,
    /*  4 */ MusicID::MISTHART,
    /*  5 */ MusicID::WF_DRAGO,
    /*  6 */ MusicID::WF_MAGES,
    /*  7 */ MusicID::BABA_SAMBA,
    /*  8 */ MusicID::BABA_MUSIC_2,
    /*  9 */ MusicID::BABA_CAVE,

    /* 10 */ MusicID::CRYSTAL,
    /* 11 */ MusicID::MISTHART,
    /* 12 */ MusicID::WF_DRAGO,
    /* 13 */ MusicID::WF_MAGES,
    /* 14 */ MusicID::BABA_SAMBA,
    /* 15 */ MusicID::BABA_MUSIC_2,
    /* 16 */ MusicID::BABA_CAVE,
    /* 17 */ MusicID::CRYSTAL,
    /* 18 */ MusicID::MISTHART,
    /* 19 */ MusicID::WF_DRAGO,

    /* 20 */ MusicID::WF_MAGES,
    /* 21 */ MusicID::BABA_SAMBA,
    /* 22 */ MusicID::BABA_CAVE,
    /* 23 */ MusicID::CRYSTAL,
    /* 24 */ MusicID::MISTHART,
    /* 25 */ MusicID::WF_DRAGO,
    /* 26 */ MusicID::WF_MAGES,
    /* 27 */ MusicID::BABA_SAMBA,
    /* 28 */ MusicID::BABA_MUSIC_2,
    /* 29 */ MusicID::BABA_CAVE,

    /* 30 */ MusicID::CRYSTAL,
    /* 31 */ MusicID::MISTHART,
    /* 32 */ MusicID::WF_DRAGO,
    /* 33 */ MusicID::WF_MAGES,
    /* 34 */ MusicID::BABA_SAMBA,
    /* 35 */ MusicID::BABA_MUSIC_2,
    /* 36 */ MusicID::BABA_CAVE,
    /* 37 */ MusicID::CRYSTAL,
    /* 38 */ MusicID::MISTHART,
    /* 39 */ MusicID::WF_DRAGO
};

// -----------------------------------------------------------------------------
//  Récupération directe : O(1)
// -----------------------------------------------------------------------------
inline MusicID getMusicForLevel(int level)
{
    if (level < 0 || level >= 40)
        return MusicID::NONE;

    return musicForLevel[level];
}
