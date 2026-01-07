# Black hole simulator

This repository contains a simple black hole renderer. Currently the code produces a picture of a black hole controlled by the parameters in code.

Prerequisites (Debian/Ubuntu):

```bash
sudo apt update
sudo apt install -y build-essential pkg-config libsdl2-dev
```

Build and run (uses `pkg-config` to locate SDL2):

```bash
g++ main.cpp graphics_tools.cpp physics.cpp -o bh `pkg-config --cflags --libs sdl2` -fopenmp
./bh
```

Or with the provided Makefile:

```bash
make build
make run
```

Press ESC or close the window to quit.
