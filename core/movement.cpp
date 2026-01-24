/*
===============================================================================
  movement.cpp — Moteur de déplacement consolidé (version propre)
-------------------------------------------------------------------------------
  Gère :
    - YOU (déplacement joueur)
    - PUSH (chaînes)
    - STOP (blocage)
    - PULL (tirer)
    - MOVE (déplacement automatique)
    - FLOATING (couches séparées)
    - HOT/MELT
    - OPEN/SHUT
    - SINK/KILL
    - WIN

  Philosophie :
    - Toutes les interactions sont séparées par couche FLOATING.
    - MOVE est appliqué avant YOU.
    - PUSH est atomique.
    - Les effets post-mouvement sont appliqués après tous les déplacements.
===============================================================================
*/

#include "movement.h"
#include "types.h"
#include <algorithm>

namespace baba {

// ============================================================================
//  Helpers FLOATING
// ============================================================================
static inline bool same_float_layer(const Properties& a, const Properties& b) {
    return a.floating == b.floating;
}



// ============================================================================
//  try_push_chain() — Pousse une chaîne d’objets PUSH (FLOATING-aware)
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

        bool allPush = true;

        for (auto& obj : c.objects) {
            const Properties& pr = props[(int)obj.type];

            // On ignore les objets d’une autre couche FLOATING
            if (pr.floating != moverIsFloat)
                continue;

            // STOP non-pushable bloque
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

    // 4) SINK sur la case finale
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
//  MOVE automatique (avant YOU)
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
//  step() — Déplacement YOU + interactions
// ============================================================================
MoveResult step(Grid& grid, const PropertyTable& props, int dx, int dy)
{
    MoveResult result{};

    // 0) MOVE automatique
    if (dx != 0 || dy != 0)
        apply_move(grid, props, dx, dy);

    // 1) Snapshot YOU
    struct YouPos { int x, y; bool floating; };
    std::vector<YouPos> yous;

    for (int y = 0; y < grid.height; ++y)
        for (int x = 0; x < grid.width; ++x)
            for (auto& obj : grid.cell(x, y).objects)
                if (props[(int)obj.type].you)
                    yous.push_back({x, y, props[(int)obj.type].floating});

    // 2) Déplacements YOU
    for (auto& yp : yous) {

        int nx = yp.x + dx;
        int ny = yp.y + dy;

        if (!grid.in_bounds(nx, ny) || !grid.in_play_area(nx, ny))
            continue;

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
    // 3) Effets post-mouvement (séparés par couche FLOATING)
    // ========================================================================
    for (auto& cell : grid.cells) {

        // On traite d'abord NON-FLOATING (layer 0), puis FLOATING (layer 1)
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

            // WIN
            if (hasYou && hasWin)
                result.hasWon = true;

            // KILL / SINK
            if (hasYou && (hasKill || hasSink))
                result.hasDied = true;

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

    return result;
}

} // namespace baba