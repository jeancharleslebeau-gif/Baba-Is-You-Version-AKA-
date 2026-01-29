/*
===============================================================================
  filesystem.h — Helpers système de fichiers (SD / FAT / VFS)
-------------------------------------------------------------------------------
  Rôle :
    - Fournir une interface simple et portable pour :
        * tester l’existence d’un fichier ou dossier
        * créer un dossier
        * lire un fichier texte
        * écrire un fichier texte
    - Abstraire les appels POSIX (stat, mkdir, fopen…) utilisés par l’AKA.
    - Éviter de polluer game.cpp avec des fonctions utilitaires.

  Notes :
    - Le système de fichiers est monté via esp_vfs_fat_sdmmc_mount().
    - Toutes les fonctions utilisent l’API POSIX exposée par le VFS.
    - Compatible avec la SD Gamebuino‑AKA.
===============================================================================
*/

#pragma once
#include <string>

namespace baba {

// Vérifie si un fichier ou dossier existe
bool fs_exists(const char* path);

// Crée un dossier (retourne true si succès)
bool fs_mkdir(const char* path);

// Écrit un texte complet dans un fichier (overwrite)
bool fs_write_text(const char* path, const char* text);

// Lit un fichier texte entier dans une std::string
bool fs_read_text(const char* path, std::string& out);

} // namespace baba
