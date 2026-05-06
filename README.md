# Project Name

> One-line description of what this does.

## Requirements

- CMake >= 3.20
- GCC >= 12 / Clang >= 15
- (optional) AddressSanitizer runtime

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### Release (no sanitizers)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Run

```bash
./build/your_binary
```

## Compiler Flags

### Warnings

| Flag                 | Meaning                                               |
| -------------------- | ----------------------------------------------------- |
| `-Wall`              | Common warnings (not literally *all*)                 |
| `-Wextra`            | Extra warnings beyond `-Wall`                         |
| `-Wpedantic`         | Strict ISO C/C++ compliance                           |
| `-Werror`            | Treat warnings as errors — build fails on any warning |
| `-Wshadow`           | Warn if local var shadows outer var                   |
| `-Wconversion`       | Warn on implicit type conversions (e.g. `double→int`) |
| `-Wsign-conversion`  | Warn on signed/unsigned mismatch                      |
| `-Wnull-dereference` | Warn on detected null pointer deref paths             |
| `-Wdouble-promotion` | Warn when `float` silently promotes to `double`       |

### Hardening

| Flag                       | Meaning                                                                      |
| -------------------------- | ---------------------------------------------------------------------------- |
| `-fstack-protector-strong` | Canary values on stack — detects stack overflows at runtime                  |
| `_FORTIFY_SOURCE=2`        | Replaces unsafe libc calls (`strcpy`, `memcpy`) with bounds-checked versions |

> ✅ Safe for production builds.

### Debug / Sanitizers

| Flag                      | Meaning                                                            |
| ------------------------- | ------------------------------------------------------------------ |
| `-g`                      | Include debug symbols                                              |
| `-fsanitize=address`      | **ASan** — detects heap/stack buffer overflows, use-after-free     |
| `-fsanitize=undefined`    | **UBSan** — detects undefined behavior (overflow, bad casts, etc.) |
| `-fno-omit-frame-pointer` | Keep frame pointer — readable stack traces in sanitizer output     |
| `add_link_options(...)`   | Sanitizers must be linked too, not just compiled                   |

> ⚠️ **Never ship production binaries with `-fsanitize`** — ~2x runtime overhead.

## Project Structure

```
.
├── CMakeLists.txt
├── src/
│   └── main.c
├── include/
└── tests/
```

## License

MIT