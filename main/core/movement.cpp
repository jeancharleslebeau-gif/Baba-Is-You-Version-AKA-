/*
===============================================================================
  movement.cpp — Moteur de déplacement consolidé (version avec SWAP + transfos)
-------------------------------------------------------------------------------
  Rôle :
    - Appliquer la logique de déplacement et d’interaction sur la grille :
        * MOVE (déplacement automatique)
        * YOU (déplacement joueur)
        * PUSH (chaînes d’objets)
        * PULL (tirer des objets)
        * STOP (blocage)
        * SWAP (échange de position)
        * FLOAT (couches séparées)
        * HOT/MELT
        * OPEN/SHUT
        * SINK/KILL
        * WIN
    - Appliquer les transformations d’objets AVANT les déplacements :
        * TransformSetTable permet :
            - transformations multiples (ROCK→WALL + ROCK→FLAG)
            - chaînes (ROCK→WALL→FLAG)
            - cycles gérés dans rules.cpp (A→B→C→A)

  Philosophie :
    - Les transformations sont appliquées en premier (apply_transformations).
    - MOVE est appliqué avant YOU.
    - PUSH est atomique (chaîne complète ou rien).
    - SWAP est traité avant STOP/PUSH sur la même couche FLOAT.
    - Les interactions (HOT/MELT, OPEN/SHUT, SINK/KILL, WIN) sont appliquées
      après tous les déplacements, séparément par couche FLOATING.
===============================================================================
*/

#include "movement.h"
#include "types.h"
#include "core/rules.h"   // apply_transformations()
#include <algorithm>
#include <vector>

