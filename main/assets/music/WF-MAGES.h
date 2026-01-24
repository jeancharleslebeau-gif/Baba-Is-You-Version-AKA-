/*
===============================================================================
  WF-MAGES.h — Ressource musicale (PMD converti en données C)
-------------------------------------------------------------------------------
  Rôle :
    - Contient les données binaires de la musique "WF-Mages"
    - Format : tableau C statique généré depuis PMD
    - Utilisé par le moteur audio pour lecture via I2S + DMA
===============================================================================
*/

#pragma once

extern const unsigned char WF_MAGES_pmf[];
extern const unsigned int WF_MAGES_pmf_len;
