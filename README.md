# RADIOFALL

### Description

A falling block game taking place inside an old radio cassette!

![Radiofall Gameplay](./assets/gameplay.gif)

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

### Developers

- https://github.com/chrg127
- https://github.com/rhighs

### Build

- Release build
    ```sh
    make release
    ```

- Debug build
    ```sh
    make debug
    ```

### Compiling to WASM

- raylib project should be cloned and located at `../raylib`
- then compile and link the raylib libraries for web platform via [this guide](https://github.com/raysan5/raylib/wiki/Working-for-Web-%28HTML5%29#21-command-line-compilation)
- in radiofall root
    ```bash
    make release # just to have the resources resolved under build/release
    cd src
    PLATFORM=PLATFORM_WEB make
    cd ..
    mv src/radiofall.html src/radiofall.js src/radiofall.wasm build/release/radiofall/
    ```

### Running on web

Spin up a local web server
```
python -m http.server 8000
```

then head to http://localhost:8000/build/release/radiofall/radiofall.html
