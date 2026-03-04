.SHELL := /bin/bash

CCC := clang++

# Key flags:
# -Werror — treat warnings as errors (remove if too strict initially)
# -fsanitize=address,undefined — catch memory bugs & UB in debug
# -flto — link-time optimization for release
# -fstack-protector-strong + _FORTIFY_SOURCE — hardening
# -Wshadow -Wconversion — catches subtle bugs
# -static <- full build
CCC_OPTS := -std=c++17 \
			-Wall -Wextra -Wpedantic -Werror \
            -Wshadow -Wconversion -Wsign-conversion \
            -Wnull-dereference -Wdouble-promotion \
            -fstack-protector-strong \
            -D_FORTIFY_SOURCE=2 \
            -g -fsanitize=address,undefined -fno-omit-frame-pointer
CCC_OPTS_COMPILE := $(CCC_OPTS) -c

install:
	sudo apt install clang
	sudo apt install valgrind
	sudo apt install libpqxx-dev

IMAGE_BUILDER := clang/clang17:builder
IMAGE_RUNNER := clang/clang17:runner
DOCKER_USER := --user "$$(id -u):$$(id -g)"
BUILDER := docker run -it -v .:/app -w /app --rm $(IMAGE_BUILDER)
RUNNER := docker run -it $(DOCKER_USER) -v .:/app -w /app -p 8080:8080 --rm $(IMAGE_RUNNER)

tag-builder:
	docker build -f ./docker/Builder -t $(IMAGE_BUILDER) .
tag-runner:
	docker build -f ./docker/Runner -t $(IMAGE_RUNNER) .
tag: tag-builder tag-runner
exec-builder:
	$(BUILDER) sh
exec-runner:
	$(RUNNER) sh

NON_APPS_CPP := $(shell find src -name "*.cpp" ! -wholename "src/apps/*")
# src/utils/string.cpp -> utils/string.cpp
NON_APPS_CPP_RELATIVE := $(shell find src -name "*.cpp" ! -wholename "src/apps/*" | cut -c5-)
NON_APPS_CPP_O := $(patsubst %.cpp,build/%.o,$(NON_APPS_CPP_RELATIVE))

echo:
	@echo $(NON_APPS_CPP_RELATIVE)
	@echo $(NON_APPS_CPP_O)
	@echo $(ALL_CPP)
	@echo $(ALL_CPP_O)

# build-socket-server:
# 	@$(BUILDER) $(CCC) $(CCC_OPTS) $(NON_APPS_CPP) src/apps/socket-server.cpp -o bin/socket-server
# run-socket-server:
# 	$(RUNNER) ./bin/socket-server

# ===== COMPILE AND LINK SEPARATELY
ALL_CPP := $(shell find src -name "*.cpp")
ALL_CPP_RELATIVE := $(shell find src -name "*.cpp")
ALL_CPP_O := $(patsubst src/%.cpp,build/%.o,$(ALL_CPP_RELATIVE))

clean-bin:
	rm -fr bin/*
clean-build:
	rm -fr build/*

build/%.o: src/%.cpp
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	@$(BUILDER) $(CCC) $(CCC_OPTS_COMPILE) $< -o $(@)
	@echo "$@ compiled"

build-all: $(ALL_CPP_O)

telemetry-producer:
	$(CCC) $(CCC_OPTS) \
	test/socket-telemetry-producer.cpp \
	-o bin/telemetry_producer
	#valgrind --leak-check=full --show-leak-kinds=all ./bin/telemetry_producer 5 0 65000
	read -p "Number of events: " EVENTS; \
	./bin/telemetry_producer $$EVENTS 0 65000 
telemetry-udp-producer:
	$(CCC) $(CCC_OPTS) \
	test/udp-telemetry-producer.cpp \
	-o bin/udp_telemetry_producer
	read -p "Number of events: " EVENTS; \
	./bin/udp_telemetry_producer $$EVENTS 0 65000 
telemetry-server:
	$(CCC) $(CCC_OPTS) \
	-lpqxx -lpq \
	src/telemetry-server/storage/telemetry-storage.cpp \
	src/telemetry-server/socket-telemetry-server.cpp \
	src/telemetry-server/telemetry-server.cpp \
	-o bin/telemetry_server
	#valgrind --leak-check=full --show-leak-kinds=definite ./bin/telemetry_server
	./bin/telemetry_server
telemetry-udp-producer:
	$(CCC) $(CCC_OPTS) \
	test/udp-telemetry-producer.cpp \
	-o bin/udp_telemetry_producer
	read -p "Number of events: " EVENTS; \
	./bin/udp_telemetry_producer $$EVENTS 0 65000 
telemetry-udp-server:
	$(CCC) $(CCC_OPTS) \
	src/telemetry-server/udp-telemetry-server.cpp \
	src/telemetry-server/telemetry-server.cpp \
	-o bin/udp_telemetry_server
	./bin/udp_telemetry_server


.PHONY: build-all