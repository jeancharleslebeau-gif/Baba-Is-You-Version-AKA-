/*
===============================================================================
  filesystem.cpp — Implémentation helpers système de fichiers
-------------------------------------------------------------------------------
  Rôle :
    - Implémenter les fonctions déclarées dans filesystem.h.
    - Utiliser les appels POSIX fournis par le VFS FAT de l’ESP32.
    - Fournir une couche simple pour le moteur Baba (sauvegardes, niveaux…)
===============================================================================
*/

#include "core/filesystem.h"
#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdio.h>

namespace baba {

/*
-------------------------------------------------------------------------------
  fs_exists(path)
    → Retourne true si le fichier ou dossier existe.
    → Utilise stat(), compatible avec le VFS FAT monté sur la SD.
-------------------------------------------------------------------------------
*/
bool fs_exists(const char* path)
{
    struct stat st;
    return (stat(path, &st) == 0);
}

/*
-------------------------------------------------------------------------------
  fs_mkdir(path)
    → Crée un dossier.
    → Retourne true si succès, false sinon.
    → mkdir() échoue si le dossier existe déjà (ce n’est pas grave).
-------------------------------------------------------------------------------
*/
bool fs_mkdir(const char* path)
{
    return (mkdir(path, 0777) == 0);
}

/*
-------------------------------------------------------------------------------
  fs_write_text(path, text)
    → Écrit un texte complet dans un fichier (overwrite).
    → Retourne false si le fichier ne peut pas être ouvert.
-------------------------------------------------------------------------------
*/
bool fs_write_text(const char* path, const char* text)
{
    FILE* f = fopen(path, "w");
    if (!f) return false;

    fputs(text, f);
    fclose(f);
    return true;
}

/*
-------------------------------------------------------------------------------
  fs_read_text(path, out)
    → Lit un fichier texte entier dans une std::string.
    → Retourne false si le fichier n’existe pas ou ne peut être ouvert.
-------------------------------------------------------------------------------
*/
bool fs_read_text(const char* path, std::string& out)
{
    FILE* f = fopen(path, "r");
    if (!f) return false;

    char buf[512];
    out.clear();

    while (fgets(buf, sizeof(buf), f))
        out += buf;

    fclose(f);
    return true;
}

} // namespace baba
