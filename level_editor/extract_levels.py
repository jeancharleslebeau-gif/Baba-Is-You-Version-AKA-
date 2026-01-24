import os
import re

# Chemin vers ton fichier C++ contenant tous les niveaux
INPUT_FILE = r"C:\Users\jean_\OneDrive\Documents 1\Aka\babaIsU\main\game\levels_data.cpp"

# Dossier de sortie pour les fichiers texte exportés
OUTPUT_DIR = "export"

# ---------------------------------------------------------------------------
# TABLE DE MAPPING : tokens C++ → noms d’objets de l’éditeur
#
# À gauche : les identifiants utilisés dans levels_data.cpp (defines.h)
# À droite : les noms utilisés dans ton éditeur HTML / ObjectType
# ---------------------------------------------------------------------------
TOKEN_MAP = {
    # Objets physiques
    "EMPTY": "EMPTY",
    "BABA":  "BABA",
    "WALL":  "WALL",
    "ROCK":  "ROCK",
    "FLAG":  "FLAG",
    "LAVA":  "LAVA",
    "GOOP":  "GOOP",
    "LOVE":  "LOVE",
    "KEY":   "KEY",
    "DOOR":  "DOOR",
    "WATER": "WATER",
    "ICE":   "ICE",
    "BOX":   "BOX",

    # Mots (noms)
    "W_BABA":  "TEXT_BABA",
    "W_WALL":  "TEXT_WALL",
    "W_ROCK":  "TEXT_ROCK",
    "W_FLAG":  "TEXT_FLAG",
    "W_LAVA":  "TEXT_LAVA",
    "W_GOOP":  "TEXT_GOOP",
    "W_LOVE":  "TEXT_LOVE",
    "W_EMPTY": "TEXT_EMPTY",
    "W_KEY":   "TEXT_KEY",
    "W_DOOR":  "TEXT_DOOR",
    "W_WATER": "TEXT_WATER",
    "W_ICE":   "TEXT_ICE",
    "W_BOX":   "TEXT_BOX",

    # Mots (propriétés / verbes)
    "W_IS":    "TEXT_IS",
    "W_PUSH":  "TEXT_PUSH",
    "W_STOP":  "TEXT_STOP",
    "W_WIN":   "TEXT_WIN",
    "W_YOU":   "TEXT_YOU",
    "W_SINK":  "TEXT_SINK",
    "W_KILL":  "TEXT_KILL",
    "W_SWAP":  "TEXT_SWAP",
    "W_HOT":   "TEXT_HOT",
    "W_MELT":  "TEXT_MELT",
    "W_MOVE":  "TEXT_MOVE",
    "W_OPEN":  "TEXT_OPEN",
    "W_SHUT":  "TEXT_SHUT",
    "W_FLOAT": "TEXT_FLOAT",
    "W_PULL":  "TEXT_PULL",
}

# ---------------------------------------------------------------------------
# Fonction principale : extraction et export de tous les niveaux
# ---------------------------------------------------------------------------
def extract_levels():
    # Lecture complète du fichier C++ contenant les niveaux
    with open(INPUT_FILE, "r", encoding="utf-8") as f:
        content = f.read()

    # -----------------------------------------------------------------------
    # Regex pour trouver tous les tableaux de la forme :
    #
    #   const uint8_t levelX[...] = {
    #       ... données ...
    #   };
    #
    # Groupe 1 : nom du niveau (level1, level2, level22, etc.)
    # Groupe 2 : contenu brut entre { ... }
    # -----------------------------------------------------------------------
    pattern = re.compile(
        r"""
        const\s+uint8_t\s+(level\d+)\s*      # nom du niveau : level1, level22...
        

\[[^\]

]*\]

\s*                        # taille du tableau : [META_FULL_SIZE] ou autre
        =\s*\{(.*?)\};                       # contenu entre accolades { ... };
        """,
        re.DOTALL | re.VERBOSE
    )

    # Recherche de tous les niveaux dans le fichier
    matches = pattern.findall(content)

    if not matches:
        print("Aucun niveau trouvé dans levels_data.cpp.")
        return

    # Création du dossier de sortie si nécessaire
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # -----------------------------------------------------------------------
    # Pour chaque niveau trouvé, on génère un fichier texte
    # -----------------------------------------------------------------------
    for level_name, data_block in matches:
        print(f"Extraction de {level_name}...")

        # -------------------------------------------------------------------
        # 1) Nettoyage des tokens
        #
        # On remplace les retours à la ligne par des espaces, puis on split.
        # Exemple : "EMPTY, BABA, W_FLAG," → ["EMPTY,", "BABA,", "W_FLAG,"]
        # -------------------------------------------------------------------
        raw_tokens = data_block.replace("\n", " ").split()

        # On enlève les virgules finales et les espaces superflus
        tokens = [
            tok.strip().rstrip(',')
            for tok in raw_tokens
            if tok.strip().rstrip(',') != ""
        ]

        # -------------------------------------------------------------------
        # 2) Conversion C++ → noms d’objets de l’éditeur
        #
        # Si un token n’est pas dans TOKEN_MAP, on le remplace par EMPTY
        # (sécurité pour éviter de casser l’export).
        # -------------------------------------------------------------------
        converted = [TOKEN_MAP.get(tok, "EMPTY") for tok in tokens]

        # -------------------------------------------------------------------
        # 3) Détermination automatique de la largeur du niveau
        #
        # On compte le nombre d’éléments par ligne dans le bloc C++ d’origine.
        # Ça permet de gérer des niveaux de tailles différentes (12x10, 13x10,
        # 20x15, etc.) sans hardcoder les dimensions.
        # -------------------------------------------------------------------
        lines = data_block.strip().split("\n")

        # Pour chaque ligne, on compte les éléments non vides séparés par des virgules
        widths = [len([t for t in line.split(",") if t.strip()]) for line in lines]

        # Largeur = max des largeurs trouvées (par sécurité)
        width = max(widths)
        height = len(lines)

        # -------------------------------------------------------------------
        # 4) Génération du texte final
        #
        # On reconstruit la grille ligne par ligne :
        #   OBJ1, OBJ2, OBJ3, ...
        # -------------------------------------------------------------------
        out_lines = []
        idx = 0
        for y in range(height):
            row = converted[idx:idx + width]
            idx += width
            out_lines.append(", ".join(row))

        # -------------------------------------------------------------------
        # 5) Sauvegarde dans un fichier texte
        #
        # Exemple : export/level1.txt, export/level22.txt, etc.
        # -------------------------------------------------------------------
        out_path = os.path.join(OUTPUT_DIR, f"{level_name}.txt")
        with open(out_path, "w", encoding="utf-8") as f:
            f.write("\n".join(out_lines))

        print(f"  ✔ Sauvé dans {out_path}")

    print("\n🎉 Export terminé ! Tous les niveaux sont dans le dossier 'export'.")

# ---------------------------------------------------------------------------
# Point d’entrée du script
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    extract_levels()
