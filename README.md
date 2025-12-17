# SDL2 Simple Graphics Example

This repository contains a minimal SDL2 example in `main.cpp` that opens a window and draws a moving rectangle.

Prerequisites (Debian/Ubuntu):

```bash
sudo apt update
sudo apt install -y build-essential pkg-config libsdl2-dev
```

Build and run (uses `pkg-config` to locate SDL2):

```bash
g++ main.cpp -o bh `pkg-config --cflags --libs sdl2`
./bh
```

Or with the provided Makefile:

```bash
make build
make run
```

Press ESC or close the window to quit.
