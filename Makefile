BUILD_DIR := build
BUILD_TYPE :=Release
VALGRIND := valgrind --leak-check=full --track-origins=yes

.PHONY: init build clean install-deps clangd-check \
        telemetry-api telemetry-ingestor

install-deps:
	sudo apt install clang clangd clang-format clang-tidy cmake valgrind libpq-dev

init:
	cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -S . -B $(BUILD_DIR)

build:
	cmake --build $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

telemetry-udp-api:
	@./$(BUILD_DIR)/src/app/telemetry_api

telemetry-af-unix-ingestor:
	@$(VALGRIND) ./$(BUILD_DIR)/src/app/telemetry_ingestor

telemetry-af-unix-producer:
	@./$(BUILD_DIR)/test/af_unix_dgram_producer 8192 0 64000

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
