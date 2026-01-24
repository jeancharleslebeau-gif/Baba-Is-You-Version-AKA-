/*
===============================================================================
  MISTHART.h — Ressource musicale (PMD converti en données C)
-------------------------------------------------------------------------------
  Rôle :
    - Contient les données binaires de la musique "Misthart"
    - Format : tableau C statique généré depuis PMD
    - Utilisé par le moteur audio pour lecture via I2S + DMA
===============================================================================
*/

#pragma once

extern const unsigned char MISTHART_pmf[];
extern const unsigned int MISTHART_pmf_len;
