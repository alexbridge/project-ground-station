BUILD_DIR := build

.PHONY: init build clean install-deps clangd-check \
        telemetry-api telemetry-ingestor

install-deps:
	sudo apt install clang clangd clang-format clang-tidy cmake valgrind libpq-dev

init:
	cmake -S . -B $(BUILD_DIR)

build:
	cmake --build $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

telemetry-api: build
	./$(BUILD_DIR)/src/app/telemetry_api

telemetry-ingestor: build
	./$(BUILD_DIR)/src/app/telemetry_ingestor

clangd-check:
	@find . -type f \( -name '*.h' -o -name '*.cpp' -o -name '*.hpp' \) | while read f; do \
		output=$$(clangd --check="$$f" 2>&1); \
		if [ $$? -gt 0 ]; then \
			echo ""; \
			echo "=== FAIL: $$f ==="; \
			echo "$$output" | tail -7; \
			echo "=== FAIL: $$f ==="; \
			echo ""; \
		else \
			echo "OK: $$f"; \
		fi \
	done