namespace baba {

// ============================================================================
//  Helpers FLOATING
// ============================================================================

/*
    same_float_layer() :
      Compare deux ensembles de propriétés pour savoir s’ils sont sur la même
      "couche" FLOATING (sol / air).
*/
static inline bool same_float_layer(const Properties& a, const Properties& b) {
    return a.floating == b.floating;
}



// ============================================================================
//  try_push_chain() — Pousse une chaîne d’objets PUSH (FLOAT-aware)
// ============================================================================
/*
    try_push_chain() :
      Tente de pousser une chaîne d’objets PUSH à partir d’une case cible.

      - startX, startY : première case à pousser (devant le mover)
      - dx, dy         : direction du mouvement
      - moverIsFloat   : couche FLOATING du "mover" (YOU ou MOVE)

      Comportement :
        - On construit une chaîne de cases successives tant que :
            * tous les objets de la couche sont PUSH
            * aucun STOP non-pushable ne bloque
        - Si la chaîne est vide :
            * on autorise la superposition si aucune propriété STOP sur la
              même couche FLOATING.
        - Si la chaîne n’est pas vide :
            * on vérifie la case finale :
                - hors limites → échec
                - STOP non-pushable → échec
                - SINK → destruction des objets SINK de la couche, puis succès
            * on déplace la chaîne tail→head.
*/
static bool try_push_chain(Grid& grid, const PropertyTable& props,
                           int startX, int startY, int dx, int dy,
                           bool moverIsFloat)
{
    int cx = startX;
    int cy = startY;

    std::vector<std::pair<int,int>> chain;

    // 1) Construire la chaîne
    while (grid.in_bounds(cx, cy) && grid.in_play_area(cx, cy)) {

        Cell& c = grid.cell(cx, cy);
        if (c.objects.empty())
            break;

        bool allPush = true;

        for (auto& obj : c.objects) {
            const Properties& pr = props[(int)obj.type];

            // On ignore les objets d’une autre couche FLOATING
            if (pr.floating != moverIsFloat)
                continue;

            // STOP non-pushable bloque immédiatement
            if (pr.stop && !pr.push)
                return false;

            // Objet non-pushable → fin de chaîne
            if (!pr.push)
                allPush = false;
        }

        if (!allPush)
            break;

        chain.emplace_back(cx, cy);
        cx += dx;
        cy += dy;
    }

    // 2) Si chaîne vide → superposition si pas STOP sur la même couche
    if (chain.empty()) {
        Cell& target = grid.cell(startX, startY);
        for (auto& obj : target.objects) {
            const Properties& pr = props[(int)obj.type];
            if (pr.floating != moverIsFloat)
                continue;
            if (pr.stop)
                return false;
        }
        return true;
    }

    // 3) Vérifier la case finale
    if (!grid.in_bounds(cx, cy) || !grid.in_play_area(cx, cy))
        return false;

    Cell& finalCell = grid.cell(cx, cy);

    bool finalIsEmpty = finalCell.objects.empty();
    bool finalAllSink = true;

    if (!finalIsEmpty) {
        for (auto& obj : finalCell.objects) {
            const Properties& pr = props[(int)obj.type];

            if (pr.floating != moverIsFloat)
                continue;

            if (!pr.sink) {
                finalAllSink = false;
                break;
            }
        }
        if (!finalAllSink)
            return false;
    }

    // 4) SINK sur la case finale : on détruit les objets SINK de la couche
    if (!finalIsEmpty && finalAllSink) {
        finalCell.objects.erase(
            std::remove_if(finalCell.objects.begin(), finalCell.objects.end(),
                           [&](const Object& o){
                               const Properties& pr = props[(int)o.type];
                               return pr.sink && pr.floating == moverIsFloat;
                           }),
            finalCell.objects.end()
        );
    }

    // 5) Déplacer la chaîne (tail → head)
    for (int i = (int)chain.size() - 1; i >= 0; --i) {

        int fromX = chain[i].first;
        int fromY = chain[i].second;
        int toX   = fromX + dx;
        int toY   = fromY + dy;

        Cell& from = grid.cell(fromX, fromY);
        Cell& to   = grid.cell(toX, toY);

        std::vector<Object> moving;

        // Extraire les objets PUSH de la bonne couche
        for (auto& obj : from.objects) {
            const Properties& pr = props[(int)obj.type];
            if (pr.push && pr.floating == moverIsFloat)
                moving.push_back(obj);
        }

        // Supprimer du from
        from.objects.erase(
            std::remove_if(from.objects.begin(), from.objects.end(),
                           [&](const Object& o){
                               const Properties& pr = props[(int)o.type];
                               return pr.push && pr.floating == moverIsFloat;
                           }),
            from.objects.end()
        );

        // Ajouter au to
        for (auto& mo : moving)
            to.objects.push_back(mo);
    }

    return true;
}



// ============================================================================
//  SWAP helper — échange les objets "movers" avec les objets SWAP
// ============================================================================
/*
    apply_swap() :
      Gère la propriété SWAP pour un "mover" (YOU ou MOVE).

      - fromX, fromY : position du mover
      - toX, toY     : case cible
      - moverIsFloat : couche FLOATING du mover
      - isMover      : prédicat sur Properties (YOU ou MOVE)

      Comportement :
        - Si la case cible contient au moins un objet SWAP sur la même couche :
            * on extrait les movers de la case source
            * on extrait les SWAP de la case cible
            * on échange les deux ensembles
        - Sinon : ne fait rien, retourne false.
*/
template<typename IsMoverPredicate>
static bool apply_swap(Grid& grid, const PropertyTable& props,
                       int fromX, int fromY, int toX, int toY,
                       bool moverIsFloat,
                       IsMoverPredicate isMover)
{
    if (!grid.in_bounds(toX, toY) || !grid.in_play_area(toX, toY))
        return false;

    Cell& src = grid.cell(fromX, fromY);
    Cell& dst = grid.cell(toX, toY);

    bool hasSwap = false;

    for (auto& obj : dst.objects) {
        const Properties& pr = props[(int)obj.type];
        if (pr.floating == moverIsFloat && pr.swap) {
            hasSwap = true;
            break;
        }
    }

    if (!hasSwap)
        return false;

    std::vector<Object> movers;
    std::vector<Object> swappers;

    // Extraire les movers (YOU ou MOVE) de la bonne couche
    for (auto& obj : src.objects) {
        const Properties& pr = props[(int)obj.type];
        if (pr.floating == moverIsFloat && isMover(pr))
            movers.push_back(obj);
    }

    // Extraire les SWAP de la bonne couche
    for (auto& obj : dst.objects) {
        const Properties& pr = props[(int)obj.type];
        if (pr.floating == moverIsFloat && pr.swap)
            swappers.push_back(obj);
    }

    if (movers.empty() || swappers.empty())
        return false;

    // Supprimer movers de src
    src.objects.erase(
        std::remove_if(src.objects.begin(), src.objects.end(),
                       [&](const Object& o){
                           const Properties& pr = props[(int)o.type];
                           return pr.floating == moverIsFloat && isMover(pr);
                       }),
        src.objects.end()
    );

    // Supprimer swappers de dst
    dst.objects.erase(
        std::remove_if(dst.objects.begin(), dst.objects.end(),
                       [&](const Object& o){
                           const Properties& pr = props[(int)o.type];
                           return pr.floating == moverIsFloat && pr.swap;
                       }),
        dst.objects.end()
    );

    // Échanger
    for (auto& o : movers)
        dst.objects.push_back(o);
    for (auto& o : swappers)
        src.objects.push_back(o);

    return true;
}



// ============================================================================
//  MOVE automatique (avant YOU)
// ============================================================================
/*
    apply_move() :
      Applique la propriété MOVE dans une direction (dx, dy).

      - Snapshot initial des positions des objets MOVE (pour éviter les
        effets de "double move" dans la même frame).
      - Pour chaque mover :
          * SWAP (si possible)
          * STOP/PUSH sur la même couche
          * PUSH (chaîne)
          * déplacement du mover
*/
static void apply_move(Grid& grid, const PropertyTable& props, int dx, int dy)
{
    struct Mover { int x, y; bool floating; };
    std::vector<Mover> movers;

    // Snapshot des objets MOVE
    for (int y = 0; y < grid.height; ++y)
        for (int x = 0; x < grid.width; ++x)
            for (auto& obj : grid.cell(x, y).objects)
                if (props[(int)obj.type].move)
                    movers.push_back({x, y, props[(int)obj.type].floating});

    // Déplacement MOVE
    for (auto& m : movers) {

        int nx = m.x + dx;
        int ny = m.y + dy;

        if (!grid.in_bounds(nx, ny) || !grid.in_play_area(nx, ny))
            continue;

        // SWAP avant STOP/PUSH
        if (apply_swap(grid, props, m.x, m.y, nx, ny, m.floating,
                       [](const Properties& pr){ return pr.move; }))
        {
            continue;
        }

        // STOP non-pushable sur la même couche
        bool blocked = false;
        for (auto& obj : grid.cell(nx, ny).objects) {
            const Properties& pr = props[(int)obj.type];
            if (pr.floating != m.floating)
                continue;
            if (pr.stop && !pr.push)
                blocked = true;
        }
        if (blocked)
            continue;

        // PUSH
        if (!try_push_chain(grid, props, nx, ny, dx, dy, m.floating))
            continue;

        // Déplacer MOVE
        Cell& src = grid.cell(m.x, m.y);
        Cell& dst = grid.cell(nx, ny);

        for (auto it = src.objects.begin(); it != src.objects.end(); )
            if (props[(int)it->type].move &&
                props[(int)it->type].floating == m.floating)
            {
                dst.objects.push_back(*it);
                it = src.objects.erase(it);
            }
            else ++it;
    }
}



// ============================================================================
//  step() — Transformations + déplacement YOU + interactions
// ============================================================================
/*
    step() :
      Pipeline complet pour une frame de mouvement dans une direction (dx, dy) :

        1) apply_transformations(grid, transforms)
        2) MOVE automatique (si dx/dy != 0)
        3) YOU (déplacement joueur, PUSH, PULL, SWAP)
        4) Interactions post-mouvement par couche FLOATING :
             - WIN
             - KILL / SINK (détection de mort)
             - HOT/MELT (destruction des MELT)
             - OPEN/SHUT (destruction des OPEN/SHUT)
             - SINK (destruction des non-SINK si plusieurs objets)

      Retourne :
        - MoveResult.hasWon
        - MoveResult.hasDied
*/
MoveResult step(Grid& grid,
                const PropertyTable& props,
                const TransformSetTable& transforms,
                int dx, int dy)
{
    MoveResult result{};

    // 0) Appliquer les transformations avant tout mouvement
    apply_transformations(grid, transforms);

    // 1) MOVE automatique
    if (dx != 0 || dy != 0)
        apply_move(grid, props, dx, dy);

    // 2) Snapshot YOU
    struct YouPos { int x, y; bool floating; };
    std::vector<YouPos> yous;

    for (int y = 0; y < grid.height; ++y)
        for (int x = 0; x < grid.width; ++x)
            for (auto& obj : grid.cell(x, y).objects)
                if (props[(int)obj.type].you)
                    yous.push_back({x, y, props[(int)obj.type].floating});

    // 3) Déplacements YOU
    for (auto& yp : yous) {

        int nx = yp.x + dx;
        int ny = yp.y + dy;

        if (!grid.in_bounds(nx, ny) || !grid.in_play_area(nx, ny))
            continue;

        // SWAP avant STOP/PUSH
        if (apply_swap(grid, props, yp.x, yp.y, nx, ny, yp.floating,
                       [](const Properties& pr){ return pr.you; }))
        {
            // PULL après SWAP (on considère que YOU a "bougé")
            int backX = yp.x - dx;
            int backY = yp.y - dy;

            if (grid.in_bounds(backX, backY) && grid.in_play_area(backX, backY)) {
                Cell& back = grid.cell(backX, backY);
                Cell& dst  = grid.cell(nx, ny);
                for (auto it = back.objects.begin(); it != back.objects.end(); )
                    if (props[(int)it->type].pull &&
                        props[(int)it->type].floating == yp.floating)
                    {
                        dst.objects.push_back(*it);
                        it = back.objects.erase(it);
                    }
                    else ++it;
            }

            continue;
        }

        // STOP non-pushable (même couche)
        bool blocked = false;
        for (auto& obj : grid.cell(nx, ny).objects) {
            const Properties& pr = props[(int)obj.type];
            if (pr.floating != yp.floating)
                continue;
            if (pr.stop && !pr.push)
                blocked = true;
        }
        if (blocked)
            continue;

        // PUSH
        if (!try_push_chain(grid, props, nx, ny, dx, dy, yp.floating))
            continue;

        // Déplacer YOU
        Cell& src = grid.cell(yp.x, yp.y);
        Cell& dst = grid.cell(nx, ny);

        for (auto it = src.objects.begin(); it != src.objects.end(); )
            if (props[(int)it->type].you &&
                props[(int)it->type].floating == yp.floating)
            {
                dst.objects.push_back(*it);
                it = src.objects.erase(it);
            }
            else ++it;

        // PULL (même couche)
        int backX = yp.x - dx;
        int backY = yp.y - dy;

        if (grid.in_bounds(backX, backY) && grid.in_play_area(backX, backY)) {
            Cell& back = grid.cell(backX, backY);
            for (auto it = back.objects.begin(); it != back.objects.end(); )
                if (props[(int)it->type].pull &&
                    props[(int)it->type].floating == yp.floating)
                {
                    dst.objects.push_back(*it);
                    it = back.objects.erase(it);
                }
                else ++it;
        }
    }



    // ========================================================================
    // 4) Effets post-mouvement (séparés par couche FLOATING)
    // ========================================================================
    for (auto& cell : grid.cells) {

        // On traite d’abord NON-FLOATING (layer 0), puis FLOATING (layer 1)
        for (int layer = 0; layer < 2; ++layer) {

            bool layerIsFloat = (layer == 1);

            bool hasYou  = false;
            bool hasWin  = false;
            bool hasKill = false;
            bool hasSink = false;
            bool hasHot  = false;
            bool hasMelt = false;
            bool hasOpen = false;
            bool hasShut = false;

            // Scanner uniquement la couche courante
            for (auto& obj : cell.objects) {
                const Properties& pr = props[(int)obj.type];
                if (pr.floating != layerIsFloat)
                    continue;

                if (pr.you)      hasYou  = true;
                if (pr.win)      hasWin  = true;
                if (pr.kill)     hasKill = true;
                if (pr.sink)     hasSink = true;
                if (pr.hot)      hasHot  = true;
                if (pr.melt)     hasMelt = true;
                if (pr.open)     hasOpen = true;
                if (pr.shut)     hasShut = true;
            }

            // WIN : YOU + WIN sur la même couche
            if (hasYou && hasWin)
                result.hasWon = true;

            // KILL / SINK : YOU + KILL/SINK sur la même couche → mort
            if (hasYou && (hasKill || hasSink))
                result.hasDied = true;

            // HOT + MELT : on supprime les MELT de cette couche
            if (hasHot && hasMelt) {
                cell.objects.erase(
                    std::remove_if(cell.objects.begin(), cell.objects.end(),
                                   [&](const Object& o){
                                       const Properties& pr = props[(int)o.type];
                                       return pr.melt && pr.floating == layerIsFloat;
                                   }),
                    cell.objects.end()
                );
            }

            // OPEN + SHUT : on supprime les deux de cette couche
            if (hasOpen && hasShut) {
                cell.objects.erase(
                    std::remove_if(cell.objects.begin(), cell.objects.end(),
                                   [&](const Object& o){
                                       const Properties& pr = props[(int)o.type];
                                       return (pr.open || pr.shut) &&
                                              pr.floating == layerIsFloat;
                                   }),
                    cell.objects.end()
                );
            }

            // SINK : si plusieurs objets sur la même couche, on supprime
            // tous les objets non-SINK (optionnel, mais cohérent).
            if (hasSink) {
                int countLayer = 0;
                for (auto& obj : cell.objects) {
                    const Properties& pr = props[(int)obj.type];
                    if (pr.floating == layerIsFloat)
                        ++countLayer;
                }

                if (countLayer > 1) {
                    cell.objects.erase(
                        std::remove_if(cell.objects.begin(), cell.objects.end(),
                                       [&](const Object& o){
                                           const Properties& pr = props[(int)o.type];
                                           return pr.floating == layerIsFloat && !pr.sink;
                                       }),
                        cell.objects.end()
                    );
                }
            }
        }
    }

    return result;
}

} // namespace baba
