/*
===============================================================================
  CRYSTAL.h — Ressource musicale (PMD converti en données C)
-------------------------------------------------------------------------------
  Rôle :
    - Contient les données binaires de la musique "Crystal"
    - Format : tableau C statique généré depuis PMD
    - Utilisé par le moteur audio pour lecture via I2S + DMA
===============================================================================
*/

#pragma once

extern const unsigned char CRYSTAL_pmf[];
extern const unsigned int CRYSTAL_pmf_len;
