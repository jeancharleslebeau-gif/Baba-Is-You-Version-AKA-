/*
===============================================================================
  types.cpp — Utilitaires liés aux types d’objets du moteur Baba
-------------------------------------------------------------------------------
  Rôle :
    - Fournir une conversion ObjectType → texte (export, debug, éditeur).
    - Fournir une conversion texte → ObjectType (import niveaux custom).
    - Centraliser les noms des objets pour éviter toute duplication.
    - S’appuyer sur l’énumération ObjectType définie dans types.h.

  Notes :
    - L’ordre des noms doit correspondre EXACTEMENT à l’ordre de ObjectType.
    - Toute modification de l’énumération doit être répercutée ici.
===============================================================================
*/

#include "types.h"
#include <cstring>   // strcmp

namespace baba {

// ============================================================================
//  Table des noms des objets
//  IMPORTANT : l’ordre doit correspondre à ObjectType dans types.h
// ============================================================================
static const char* OBJECT_NAMES[(int)ObjectType::Count] = {

    // Objets physiques
    "Baba",
    "Wall",
    "Rock",
    "Flag",
    "Lava",
    "Goop",
    "Love",
    "Empty",
    "Key",
    "Door",
    "Water",
    "Ice",
    "Box",

    // Mots
    "Text_Baba",
    "Text_Wall",
    "Text_Rock",
    "Text_Flag",
    "Text_Lava",
    "Text_Goop",
    "Text_Love",
    "Text_Empty",
    "Text_Key",
    "Text_Door",
    "Text_Water",
    "Text_Ice",
    "Text_Box",
    "Text_Is",
    "Text_And",

    "Text_Push",
    "Text_Stop",
    "Text_Win",
    "Text_You",
    "Text_Sink",
    "Text_Kill",
    "Text_Swap",
    "Text_Hot",
    "Text_Melt",
    "Text_Move",
    "Text_Open",
    "Text_Shut",
    "Text_Float",
    "Text_Pull"
};

// ============================================================================
//  Conversion ObjectType → texte
//  Retourne "Unknown" si l’index est invalide.
// ============================================================================
const char* object_type_to_text(ObjectType t)
{
    int i = (int)t;
    if (i < 0 || i >= (int)ObjectType::Count)
        return "Unknown";
    return OBJECT_NAMES[i];
}

// ============================================================================
//  Conversion texte → ObjectType
//  Utile pour parser les niveaux custom (CSV, texte brut, éditeur).
//  Retourne ObjectType::Empty si non trouvé.
// ============================================================================
ObjectType parse_object_type(const char* name)
{
    for (int i = 0; i < (int)ObjectType::Count; i++) {
        if (std::strcmp(name, OBJECT_NAMES[i]) == 0)
            return (ObjectType)i;
    }
    return ObjectType::Empty;
}

} // namespace baba
