# QuickdmgKit

<p align="center">
  <b>High-performance, non-mounting Apple Disk Image (DMG) extraction and creation engine for macOS.</b>
</p>

<p align="center">
  <b>English</b> | <a href="README.zh-CN.md">简体中文</a>
</p>

---

## 🌟 Overview

**QuickdmgKit** is a high-performance Apple Disk Image (DMG) parsing, extraction, and generation engine packaged as a Universal XCFramework for macOS (`arm64` / `x86_64`).

Built upon a custom-patched 7-Zip C/C++ core with an integrated pure C `.DS_Store` binary encoder, QuickdmgKit enables both **instant zero-mount DMG extraction** and **sandboxed visual DMG creation** without relying on deprecated Carbon APIs or fragile AppleScript Finder automation.

---

## ✨ Key Features & Technical Highlights

### 1. 🔗 Framework Symlink Fidelity (Extraction)
- **The Problem**: Stock 7-Zip extracts HFS+ symbolic links as regular files and writes garbage bytes, breaking macOS applications containing nested Frameworks (such as Electron, Chromium, and Sparkle).
- **The Fix**: QuickdmgKit deep-patches 7-Zip's `HfsHandler.cpp` to accurately recognize `kpidSymLink` metadata and restores valid POSIX symlinks atomically on extraction.

### 2. 🎨 Pure C `.DS_Store` Binary Generator (Creation)
- Self-contained implementation of Apple's Buddy Allocator and B-tree binary encoding in standard C.
- Directly constructs `Iloc` (custom icon coordinates), `bwsp` (Finder window bounds and chrome-less styling), `icvo` (icon size from 48pt to 128pt), and `BKGD` (background image path).
- **Zero AppleScript / Finder IPC required** — fully compliant with App Sandbox and Mac App Store review guidelines.

### 3. 🔒 Apple Encrypted DMG (`encrcdsa` V2) Streaming Decryption
- Native support for AES-128 and AES-256 encrypted DMG containers.
- Features PBKDF2-HMAC-SHA1 key derivation, 3DES key unwrapping, `koly` trailer password verification, and per-sector AES-CBC in-memory stream decryption.
- Zero plaintext footprint written to disk.

### 4. ⚡️ Zero Kernel Mounting Overhead
- Parsing and extraction operate entirely in userspace without calling `hdiutil attach` or involving `diskarbitrationd`.

### 5. 📦 Universal XCFramework Binary
- Native Apple Silicon (`arm64`) and Intel (`x86_64`) support.
- Clean C API with complete Clang module map (`module.modulemap`) for seamless Swift 6 integration.

---

## 🚀 Swift Integration

### 1. Swift Package Manager (SPM)

```swift
// swift-tools-version: 6.0
import PackageDescription

let package = Package(
    name: "MyApp",
    platforms: [.macOS(.v14)],
    products: [
        .executable(name: "MyApp", targets: ["MyApp"]),
    ],
    targets: [
        .target(
            name: "MyApp",
            dependencies: ["QuickdmgKit"]
        ),
        .binaryTarget(
            name: "QuickdmgKit",
            url: "https://github.com/SteveShi/quickdmg-kit/releases/download/v1.0.0/QuickdmgKit.xcframework.zip",
            checksum: "d4f0279c19068c9b2ba356653591762294cf72ea6a1900b29f14dd15b7f75b81"
        ),
    ]
)
```

### 2. Swift Usage Examples

#### A. Extracting a DMG Image
```swift
import QuickdmgKit

var archive: OpaquePointer?
let status = quickdmg_open("/path/to/app.dmg", { buf, maxLen, _ in
    let pass = "mypassword"
    strncpy(buf, pass, maxLen - 1)
    return true
}, nil, &archive)

guard status == QUICKDMG_OK, let handle = archive else {
    fatalError("Failed to open DMG")
}

// Stream-extract all items directly to target directory
quickdmg_extract_all(handle, "/Applications", { completed, total, currentPath, _ in
    let pct = total > 0 ? Double(completed) / Double(total) * 100 : 0
    print("Progress: \(Int(pct))%")
}, nil)

quickdmg_close(handle)
```

#### B. Creating a Styled DMG Image
```swift
import QuickdmgKit

var appPos = quickdmg_icon_position(filename: "MyApp.app", x: 150, y: 200)
var appsPos = quickdmg_icon_position(filename: "Applications", x: 450, y: 200)
var positions = [appPos, appsPos]

var config = quickdmg_create_config(
    volume_name: "MyApp Installer",
    source_dir: "/path/to/source_payload",
    background_image: "/path/to/background.png",
    window_width: 600,
    window_height: 400,
    icon_size: 128,
    icon_positions: &positions,
    num_icon_positions: 2
)

let status = quickdmg_create_dmg(&config, "/path/to/output.dmg", { completed, total, _, _ in
    print("Step \(completed) of \(total)")
}, nil)
```

---

## 🔨 Build from Source

```bash
# Run automated build script to produce QuickdmgKit.xcframework
./Scripts/make_xcframework.sh

# Build artifact is generated at: output/QuickdmgKit.xcframework
```

---

## 🤖 CI / GitHub Actions

- **`build.yml`**: Automatically builds and verifies the Universal XCFramework on `macos-14` runner for every push and pull request to `main`.
- **`release.yml`**: Automatically packages `QuickdmgKit.xcframework.zip`, computes SHA256, and creates a GitHub Release when a version tag (`v*`) is pushed.

---

## 📄 License & Attribution

- 7-Zip upstream engine: **GNU LGPL v2.1 or later** (Igor Pavlov)
- LZFSE Decompressor: **BSD 3-Clause** (Apple Inc.)
- Zstandard Decompressor: **BSD 3-Clause** (Facebook Inc.)
- QuickdmgKit modifications, C Bridge, and `.DS_Store` builder: **GNU LGPL v2.1 or later** (Steve Shi / 轩楝)
