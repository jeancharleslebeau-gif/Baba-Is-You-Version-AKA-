/*
===============================================================================
  rules.cpp — Implémentation du moteur de règles
-------------------------------------------------------------------------------
  Rôle :
    - Scanner la grille pour détecter les triplets (SUBJECT IS STATUS).
    - Remplir la PropertyTable utilisée par le moteur de mouvement.
    - Gérer les règles horizontales et verticales.

  Limitations actuelles :
    - Les règles "SUBJECT IS SUBJECT" (transformations) ne sont pas encore
      appliquées ici. Elles seront traitées dans le moteur de gameplay.
    - Les règles composées (ex : BABA IS YOU AND WIN) ne sont pas encore gérées.
    - Les propriétés avancées (HOT/MELT, OPEN/SHUT, MOVE…) sont reconnues
      mais pas encore appliquées dans movement.cpp.

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#include "rules.h"

namespace baba {

// ============================================================================
//  Classification des mots
// ============================================================================
/*
    is_word() :
      Retourne true si le type correspond à un mot TEXT_*.
*/
bool is_word(ObjectType t) {
    return t >= ObjectType::Text_Baba;
}

/*
    is_subject_word() :
      Retourne true si le mot peut apparaître en position SUBJECT.
      Exemple : TEXT_BABA, TEXT_ROCK, TEXT_FLAG…
*/
bool is_subject_word(ObjectType t) {
    switch (t) {
        case ObjectType::Text_Baba:
        case ObjectType::Text_Wall:
        case ObjectType::Text_Rock:
        case ObjectType::Text_Flag:
        case ObjectType::Text_Lava:
        case ObjectType::Text_Goop:
        case ObjectType::Text_Love:
        case ObjectType::Text_Empty:
            return true;
        default:
            return false;
    }
}

/*
    is_status_word() :
      Retourne true si le mot peut apparaître en position STATUS.
      Exemple : TEXT_PUSH, TEXT_STOP, TEXT_WIN…
*/
bool is_status_word(ObjectType t) {
    switch (t) {
        case ObjectType::Text_Push:
        case ObjectType::Text_Stop:
        case ObjectType::Text_Win:
        case ObjectType::Text_You:
        case ObjectType::Text_Sink:
        case ObjectType::Text_Kill:
        case ObjectType::Text_Swap:
        case ObjectType::Text_Hot:
        case ObjectType::Text_Melt:
        case ObjectType::Text_Move:
        case ObjectType::Text_Open:
        case ObjectType::Text_Shut:
        case ObjectType::Text_Float:
            return true;
        default:
            return false;
    }
}

// ============================================================================
//  Conversion TEXT_BABA → BABA
// ============================================================================
/*
    subject_to_object() :
      Convertit un mot SUBJECT en son objet correspondant.
      Exemple : TEXT_ROCK → ROCK.
*/
ObjectType subject_to_object(ObjectType word) {
    switch (word) {
        case ObjectType::Text_Baba: return ObjectType::Baba;
        case ObjectType::Text_Wall: return ObjectType::Wall;
        case ObjectType::Text_Rock: return ObjectType::Rock;
        case ObjectType::Text_Flag: return ObjectType::Flag;
        case ObjectType::Text_Lava: return ObjectType::Lava;
        case ObjectType::Text_Goop: return ObjectType::Goop;
        case ObjectType::Text_Love: return ObjectType::Love;
        case ObjectType::Text_Empty:return ObjectType::Empty;
        default: return ObjectType::Empty;
    }
}

// ============================================================================
//  Applique une propriété à un ensemble Properties
// ============================================================================
/*
    apply_status() :
      Active un attribut dans la structure Properties.
      Exemple : TEXT_PUSH → p.isPush = true.
*/
void apply_status(Properties& p, ObjectType s) {
    switch (s) {
        case ObjectType::Text_You:  p.isYou=true; break;
        case ObjectType::Text_Push: p.isPush=true; break;
        case ObjectType::Text_Stop: p.isStop=true; break;
        case ObjectType::Text_Win:  p.isWin=true; break;
        case ObjectType::Text_Sink: p.isSink=true; break;
        case ObjectType::Text_Kill: p.isKill=true; break;
        case ObjectType::Text_Hot:  p.isHot=true; break;
        case ObjectType::Text_Melt: p.isMelt=true; break;
        case ObjectType::Text_Move: p.isMove=true; break;
        case ObjectType::Text_Open: p.isOpen=true; break;
        case ObjectType::Text_Shut: p.isShut=true; break;
        case ObjectType::Text_Float:p.isFloat=true; break;
        default: break;
    }
}

// ============================================================================
//  Réinitialise toutes les propriétés
// ============================================================================
/*
    rules_reset() :
      Remet toutes les propriétés à false sauf les mots toujours déplaçables par défaut
*/
void rules_reset(PropertyTable& table) {
    for (size_t i = 0; i < table.size(); i++) {
        table[i] = Properties{};

        // ✅ Les mots TEXT_* sont toujours PUSH
        if (is_word((ObjectType)i)) {
            table[i].isPush = true;
        }
    }
}


// ============================================================================
//  Analyse la grille et extrait les règles actives
// ============================================================================
/*
    rules_parse() :
      Parcourt la grille pour détecter les triplets :
          SUBJECT — IS — STATUS

      Deux scans :
        - horizontal : (x,y), (x+1,y), (x+2,y)
        - vertical   : (x,y), (x,y+1), (x,y+2)

      Exemple :
        BABA IS YOU
        ROCK IS PUSH
        FLAG IS WIN

      Les propriétés détectées sont stockées dans table[subj].
*/
void rules_parse(const Grid& g, PropertyTable& table) {
    rules_reset(table);

    int w = g.width;
    int h = g.height;

    // Fonction locale pour traiter un triplet
    auto process = [&](ObjectType a, ObjectType b, ObjectType c){
        if (b != ObjectType::Text_Is) return;
        if (!is_subject_word(a)) return;

        ObjectType subj = subject_to_object(a);

        if (is_status_word(c))
            apply_status(table[(size_t)subj], c);
    };

    // -------------------------------------------------------------------------
    // Scan horizontal
    // -------------------------------------------------------------------------
    for (int y=0; y<h; y++)
        for (int x=0; x<w-2; x++) {
            auto& c0 = g.cell(x,y).objects;
            auto& c1 = g.cell(x+1,y).objects;
            auto& c2 = g.cell(x+2,y).objects;
            if (c0.empty() || c1.empty() || c2.empty()) continue;
            process(c0[0].type, c1[0].type, c2[0].type);
        }

    // -------------------------------------------------------------------------
    // Scan vertical
    // -------------------------------------------------------------------------
    for (int y=0; y<h-2; y++)
        for (int x=0; x<w; x++) {
            auto& c0 = g.cell(x,y).objects;
            auto& c1 = g.cell(x,y+1).objects;
            auto& c2 = g.cell(x,y+2).objects;
            if (c0.empty() || c1.empty() || c2.empty()) continue;
            process(c0[0].type, c1[0].type, c2[0].type);
        }
}

} // namespace baba
