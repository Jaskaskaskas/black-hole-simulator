CC = g++
PKG_CONFIG = pkg-config
SRC = main.cpp
GRP = graphics_tools.cpp
PHY = physics.cpp
OUT = bh

.PHONY: build run clean

build:
	$(CC) -fopenmp $(shell $(PKG_CONFIG) --cflags sdl2) $(SRC) $(GRP) $(PHY) -o $(OUT) $(shell $(PKG_CONFIG) --libs sdl2) -fopenmp

run: build
	./$(OUT)

clean:
	rm -f $(OUT)
