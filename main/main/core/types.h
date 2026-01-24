#pragma once
#include <array>
#include <cstdint>

namespace baba {

enum class ObjectType : uint8_t {
    Empty = 0,

    // Objets physiques
    Baba,
    Wall,
    Rock,
    Flag,
    Lava,
    Goop,
    Love,

    // Nouveaux objets simples
    Key,
    Door,
    Water,
    Ice,
    Box,

    // Mots (noms)
    Text_Baba,
    Text_Wall,
    Text_Rock,
    Text_Flag,
    Text_Lava,
    Text_Goop,
    Text_Love,
    Text_Empty,

    // Nouveaux mots (noms)
    Text_Key,
    Text_Door,
    Text_Water,
    Text_Ice,
    Text_Box,

    // Mots (verbes / propriétés)
    Text_Is,
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

    // Nouvelle propriété simple
    Text_Pull,

    Count
};

struct Properties {
    bool you      = false;
    bool push     = false;
    bool stop     = false;
    bool win      = false;
    bool defeat   = false;
    bool hot      = false;
    bool melt     = false;
    bool sink     = false;
    bool open     = false;
    bool shut     = false;
    bool move     = false;
    bool floating = false;  
    bool pull     = false;
    bool kill     = false;
};

using PropertyTable = std::array<Properties, (int)ObjectType::Count>;

} // namespace baba
