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

<!-- BEGIN brain.md -->
## Project Brain

This project keeps a **Project Brain**: a persistent memory layer of its durable decisions, requirements, and constraints. Read `./BRAIN.md` for the full read/write contract.

Maintain the brain as part of normal coding work — not as a separate task. While discussing or implementing features:
- **Start of a task:** load relevant context with the `brain` CLI (`list-pages`, `read-page`, `read-root`). Prefer a narrow read over scanning everything.
- **When a decision, requirement, constraint, or durable insight settles** (in chat or while coding): capture it immediately via the `brain` CLI. Do not wait to be asked and do not batch it for later.
- **Pure implementation with no new decision:** do not write to the brain.
- **When overturning a prior conclusion:** update the page (`update-truth` and/or `append-timeline` with `kind: reversal`, or `archive-page`).
- Only store what will still matter in six months and is hard to reconstruct from the code alone.
- All reads and writes go through the `brain` CLI — never hand-edit brain files.

The brain skills (`brain-setup`, `brain-page`, `brain-ingest`, `brain-bootstrap`) are installed in your global skills directory. Prefer `brain init` to scaffold a new project.
<!-- END brain.md -->
