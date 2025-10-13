.PHONY: release debug
run: release
	./build/release/radiofall/radiofall

debug:
	cmake --preset=debug
	cmake --build build/debug

release:
	cmake --preset=release
	cmake --build build/release
