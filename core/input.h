/*
===============================================================================
  input.h — Interface du système d’entrée
-------------------------------------------------------------------------------
  Rôle :
    - Définir la structure Keys (état complet des entrées).
    - Fournir les fonctions d’initialisation et de polling.
    - Offrir un helper pour détecter les pressions longues.

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#pragma once
#include <cstdint>

struct Keys {
    uint32_t raw;
    uint32_t pressed;
    uint32_t released;

    int joxx, joxy;

    bool up, down, left, right;
    bool A, B, C, D;
    bool RUN, MENU, R1, L1;

    int joyX;
    int joyY;
};

extern Keys g_keys;

void input_init();
void input_poll(Keys& k);

bool isLongPress(const Keys& k, int key);
