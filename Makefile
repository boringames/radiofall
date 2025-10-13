.PHONY: release debug
run: release
	./build/release/radiofall/radiofall

release:
	cmake --preset=debug
	cmake --build build/debug

debug:
	cmake --preset=release
	cmake --build build/release
