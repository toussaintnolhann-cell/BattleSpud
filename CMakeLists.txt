cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED ENV{DEVKITPRO})
    message(FATAL_ERROR "DEVKITPRO not set — source /etc/profile.d/devkit-env.sh")
endif()

set(CMAKE_TOOLCHAIN_FILE "$ENV{DEVKITPRO}/cmake/3DS.cmake")
project(BattleSpud C)

set(SOURCES src/main.c src/game.c src/render.c src/ui.c)
set(APP_TITLE       "BattleSpud")
set(APP_DESCRIPTION "BR on 3DS")
set(APP_AUTHOR      "olliE")
set(APP_ICON        "${CMAKE_SOURCE_DIR}/icon.png")

add_executable(BattleSpud ${SOURCES})
target_include_directories(BattleSpud PRIVATE include)
target_compile_options(BattleSpud PRIVATE -Wall -Wextra -O2)
target_link_libraries(BattleSpud citro3d ctru m)

ctr_generate_smdh(BattleSpud.smdh
    NAME        "${APP_TITLE}"
    DESCRIPTION "${APP_DESCRIPTION}"
    AUTHOR      "${APP_AUTHOR}"
    ICON        "${APP_ICON}"
)
ctr_create_3dsx(BattleSpud SMDH BattleSpud.smdh)
# CIA: makerom -f cia -o BattleSpud.cia -rsf BattleSpud.rsf -target t -exefslogo -elf BattleSpud.elf -icon BattleSpud.smdh
