/*
===============================================================================
  baba_music_2.h — Ressource musicale (PMD converti en données C)
-------------------------------------------------------------------------------
  Rôle :
    - Contient les données binaires de la musique "Baba Music 2"
    - Format : tableau C statique généré depuis PMD
    - Utilisé par le moteur audio pour lecture via I2S + DMA
===============================================================================
*/

#pragma once

extern const unsigned char baba_music_2_pmf[];
extern const unsigned int baba_music_2_pmf_len;
