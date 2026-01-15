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

        # --- Lib Gamebuino AKA (bas niveau) ---
        lib/gb_ll_adc.c
        lib/gb_ll_audio.c
        lib/gb_ll_expander.c
        lib/gb_ll_i2c.c
        lib/gb_ll_lcd.c
        lib/gb_ll_sdcard.c
        lib/gb_ll_system.c

        # --- Lib Gamebuino AKA (haut niveau) ---
        lib/gb_core.cpp
	lib/font8x8_basic.cpp
        lib/gb_graphics.cpp
        lib/gb_audio_player.cpp
        lib/gb_audio_track_tone.cpp
        lib/gb_audio_track_fx.cpp
        lib/gb_audio_track_wav.cpp
        lib/gb_audio_track_pmf.cpp

        # --- PMF Player officiel ---
        lib/pmf_player/pmf_player.cpp

        # --- Logique de jeu ---
        game/game.cpp
        game/levels_data.cpp
        game/levels.cpp
        game/options.cpp

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
        lib
        lib/pmf_player
        game
        tasks
        assets
        assets/gfx
        assets/music
)
