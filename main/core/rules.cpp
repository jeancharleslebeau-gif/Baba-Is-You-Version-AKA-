/*
===============================================================================
  rules.cpp — Implémentation du moteur de règles
-------------------------------------------------------------------------------
  Rôle :
    - Scanner la grille pour détecter les triplets :
          SUBJECT — IS — STATUS
          SUBJECT — IS — SUBJECT   (transformations)
    - Remplir :
          * PropertyTable        (propriétés logiques : YOU, PUSH, STOP…)
          * TransformSetTable    (transformations multiples : ROCK→WALL+FLAG)
    - Gérer les règles horizontales et verticales.

  Notes :
    - Les règles composées (BABA IS YOU AND WIN) ne sont pas encore gérées.
    - Les propriétés avancées (HOT/MELT, OPEN/SHUT, MOVE, FLOATING, PULL…)
      sont appliquées dans movement.cpp.
    - Les transformations sont appliquées via apply_transformations() :
         * transformations multiples
         * transformations en chaîne
         * détection de cycles
         * EMPTY IS X

  Auteur : Jean-Charles LEBEAU
  Date   : Janvier 2026
===============================================================================
*/

#include "rules.h"
#include "types.h"
#include "grid.h"
#include <cstddef>

namespace baba {

// ============================================================================
//  Classification des mots
// ============================================================================
bool is_word(ObjectType t) {
    return t >= ObjectType::Text_Baba;
}

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
        case ObjectType::Text_Key:
        case ObjectType::Text_Door:
        case ObjectType::Text_Water:
        case ObjectType::Text_Ice:
        case ObjectType::Text_Box:
            return true;
        default:
            return false;
    }
}

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
        case ObjectType::Text_Pull:
            return true;
        default:
            return false;
    }
}

// ============================================================================
//  Conversion TEXT_* → objet physique
// ============================================================================
ObjectType subject_to_object(ObjectType word) {
    switch (word) {
        case ObjectType::Text_Baba:  return ObjectType::Baba;
        case ObjectType::Text_Wall:  return ObjectType::Wall;
        case ObjectType::Text_Rock:  return ObjectType::Rock;
        case ObjectType::Text_Flag:  return ObjectType::Flag;
        case ObjectType::Text_Lava:  return ObjectType::Lava;
        case ObjectType::Text_Goop:  return ObjectType::Goop;
        case ObjectType::Text_Love:  return ObjectType::Love;
        case ObjectType::Text_Empty: return ObjectType::Empty;
        case ObjectType::Text_Key:   return ObjectType::Key;
        case ObjectType::Text_Door:  return ObjectType::Door;
        case ObjectType::Text_Water: return ObjectType::Water;
        case ObjectType::Text_Ice:   return ObjectType::Ice;
        case ObjectType::Text_Box:   return ObjectType::Box;
        default: return ObjectType::Empty;
    }
}

// ============================================================================
//  Applique une propriété logique
// ============================================================================
void apply_status(Properties& p, ObjectType s) {
    switch (s) {
        case ObjectType::Text_You:   p.you      = true; break;
        case ObjectType::Text_Push:  p.push     = true; break;
        case ObjectType::Text_Stop:  p.stop     = true; break;
        case ObjectType::Text_Win:   p.win      = true; break;
        case ObjectType::Text_Sink:  p.sink     = true; break;
        case ObjectType::Text_Kill:  p.kill     = true; break;
        case ObjectType::Text_Hot:   p.hot      = true; break;
        case ObjectType::Text_Melt:  p.melt     = true; break;
        case ObjectType::Text_Move:  p.move     = true; break;
        case ObjectType::Text_Open:  p.open     = true; break;
        case ObjectType::Text_Shut:  p.shut     = true; break;
        case ObjectType::Text_Float: p.floating = true; break;
        case ObjectType::Text_Pull:  p.pull     = true; break;
        case ObjectType::Text_Swap:  p.swap     = true; break;
        default: break;
    }
}

