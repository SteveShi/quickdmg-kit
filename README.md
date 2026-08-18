# QuickdmgKit

<p align="center">
  <b>High-performance, non-mounting DMG & disk image extraction engine for macOS & iOS.</b>
</p>

<p align="center">
  <b>English</b> | <a href="README.zh-CN.md">简体中文</a>
</p>

---

## 🌟 Overview

**QuickdmgKit** is a high-performance Apple Disk Image (DMG) parsing and extraction engine built on a custom-patched 7-Zip C/C++ core, packaged as a Universal XCFramework for macOS (`arm64` / `x86_64`).

Designed for zero-mount extraction workflows, it resolves fundamental shortcomings found in previous community utilities (such as Rapidmg) and stock 7-Zip builds.

---

## ✨ Key Features & Technical Highlights

1. **🔗 Framework Symlink Fidelity**
   - **The Problem**: Stock 7-Zip extracts HFS+ symbolic links as plain files and populates them with garbage data, breaking macOS applications with nested Frameworks (such as Electron, Chromium, and Sparkle).
   - **The Fix**: QuickdmgKit deep-patches 7-Zip's `HfsHandler.cpp` to correctly flag `kpidSymLink` properties and restores valid POSIX symlinks atomically on extraction.
2. **🔒 Apple Encrypted DMG (`encrcdsa` V2) Streaming Decryption**
   - Native support for AES-128 and AES-256 encrypted DMG images.
   - Powered by PBKDF2-HMAC-SHA1 key derivation, 3DES key unwrapping, `koly` trailer password verification, and per-sector AES-CBC in-memory decryption.
   - Zero temporary plaintext files written to disk.
3. **⚡️ Zero Kernel Mounting Overhead**
   - Operates entirely in userspace without calling `hdiutil attach` or invoking `diskarbitrationd`.
4. **📦 Universal XCFramework Binary**
   - Native Apple Silicon (`arm64`) and Intel (`x86_64`) support.
   - Clean C API and Clang module map (`module.modulemap`) for seamless `import QuickdmgKit` in Swift 6.

---

## 🚀 Swift Integration

### 1. Swift Package Manager (SPM)

Add the binary target to your `Package.swift`:

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

### 2. Swift Code Example

```swift
import QuickdmgKit

// Open DMG image with optional password callback
var archive: OpaquePointer?
let status = quickdmg_open("/path/to/app.dmg", { buf, maxLen, _ in
    let pass = "mypassword"
    strncpy(buf, pass, maxLen - 1)
    return true
}, nil, &archive)

guard status == QUICKDMG_OK, let handle = archive else {
    fatalError("Failed to open DMG")
}

// Stream-extract all items to target directory
quickdmg_extract_all(handle, "/Applications", { completed, total, currentPath, _ in
    let pct = total > 0 ? Double(completed) / Double(total) * 100 : 0
    print("Progress: \(Int(pct))%")
}, nil)

quickdmg_close(handle)
```

---

## 🔨 Build from Source

```bash
# Run automated build script to produce QuickdmgKit.xcframework
./Scripts/make_xcframework.sh

# Output is located at: output/QuickdmgKit.xcframework
```

---

## 🤖 CI / GitHub Actions

- **`build.yml`**: Automatically builds and verifies the XCFramework on `macos-14` for every push and pull request to `main`.
- **`release.yml`**: Automatically builds the framework, packages `QuickdmgKit.xcframework.zip`, computes SHA256, and creates a GitHub Release when a version tag (`v*`) is pushed.

---

## 📄 License & Attribution

- 7-Zip upstream engine: **GNU LGPL v2.1 or later** (Igor Pavlov)
- LZFSE Decompressor: **BSD 3-Clause** (Apple Inc.)
- Zstandard Decompressor: **BSD 3-Clause** (Facebook Inc.)
- QuickdmgKit modifications & C Bridge: **GNU LGPL v2.1 or later** (Steve Shi / 轩楝)
