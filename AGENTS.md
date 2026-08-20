# QuickdmgKit - Agent Guidelines & Instructions

This repository contains the core C/C++ engine and Universal XCFramework for **QuickdmgKit**, the high-performance non-mounting DMG extraction and creation library.

## 1. Project Standards & Identity

- **Language**: Simplified Chinese for communication. English for documentation and comments.
- **Documentation**: All READMEs must be split into `README.md` (English) and `README.zh-CN.md` (Simplified Chinese) with mutual cross-links at the top.
- **Author**: Steve Shi / 轩楝 (`zh-Hans`).
- **Default Branch**: `main`.
- **License**: GNU LGPL v2.1+ / BSD 3-Clause.
- **Platform Scope**: macOS only (`arm64` and `x86_64`). No iOS references.

## 2. Engineering & Architecture Rules

- **Core Engine**: Custom-patched 7-Zip (`7zip/CPP/7zip/Archive/HfsHandler.cpp`) with atomic POSIX symlink handling.
- **Metadata Generation**: Pure C `.DS_Store` binary encoder (`Bridge/src/dsstore_builder.c`) without AppleScript or Finder IPC.
- **Bridge Layer**: Clean C API (`Bridge/include/quickdmg_kit.h`) with Clang module map.
- **Framework Build**: Automated via `./Scripts/make_xcframework.sh` into `output/QuickdmgKit.xcframework`.

## 3. Build & Release Commands

```bash
# Build Universal XCFramework locally
./Scripts/make_xcframework.sh

# Run Tests
swift test
```
