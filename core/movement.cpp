/*
===============================================================================
  movement.cpp — Implémentation du moteur de déplacement (corrigé)
-------------------------------------------------------------------------------
  Rôle :
    - Déplacer tous les objets ayant la propriété YOU.
    - Gérer les chaînes de PUSH (ex : YOU → ROCK → ROCK → EMPTY).
    - Respecter STOP (bloque le mouvement).
    - Autoriser la superposition avec les objets non‑STOP (ex : FLAG).
    - Appliquer les effets post‑mouvement (WIN, KILL, SINK).
  Notes :
    - La résolution des pushes est atomique : inspection -> suppression (SINK)
      -> déplacement (tail -> head) -> déplacement de YOU -> recalcul règles.
===============================================================================
*/

#include "movement.h"
#include <algorithm>
#include <cstdio>   // pour debug temporaire si besoin

namespace baba {

// ============================================================================
//  Helper : tente de pousser une chaîne d’objets d’une case (atomique)
//  - startX/startY : première case contenant des objets (case directement devant YOU)
//  - dx/dy : direction du push
//  Retour : true si la chaîne a été poussée (ou la case finale vidée par SINK),
//           false si le push est impossible (STOP, bord, case finale non libre).
// ============================================================================
static bool try_push_chain(Grid& grid, const PropertyTable& props,
                           int startX, int startY, int dx, int dy)
{
    int cx = startX;
    int cy = startY;

    std::vector<std::pair<int,int>> chain;

    // 1) Construire la chaîne (inspection seule)
    while (grid.in_bounds(cx, cy) && grid.in_play_area(cx, cy)) {
        Cell& c = grid.cell(cx, cy);
        if (c.objects.empty()) break;

        bool allPush = true;
        for (auto& obj : c.objects) {
            const Properties& pr = props[(int)obj.type];
            // Si un objet STOP non pushable est présent -> blocage immédiat
            if (pr.isStop && !pr.isPush) return false;
            // Si un objet n'est pas pushable, on arrête la chaîne (on ne peut pas pousser)
            if (!pr.isPush) { allPush = false; break; }
        }

        if (!allPush) break;

        chain.emplace_back(cx, cy);
        cx += dx;
        cy += dy;
    }

    // Si la chaîne est vide, cela signifie que la case directement devant YOU
    // contient des objets non-pushables. Dans ce cas, autoriser le mouvement
    // **si et seulement si** aucun de ces objets n'a la propriété STOP.
    if (chain.empty()) {
        Cell& target = grid.cell(startX, startY);
        for (auto& obj : target.objects) {
            const Properties& pr = props[(int)obj.type];
            if (pr.isStop) return false; // case bloquée par STOP -> mouvement impossible
        }
        // Pas de STOP -> superposition autorisée (YOU peut entrer)
        return true;
    }

    // 2) Vérifier la case finale (cx,cy) pour la chaîne non vide
    if (!grid.in_bounds(cx, cy) || !grid.in_play_area(cx, cy)) return false;

    Cell& finalCell = grid.cell(cx, cy);

    // Si la case finale est vide -> ok
    bool finalIsEmpty = finalCell.objects.empty();

    // Si la case finale contient des objets, autoriser uniquement si tous sont SINK
    bool finalAllSink = true;
    if (!finalIsEmpty) {
        for (auto& obj : finalCell.objects) {
            const Properties& pr = props[(int)obj.type];
            if (!pr.isSink) { finalAllSink = false; break; }
        }
        if (!finalAllSink) return false; // case finale occupée par objet non-sink -> impossible
    }

    // 3) Appliquer atomiquement :
    if (!finalIsEmpty && finalAllSink) {
        finalCell.objects.erase(
            std::remove_if(finalCell.objects.begin(), finalCell.objects.end(),
                           [&](const Object& o){ return props[(int)o.type].isSink; }),
            finalCell.objects.end()
        );
    }

    // 4) Déplacer la chaîne (tail -> head)
    for (int i = (int)chain.size() - 1; i >= 0; --i) {
        int fromX = chain[i].first;
        int fromY = chain[i].second;
        int toX   = fromX + dx;
        int toY   = fromY + dy;

        Cell& from = grid.cell(fromX, fromY);
        Cell& to   = grid.cell(toX, toY);

        std::vector<Object> moving;
        for (auto& obj : from.objects) {
            if (props[(int)obj.type].isPush) moving.push_back(obj);
        }

        from.objects.erase(
            std::remove_if(from.objects.begin(), from.objects.end(),
                           [&](const Object& o){ return props[(int)o.type].isPush; }),
            from.objects.end()
        );

        for (auto& mo : moving) to.objects.push_back(mo);
    }

    return true;
}


// ============================================================================
//  step() — Applique un déplacement dx/dy à tous les objets YOU
//  - Utilise try_push_chain pour garantir atomicité et gestion correcte de SINK
// ============================================================================
MoveResult step(Grid& grid, const PropertyTable& props, int dx, int dy)
{
    MoveResult result;

    // 1) Snapshot des positions YOU au début
    struct YouPos { int x, y; };
    std::vector<YouPos> yous;
    yous.reserve(grid.width * grid.height);

    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            const Cell& c = grid.cell(x, y);
            for (const auto& obj : c.objects) {
                if (props[(int)obj.type].isYou) {
                    yous.push_back({x, y});
                }
            }
        }
    }

    // 2) Pour chaque YOU, tenter de pousser la chaîne devant lui
    for (const auto& yp : yous) {
        int nx = yp.x + dx;
        int ny = yp.y + dy;

        // Bloquer hors grille / hors zone jouable
        if (!grid.in_bounds(nx, ny)) continue;
        if (!grid.in_play_area(nx, ny)) continue;

        // Vérifier STOP dans la case cible (si un objet STOP non-push y est, on bloque)
        bool blocked = false;
        for (auto& obj : grid.cell(nx, ny).objects) {
            if (props[(int)obj.type].isStop && !props[(int)obj.type].isPush) { blocked = true; break; }
        }
        if (blocked) continue;

        // Essayer de pousser la chaîne devant (si PUSH)
        // IMPORTANT : try_push_chain effectue l'inspection et applique les suppressions SINK
        bool pushed = try_push_chain(grid, props, nx, ny, dx, dy);
        if (!pushed) {
            // push impossible -> ne pas déplacer ce YOU
            continue;
        }

        // 3) Déplacer YOU d’une case (superposition autorisée)
        Cell& src = grid.cell(yp.x, yp.y);
        Cell& dst = grid.cell(nx, ny);

        // Déplacer toutes les entités YOU présentes dans la cellule source
        for (auto it = src.objects.begin(); it != src.objects.end(); ) {
            if (props[(int)it->type].isYou) {
                dst.objects.push_back(*it);
                it = src.objects.erase(it);
            } else {
                ++it;
            }
        }
    }

    // 4) Effets post-mouvement par superposition (WIN, KILL, SINK)
    for (auto& cell : grid.cells) {
        bool hasYou = false, hasWin = false, hasKill = false, hasSink = false;
        for (auto& obj : cell.objects) {
            const Properties& pr = props[(int)obj.type];
            if (pr.isYou)  hasYou = true;
            if (pr.isWin)  hasWin = true;
            if (pr.isKill) hasKill = true;
            if (pr.isSink) hasSink = true;
        }
        if (hasYou && hasWin) result.hasWon = true;
        if (hasYou && (hasKill || hasSink)) result.hasDied = true;
    }

    return result;
}

} // namespace baba