// ============================================================================
//  Réinitialisation des propriétés + transformations multiples
// ============================================================================
void rules_reset(PropertyTable& props, TransformSetTable& sets) {
    for (int i = 0; i < (int)ObjectType::Count; i++) {
        props[i] = Properties{};
        sets[i].count = 0;

        if (is_word((ObjectType)i))
            props[i].push = true;
    }
}

// ============================================================================
//  Analyse des règles (propriétés + transformations multiples)
// ============================================================================
void rules_parse(const Grid& g, PropertyTable& props, TransformSetTable& sets) {
    rules_reset(props, sets);

    int w = g.width;
    int h = g.height;

    auto process = [&](ObjectType a, ObjectType b, ObjectType c){
        if (b != ObjectType::Text_Is) return;
        if (!is_subject_word(a)) return;

        ObjectType subj = subject_to_object(a);

        // SUBJECT IS STATUS
        if (is_status_word(c)) {
            apply_status(props[(int)subj], c);
            return;
        }

        // SUBJECT IS SUBJECT (transformation multiple)
        if (is_subject_word(c)) {
            ObjectType target = subject_to_object(c);
            if (target == subj) return;

            TransformSet& set = sets[(int)subj];

            if (set.count < 3) {
                bool exists = false;
                for (int i = 0; i < set.count; i++)
                    if (set.targets[i] == target)
                        exists = true;

                if (!exists)
                    set.targets[set.count++] = target;
            }
        }
    };

    // Scan horizontal
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w - 2; x++) {
            auto& c0 = g.cell(x,   y).objects;
            auto& c1 = g.cell(x+1, y).objects;
            auto& c2 = g.cell(x+2, y).objects;
            if (!c0.empty() && !c1.empty() && !c2.empty())
                process(c0[0].type, c1[0].type, c2[0].type);
        }

    // Scan vertical
    for (int y = 0; y < h - 2; y++)
        for (int x = 0; x < w; x++) {
            auto& c0 = g.cell(x,   y).objects;
            auto& c1 = g.cell(x, y+1).objects;
            auto& c2 = g.cell(x, y+2).objects;
            if (!c0.empty() && !c1.empty() && !c2.empty())
                process(c0[0].type, c1[0].type, c2[0].type);
        }
}

// ============================================================================
//  Résolution des chaînes + détection de cycles
// ============================================================================
static ObjectType resolve_chain(const TransformSetTable& sets, ObjectType start) {
    bool visited[(int)ObjectType::Count] = {false};
    ObjectType cur = start;

    while (true) {
        int idx = (int)cur;
        if (visited[idx])
            return start; // cycle → on garde l’original

        visited[idx] = true;

        const TransformSet& s = sets[idx];
        if (s.count == 0)
            return cur;

        cur = s.targets[0]; // première cible = cible principale
    }
}

// ============================================================================
//  Application des transformations multiples + EMPTY IS X
// ============================================================================
void apply_transformations(Grid& g, const TransformSetTable& sets) {
    int w = g.width;
    int h = g.height;

    // Transformations normales + multiples
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {

            auto& cell = g.cell(x, y).objects;
            std::vector<Object> extra;

            for (auto& obj : cell) {
                ObjectType t = obj.type;
                const TransformSet& s = sets[(int)t];

                if (s.count == 0)
                    continue;

                // Résolution de chaîne + cycle
                ObjectType resolved = resolve_chain(sets, t);
                obj.type = resolved;

                // Transformations multiples → duplication
                for (int i = 1; i < s.count; i++) {
                    Object copy = obj;
                    copy.type = s.targets[i];
                    extra.push_back(copy);
                }
            }

            for (auto& o : extra)
                cell.push_back(o);
        }

    // EMPTY IS X
    ObjectType emptyTarget =
        (sets[(int)ObjectType::Empty].count > 0)
            ? sets[(int)ObjectType::Empty].targets[0]
            : ObjectType::Empty;

    if (emptyTarget != ObjectType::Empty) {
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                if (g.cell(x, y).objects.empty()) {
                    Object o;
                    o.type = emptyTarget;
                    g.cell(x, y).objects.push_back(o);
                }
    }
}

} // namespace baba
