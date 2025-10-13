# Game

## RADIOFALL

![RADIOFALL](screenshots/screenshot000.png "Game")

### Description

A falling block game taking place inside an old radio cassette!

### Features

- Gameplay similar to Tetris and Puyo Puyo
- Move around a piece and match corresponding colors
- Any matching line will be taken by the radio, shaken and given back to you
- Beware of the volume! It will constantly go down. Continue matching lines to keep it up!

### Controls

Keyboard:
- ←, →: move around piece
- ↓: push down piece
- Z: rotate piece clockwise
- X: rotate piece counterclockwise

### Screenshots

_TODO: Show your game to the world, animated GIFs recommended!._

### Developers

 - chrg: programming and graphics
 - rhighs: programming and graphics

### Links

 - itch.io Release: $(itch.io Game Page)

### Build

- Type the follow command:

```sh
cmake . -B build -D CMAKE_BUILD_TYPE=Debug
cmake --build build
```

- Inside the build directory is another directory, called `game`, with the executable and resources folder.
- CMake will automatically download a current release of raylib but if you want to use your local version you can pass `-DFETCHCONTENT_SOURCE_DIR_RAYLIB=<dir_with_raylib>`

### License

This game sources are licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE) for further details.

*Copyright (c) 2025 chrg (chrg127), rhighs*

### TODO

- animazione della preview
- musica in generale
- animazione per le casse della radio (i due cerchi a destra e a sinistra)
- fare qualcosa in più con l'indicatore del volume (pensare a cosa fare)
- mettere il titolo per bene
