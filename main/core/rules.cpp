/*
===============================================================================
  rules.cpp — Implémentation du moteur de règles (préparé pour AND)
-------------------------------------------------------------------------------
*/

#include "rules.h"
#include "types.h"
#include "grid.h"
#include <cstddef>
#include <vector>

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

static bool is_and_word(ObjectType t) {
    return t == ObjectType::Text_And;
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
        default:                     return ObjectType::Empty;
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

        if (is_word((ObjectType)i)) {
            props[i].push     = true;
            props[i].floating = true;
        }
    }
}

// ============================================================================
//  Ajout d’une transformation
// ============================================================================
static void add_transform(TransformSetTable& sets,
                          ObjectType subj, ObjectType target)
{
    if (subj == target)
        return;

    TransformSet& set = sets[(int)subj];

    if (set.count >= 3)
        return;

    for (int i = 0; i < set.count; i++)
        if (set.targets[i] == target)
            return;

    set.targets[set.count++] = target;
}

// ============================================================================
//  Parsing d’une séquence de mots
// ============================================================================
static void process_word_sequence(const std::vector<ObjectType>& seq,
                                  PropertyTable& props,
                                  TransformSetTable& sets)
{
    if (seq.size() < 3)
        return;

    for (size_t isPos = 1; isPos + 1 < seq.size(); ++isPos) {
        if (seq[isPos] != ObjectType::Text_Is)
            continue;

        // Sujets
        std::vector<ObjectType> subjects;
        {
            size_t i = 0;
            bool expectSubject = true;
            while (i < isPos) {
                ObjectType t = seq[i];
                if (expectSubject) {
                    if (!is_subject_word(t))
                        break;
                    subjects.push_back(t);
                    expectSubject = false;
                    ++i;
                } else {
                    if (!is_and_word(t))
                        break;
                    ++i;
                    if (i >= isPos || !is_subject_word(seq[i]))
                        break;
                    subjects.push_back(seq[i]);
                    ++i;
                }
            }
            if (subjects.empty())
                continue;
        }

        // Prédicats
        std::vector<ObjectType> predicates;
        {
            size_t i = isPos + 1;
            bool expectPred = true;
            while (i < seq.size()) {
                ObjectType t = seq[i];
                if (expectPred) {
                    if (!is_status_word(t) && !is_subject_word(t))
                        break;
                    predicates.push_back(t);
                    expectPred = false;
                    ++i;
                } else {
                    if (!is_and_word(t))
                        break;
                    ++i;
                    if (i >= seq.size())
                        break;
                    ObjectType t2 = seq[i];
                    if (!is_status_word(t2) && !is_subject_word(t2))
                        break;
                    predicates.push_back(t2);
                    ++i;
                }
            }
            if (predicates.empty())
                continue;
        }

        // Application
        for (auto subjWord : subjects) {
            ObjectType subjObj = subject_to_object(subjWord);

            for (auto predWord : predicates) {
                if (is_status_word(predWord)) {
                    apply_status(props[(int)subjObj], predWord);
                } else if (is_subject_word(predWord)) {
                    add_transform(sets, subjObj, subject_to_object(predWord));
                }
            }
        }
    }
}

// ============================================================================
//  Analyse des règles
// ============================================================================
void rules_parse(const Grid& g, PropertyTable& props, TransformSetTable& sets) {
    rules_reset(props, sets);

    int w = g.width;
    int h = g.height;

    // -------------------------
    // Scan horizontal
    // -------------------------
    for (int y = 0; y < h; ++y) {
        std::vector<ObjectType> seq;
        seq.reserve(w);

        for (int x = 0; x < w; ++x) {
            const auto& objs = g.cell(x, y).objects;

            ObjectType word = ObjectType::Empty;
            for (auto& o : objs) {
                if ((is_word(o.type) && o.type != ObjectType::Text_Empty) ||
                    o.type == ObjectType::Text_Is ||
                    is_and_word(o.type))
                {
                    word = o.type;
                    break;
                }
            }
            seq.push_back(word);
        }

        std::vector<ObjectType> segment;
        for (int x = 0; x < w; ++x) {
            ObjectType t = seq[x];
            if ((is_word(t) && t != ObjectType::Text_Empty) ||
                t == ObjectType::Text_Is ||
                is_and_word(t))
            {
                segment.push_back(t);
            } else {
                if (segment.size() >= 3)
                    process_word_sequence(segment, props, sets);
                segment.clear();
            }
        }
        if (segment.size() >= 3)
            process_word_sequence(segment, props, sets);
    }

    // -------------------------
    // Scan vertical
    // -------------------------
    for (int x = 0; x < w; ++x) {
        std::vector<ObjectType> seq;
        seq.reserve(h);

        for (int y = 0; y < h; ++y) {
            const auto& objs = g.cell(x, y).objects;

            ObjectType word = ObjectType::Empty;
            for (auto& o : objs) {
                if ((is_word(o.type) && o.type != ObjectType::Text_Empty) ||
                    o.type == ObjectType::Text_Is ||
                    is_and_word(o.type))
                {
                    word = o.type;
                    break;
                }
            }
            seq.push_back(word);
        }

        std::vector<ObjectType> segment;
        for (int y = 0; y < h; ++y) {
            ObjectType t = seq[y];
            if ((is_word(t) && t != ObjectType::Text_Empty) ||
                t == ObjectType::Text_Is ||
                is_and_word(t))
            {
                segment.push_back(t);
            } else {
                if (segment.size() >= 3)
                    process_word_sequence(segment, props, sets);
                segment.clear();
            }
        }
        if (segment.size() >= 3)
            process_word_sequence(segment, props, sets);
    }
}

// ============================================================================
//  Résolution des chaînes + cycles
// ============================================================================
static ObjectType resolve_chain(const TransformSetTable& sets, ObjectType start) {
    bool visited[(int)ObjectType::Count] = {false};
    ObjectType cur = start;

    while (true) {
        int idx = (int)cur;
        if (visited[idx])
            return start;

        visited[idx] = true;

        const TransformSet& s = sets[idx];
        if (s.count == 0)
            return cur;

        cur = s.targets[0];
    }
}

// ============================================================================
//  Application des transformations
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

                ObjectType resolved = resolve_chain(sets, t);
                obj.type = resolved;

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
    ObjectType emptyTarget = ObjectType::Empty;
    if (sets[(int)ObjectType::Empty].count > 0) {
        emptyTarget = resolve_chain(sets, ObjectType::Empty);
    }

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
