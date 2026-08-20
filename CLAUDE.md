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
