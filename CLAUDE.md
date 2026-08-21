# QuickdmgKit - Claude Code Instructions

## Build & Test Commands
- **Build XCFramework**: `./Scripts/make_xcframework.sh`
- **Output Path**: `output/QuickdmgKit.xcframework`

## Architecture & Code Rules
- **Language**: C / C++20 / Swift 6.0.
- **Scope**: macOS platform only (`arm64` and `x86_64`).
- **Core Components**:
  - `7zip/`: Upstream 7-Zip core with HFS+ symlink patches.
  - `Bridge/src/dsstore_builder.c`: Standalone pure C `.DS_Store` binary encoder.
  - `Bridge/src/quickdmg_create.cpp`: Sandboxed DMG creation pipeline.
  - `Bridge/include/quickdmg_kit.h`: Exported C API.
- **Documentation**: All READMEs must be split into `README.md` and `README.zh-CN.md`.

<!-- BEGIN brain.md -->
## Project Brain

This project keeps a **Project Brain**: a persistent memory layer of its durable decisions, requirements, and constraints. Read `./BRAIN.md` for the full read/write contract.
@import ./BRAIN.md

Maintain the brain as part of normal coding work — not as a separate task. While discussing or implementing features:
- **Start of a task:** load relevant context with the `brain` CLI (`list-pages`, `read-page`, `read-root`). Prefer a narrow read over scanning everything.
- **When a decision, requirement, constraint, or durable insight settles** (in chat or while coding): capture it immediately via the `brain` CLI. Do not wait to be asked and do not batch it for later.
- **Pure implementation with no new decision:** do not write to the brain.
- **When overturning a prior conclusion:** update the page (`update-truth` and/or `append-timeline` with `kind: reversal`, or `archive-page`).
- Only store what will still matter in six months and is hard to reconstruct from the code alone.
- All reads and writes go through the `brain` CLI — never hand-edit brain files.

The brain skills (`brain-setup`, `brain-page`, `brain-ingest`, `brain-bootstrap`) are installed in your global skills directory. Prefer `brain init` to scaffold a new project.
<!-- END brain.md -->
