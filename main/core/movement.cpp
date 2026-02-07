/*
===============================================================================
  movement.cpp — Moteur de déplacement consolidé (version blindée SWAP + transfos)
-------------------------------------------------------------------------------
  Rôle :
    - Appliquer la logique de déplacement et d’interaction sur la grille :
        * MOVE (déplacement automatique)
        * YOU (déplacement joueur)
        * PUSH (chaînes d’objets)
        * PULL (tirer des objets)
        * STOP (blocage)
        * SWAP (échange de position)
        * FLOAT (couches séparées pour interactions, PAS pour collisions)
        * HOT/MELT
        * OPEN/SHUT
        * SINK/KILL
        * WIN
    - Appliquer les transformations d’objets AVANT les déplacements.
===============================================================================
*/

#include "movement.h"
#include "types.h"
#include "core/rules.h"
#include <algorithm>
#include <vector>

namespace baba {

// ============================================================================
//  Helper FLOATING
//  (FLOAT ne sépare PAS les collisions dans Baba Is You)
// ============================================================================
static inline bool same_float_layer(const Properties& a, const Properties& b) {
    return a.floating == b.floating;
}

// ============================================================================
//  try_push_chain() — Pousse une chaîne d’objets PUSH
//  IMPORTANT (comportement canon Baba Is You) :
//    - FLOAT n’empêche PAS PUSH
//    - STOP bloque même si l’objet est PUSH
//    - Un objet ni PUSH ni STOP ne bloque pas, mais ne fait pas partie de la chaîne
//    - Une case fait partie de la chaîne s’il y a AU MOINS un PUSH et aucun STOP
// ============================================================================
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

        bool hasPush = false;

        for (auto& obj : c.objects) {
            const Properties& pr = props[(int)obj.type];

            // FLOAT ignoré pour les collisions

            // STOP bloque immédiatement, même si PUSH
            if (pr.stop)
                return false;

            if (pr.push)
                hasPush = true;
        }

        // S’il n’y a aucun PUSH sur cette case, la chaîne s’arrête ici
        if (!hasPush)
            break;

