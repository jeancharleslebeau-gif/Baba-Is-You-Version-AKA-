# ============================================================
#  Sélection du backend graphique (optionnel)
# ============================================================

idf_component_register(
    SRCS

        # --- Point d’entrée ---
        app_main.cpp

        # --- Core BabaIsU ---
        core/audio.cpp
        core/input.cpp
        core/grid.cpp
        core/graphics.cpp
        core/rules.cpp
        core/movement.cpp
        core/sprites.cpp
        core/persist.cpp
        core/filesystem.cpp    
	core/types.cpp 

        # --- Logique de jeu ---
        game/game.cpp
        game/levels_data.cpp
        game/levels.cpp
	game/config.cpp
        game/options.cpp
        game/menu.cpp          

        # --- Tâches FreeRTOS ---
        tasks/task_game.cpp
        tasks/task_input.cpp
        tasks/task_audio.cpp

        # --- Assets ---
        assets/gfx/title.cpp
        assets/gfx/atlas.cpp

        assets/music/WF-MAGES.cpp
        assets/music/WF-DRAGO.cpp
        assets/music/CRYSTAL.cpp
        assets/music/MISTHART.cpp
        assets/music/baba_samba_la_baba.cpp
        assets/music/baba_music_2.cpp
        assets/music/baba_cave_newer_short.cpp

    INCLUDE_DIRS
        .
        core
        game
        tasks
        assets
        assets/gfx
        assets/music

    REQUIRES
        gamebuino
)
