/*
===============================================================================
  WF-DRAGO.h — Ressource musicale (PMD converti en données C)
-------------------------------------------------------------------------------
  Rôle :
    - Contient les données binaires de la musique "WF-Drago"
    - Format : tableau C statique généré depuis PMD
    - Utilisé par le moteur audio pour lecture via I2S + DMA
===============================================================================
*/

#pragma once

extern const unsigned char WF_DRAGO_pmf[];
extern const unsigned int WF_DRAGO_pmf_len;