        chain.emplace_back(cx, cy);
        cx += dx;
        cy += dy;
    }

    // 2) Si chaîne vide → vérifier STOP sur la case de départ
    if (chain.empty()) {
        Cell& target = grid.cell(startX, startY);
        for (auto& obj : target.objects) {
            const Properties& pr = props[(int)obj.type];
            // FLOAT ignoré pour les collisions
            if (pr.stop)
                return false;
        }
        // Pas de STOP → le mover peut entrer, mais rien n’est poussé
        return true;
    }

    // 3) Vérifier la case finale (hors grille → bloqué)
    if (!grid.in_bounds(cx, cy) || !grid.in_play_area(cx, cy))
        return false;

    // 4) Déplacer la chaîne (tail → head)
    for (int i = (int)chain.size() - 1; i >= 0; --i) {

        int fromX = chain[i].first;
        int fromY = chain[i].second;
        int toX   = fromX + dx;
        int toY   = fromY + dy;

        Cell& from = grid.cell(fromX, fromY);
        Cell& to   = grid.cell(toX, toY);

        std::vector<Object> moving;

        // Extraire les objets PUSH (FLOAT ignoré)
        for (auto& obj : from.objects) {
            const Properties& pr = props[(int)obj.type];
            if (pr.push)
                moving.push_back(obj);
        }

        // Supprimer du from
        from.objects.erase(
            std::remove_if(from.objects.begin(), from.objects.end(),
                           [&](const Object& o){
                               const Properties& pr = props[(int)o.type];
                               return pr.push;
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
//  SWAP — FLOAT n’empêche PAS SWAP dans Baba Is You
// ============================================================================
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
        // FLOAT ignoré pour SWAP
        if (pr.swap) {
            hasSwap = true;
            break;
        }
    }

    if (!hasSwap)
        return false;

    std::vector<Object> movers;
    std::vector<Object> swappers;

    // Extraire movers (YOU ou MOVE)
    for (auto& obj : src.objects) {
        const Properties& pr = props[(int)obj.type];
        if (isMover(pr))
            movers.push_back(obj);
    }

    // Extraire swappers
    for (auto& obj : dst.objects) {
        const Properties& pr = props[(int)obj.type];
        if (pr.swap)
            swappers.push_back(obj);
    }

    if (movers.empty() || swappers.empty())
        return false;

    // Supprimer movers de src
    src.objects.erase(
        std::remove_if(src.objects.begin(), src.objects.end(),
                       [&](const Object& o){
                           const Properties& pr = props[(int)o.type];
                           return isMover(pr);
                       }),
        src.objects.end()
    );

    // Supprimer swappers de dst
    dst.objects.erase(
        std::remove_if(dst.objects.begin(), dst.objects.end(),
                       [&](const Object& o){
                           const Properties& pr = props[(int)o.type];
                           return pr.swap;
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
//  MOVE automatique — FLOAT n’empêche PAS MOVE/PUSH/STOP
// ============================================================================
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

        // STOP bloque tout, même PUSH (FLOAT ignoré)
        bool blocked = false;
        for (auto& obj : grid.cell(nx, ny).objects) {
            const Properties& pr = props[(int)obj.type];
            if (pr.stop)
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
            if (props[(int)it->type].move)
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
MoveResult step(Grid& grid,
                const PropertyTable& props,
                const TransformSetTable& transforms,
                int dx, int dy)
{
    MoveResult result{};

    const int w = grid.width;
    const int h = grid.height;
    const int cellCount = w * h;

    auto cell_index = [w](int x, int y) {
        return y * w + x;
    };

    // ------------------------------------------------------------------------
    // 0) Snapshot avant mouvement (pour détecter les collisions nouvelles)
    // ------------------------------------------------------------------------
    struct LayerSnapshot {
        bool any[2]  = {false, false};
        bool you[2]  = {false, false};
        bool kill[2] = {false, false};
        bool sink[2] = {false, false};
    };

    std::vector<LayerSnapshot> before(cellCount);

    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            int idx = cell_index(x, y);
            Cell& c = grid.cell(x, y);

            for (auto& obj : c.objects) {
                const Properties& pr = props[(int)obj.type];
                int layer = pr.floating ? 1 : 0;

                before[idx].any[layer]  = true;
                if (pr.you)  before[idx].you[layer]  = true;
                if (pr.kill) before[idx].kill[layer] = true;
                if (pr.sink) before[idx].sink[layer] = true;
            }
        }

    // ------------------------------------------------------------------------
    // 1) MOVE automatique
    // ------------------------------------------------------------------------
    if (dx != 0 || dy != 0)
        apply_move(grid, props, dx, dy);

    // ------------------------------------------------------------------------
    // 2) Snapshot des YOU après MOVE
    // ------------------------------------------------------------------------
    struct YouPos { int x, y; bool floating; };
    std::vector<YouPos> yous;

    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            for (auto& obj : grid.cell(x, y).objects)
                if (props[(int)obj.type].you)
                    yous.push_back({x, y, props[(int)obj.type].floating});

    // ------------------------------------------------------------------------
    // 3) Déplacements YOU (PUSH, PULL, SWAP)
    // ------------------------------------------------------------------------
    for (auto& yp : yous) {

        int nx = yp.x + dx;
        int ny = yp.y + dy;

        if (!grid.in_bounds(nx, ny) || !grid.in_play_area(nx, ny))
            continue;

        // SWAP avant STOP/PUSH
        if (apply_swap(grid, props, yp.x, yp.y, nx, ny, yp.floating,
                       [](const Properties& pr){ return pr.you; }))
        {
            // PULL après SWAP
            int backX = nx - dx;
            int backY = ny - dy;

            if (grid.in_bounds(backX, backY) && grid.in_play_area(backX, backY)) {
                Cell& back = grid.cell(backX, backY);
                Cell& dst  = grid.cell(nx, ny);

                for (auto it = back.objects.begin(); it != back.objects.end(); )
                    if (props[(int)it->type].pull) {
                        dst.objects.push_back(*it);
                        it = back.objects.erase(it);
                    } else ++it;
            }

            continue;
        }

        // STOP bloque tout
        bool blocked = false;
        for (auto& obj : grid.cell(nx, ny).objects)
            if (props[(int)obj.type].stop)
                blocked = true;

        if (blocked)
            continue;

        // PUSH
        if (!try_push_chain(grid, props, nx, ny, dx, dy, yp.floating))
            continue;

        // Déplacer YOU
        Cell& src = grid.cell(yp.x, yp.y);
        Cell& dst = grid.cell(nx, ny);

        for (auto it = src.objects.begin(); it != src.objects.end(); )
            if (props[(int)it->type].you) {
                dst.objects.push_back(*it);
                it = src.objects.erase(it);
            } else ++it;

        // PULL
        int backX = yp.x - dx;
        int backY = yp.y - dy;

        if (grid.in_bounds(backX, backY) && grid.in_play_area(backX, backY)) {
            Cell& back = grid.cell(backX, backY);
            for (auto it = back.objects.begin(); it != back.objects.end(); )
                if (props[(int)it->type].pull) {
                    dst.objects.push_back(*it);
                    it = back.objects.erase(it);
                } else ++it;
        }
    }

    // ------------------------------------------------------------------------
    // 4) Interactions post-mouvement (par couche FLOAT)
    // ------------------------------------------------------------------------
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {

            int idx = cell_index(x, y);
            Cell& cell = grid.cell(x, y);

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

                int countLayer = 0;

                for (auto& obj : cell.objects) {
                    const Properties& pr = props[(int)obj.type];
                    if (pr.floating != layerIsFloat)
                        continue;

                    ++countLayer;

                    if (pr.you)  hasYou  = true;
                    if (pr.win)  hasWin  = true;
                    if (pr.kill) hasKill = true;
                    if (pr.sink) hasSink = true;
                    if (pr.hot)  hasHot  = true;
                    if (pr.melt) hasMelt = true;
                    if (pr.open) hasOpen = true;
                    if (pr.shut) hasShut = true;
                }

                // WIN
                if (hasYou && hasWin)
                    result.hasWon = true;

                // YOU + KILL : détruire uniquement les YOU
                if (hasYou && hasKill) {
                    cell.objects.erase(
                        std::remove_if(cell.objects.begin(), cell.objects.end(),
                                       [&](const Object& o){
                                           const Properties& pr = props[(int)o.type];
                                           return pr.you && pr.floating == layerIsFloat;
                                       }),
                        cell.objects.end()
                    );
                }

                // SINK : destruction mutuelle (objet entrant + SINK)
                if (hasSink && countLayer > 1) {
                    cell.objects.erase(
                        std::remove_if(cell.objects.begin(), cell.objects.end(),
                                       [&](const Object& o){
                                           const Properties& pr = props[(int)o.type];
                                           return pr.floating == layerIsFloat;
                                       }),
                        cell.objects.end()
                    );
                }

                // HOT + MELT
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

                // OPEN + SHUT
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
            }
        }

    // ------------------------------------------------------------------------
    // 5) Vérifier s'il reste au moins un YOU (multi-YOU canonique)
    // ------------------------------------------------------------------------
    bool anyYouLeft = false;

    for (auto& c : grid.cells)
        for (auto& o : c.objects)
            if (props[(int)o.type].you)
                anyYouLeft = true;

    if (!anyYouLeft)
        result.hasDied = true;

    return result;
}


} // namespace baba
