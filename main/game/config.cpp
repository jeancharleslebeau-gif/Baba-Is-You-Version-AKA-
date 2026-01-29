/*
===============================================================================
  config.cpp — Gestion de la configuration persistante du moteur Baba
-------------------------------------------------------------------------------
  Rôle :
    - Charger et sauvegarder les options utilisateur :
        * son activé/désactivé
        * musique forcée
        * persistance musique forcée
    - Stockage sur SD via core/filesystem.
===============================================================================
*/

#include "config.h"
#include "core/filesystem.h"
#include "game.h"        // soundEnabled, game_set_forced_music()
#include <string>

namespace baba {

static const char* CONFIG_PATH = "/babaisyou/config.txt";

// -----------------------------------------------------------------------------
//  Chargement configuration
// -----------------------------------------------------------------------------
void load_config()
{
    std::string text;
    if (!fs_read_text(CONFIG_PATH, text)) {
        // Pas de fichier → valeurs par défaut
        soundEnabled = true;
        game_set_forced_music(MusicID::NONE, false);
        return;
    }

    // Format simple :
    // sound=1
    // forcedMusic=3
    // persist=0

    int sound = 1;
    int forced = 0;
    int persist = 0;

    sscanf(text.c_str(),
           "sound=%d\nforcedMusic=%d\npersist=%d",
           &sound, &forced, &persist);

    soundEnabled = (sound != 0);

    if (forced > 0 && forced < (int)MusicID::COUNT)
        game_set_forced_music((MusicID)forced, persist != 0);
    else
        game_set_forced_music(MusicID::NONE, false);
}

// -----------------------------------------------------------------------------
//  Sauvegarde configuration
// -----------------------------------------------------------------------------
void save_config()
{
    char buf[128];

    snprintf(buf, sizeof(buf),
             "sound=%d\nforcedMusic=%d\npersist=%d\n",
             soundEnabled ? 1 : 0,
             (int)game_get_forced_music(),
             0 // TODO : persistance musique forcée
    );

    fs_write_text(CONFIG_PATH, buf);
}

} // namespace baba
