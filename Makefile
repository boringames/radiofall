.PHONY: build run
run: build
	./build/game/game
build:
	cmake --build build
