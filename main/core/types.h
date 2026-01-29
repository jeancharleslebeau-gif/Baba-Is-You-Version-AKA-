#pragma once
#include <array>
#include <cstdint>

namespace baba {

// ============================================================================
//  Types d'objets du jeu
// ============================================================================
enum class ObjectType {
    // objets physiques…
    Baba, Wall, Rock, Flag, Lava, Goop, Love, Empty, Key, Door, Water, Ice, Box,

    // mots
    Text_Baba,
    Text_Wall,
    Text_Rock,
    Text_Flag,
    Text_Lava,
    Text_Goop,
    Text_Love,
    Text_Empty,
    Text_Key,
    Text_Door,
    Text_Water,
    Text_Ice,
    Text_Box,
    Text_Is,
    Text_And, 

    Text_Push,
    Text_Stop,
    Text_Win,
    Text_You,
    Text_Sink,
    Text_Kill,
    Text_Swap,
    Text_Hot,
    Text_Melt,
    Text_Move,
    Text_Open,
    Text_Shut,
    Text_Float,
    Text_Pull,

    Count
};


// ============================================================================
//  Propriétés logiques appliquées aux objets
// ============================================================================
struct Properties {
    bool you      = false;   // Contrôlé par le joueur
    bool push     = false;   // Peut être poussé
    bool stop     = false;   // Bloque le mouvement
    bool win      = false;   // Condition de victoire
    bool defeat   = false;   // Alias possible de kill
    bool hot      = false;   // Détruit les objets MELT
    bool melt     = false;   // Fond au contact de HOT
    bool sink     = false;   // Détruit les deux objets
    bool open     = false;   // Ouvre SHUT
    bool shut     = false;   // S'ouvre avec OPEN
    bool move     = false;   // Se déplace automatiquement
    bool floating = false;   // Plan supérieur
    bool pull     = false;   // Tiré par YOU
    bool kill     = false;   // Détruit YOU
    bool swap     = false;   // Échange de position
};

// ============================================================================
//  Tables globales utilisées par le moteur
// ============================================================================

// Propriétés logiques : BABA IS YOU, ROCK IS PUSH, etc.
using PropertyTable = std::array<Properties, (int)ObjectType::Count>;

/*
===============================================================================
  Transformations multiples :
    - Chaque objet peut avoir jusqu’à 3 transformations simultanées.
    - Exemple :
        ROCK IS WALL
        ROCK IS FLAG
      → ROCK devient WALL + FLAG
===============================================================================
*/
struct TransformSet {
    uint8_t count = 0;
    ObjectType targets[3];
};

const char* object_type_to_text(ObjectType t);
ObjectType parse_object_type(const char* name);


using TransformSetTable = std::array<TransformSet, (int)ObjectType::Count>;

} // namespace baba
