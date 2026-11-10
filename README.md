# BattleSpud — 3DS Homebrew CIA

Battle royale on Old 3DS. 8-player (1 local + 7 bots). Storm shrinks. Loot spawns. Top screen renders. Bottom screen HUD.

## Controls
- Circle Pad — move
- D-Pad — aim direction
- A — shoot
- START — quit

## Build
```
source /etc/profile.d/devkit-env.sh
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```
Produces `BattleSpud.3dsx`. For CIA:
```
makerom -f cia -o BattleSpud.cia -rsf ../BattleSpud.rsf -target t -exefslogo -elf BattleSpud.elf -icon BattleSpud.smdh
```
Install via FBI on 3DS.

## Deps
- devkitARM r57
- libctru 2.3.0
- citro3d 1.7.0
- makerom (for CIA)
- icon.png 48×48 (placeholder ok)

## FILL
- BattleSpud.rsf — standard homebrew RSF, grab template from ctrulib/examples
- icon.png — 48×48 PNG, any image
