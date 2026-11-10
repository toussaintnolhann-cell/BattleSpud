
cmake_minimum_required(VERSION 3.13)

# Configuration Nintendo 3DS
if(NOT DEFINED ENV{DEVKITPRO})
    message(FATAL_ERROR "DEVKITPRO n'est pas configure")
endif()

if(NOT DEFINED ENV{DEVKITARM})
    message(FATAL_ERROR "DEVKITARM n'est pas configure")
endif()

set(CMAKE_TOOLCHAIN_FILE "$ENV{DEVKITPRO}/cmake/3DS.cmake")

project(BattleSpud C)

# Informations du jeu
set(APP_TITLE "BattleSpud")
set(APP_DESCRIPTION "Battle Royale 3DS")
set(APP_AUTHOR "Nolhann")

# Fichiers source : ils sont directement a la racine
set(SOURCES
    main.c
    game.c
    render.c
    ui.c
)

# Creation de l'executable
add_executable(BattleSpud ${SOURCES})

# Headers
target_include_directories(BattleSpud PRIVATE
    "${CMAKE_SOURCE_DIR}"
)

# Optimisation
target_compile_options(BattleSpud PRIVATE
    -Wall
    -Wextra
    -O2
)

# Bibliotheques Nintendo 3DS
target_link_libraries(BattleSpud
    citro3d
    ctru
    m
)

# Icone
set(APP_ICON "${CMAKE_SOURCE_DIR}/icon.png")

if(NOT EXISTS "${APP_ICON}")
    message(FATAL_ERROR "icon.png manquant !")
endif()

# Creation des informations de l'application
ctr_generate_smdh(BattleSpud.smdh
    NAME "${APP_TITLE}"
    DESCRIPTION "${APP_DESCRIPTION}"
    AUTHOR "${APP_AUTHOR}"
    ICON "${APP_ICON}"
)

# Creation du fichier 3DSX
ctr_create_3dsx(BattleSpud
    SMDH BattleSpud.smdh
)
