# QuickdmgKit

<p align="center">
  <b>High-performance, non-mounting DMG & disk image extraction engine for macOS & iOS.</b><br>
  <b>专为 Apple 平台打造的免挂载 DMG 镜像极速解压与符号链接完整性还原引擎。</b>
</p>

---

## 简体中文 (Simplified Chinese)

### 🌟 项目简介

**QuickdmgKit** 是一个基于 7-Zip 原生 C/C++ 核心改造的高性能 Apple 磁盘镜像（DMG）解压引擎框架，编译为标准的 Universal XCFramework（支持 `macOS arm64/x86_64` 及 iOS）。

它专为免内核挂载（Non-mounting）、超高速应用解压提取和归档分析而设计，彻底解决了传统工具（如 Rapidmg）与原生 7-Zip 在 macOS 平台上的关键缺陷。

### ✨ 核心特性与技术优势

1. **🔗 完美还原 Framework 符号链接（Symlink Integrity）**
   - **痛点**：原生 7-Zip 在处理 macOS HFS+ 镜像时，会将符号链接节点当作普通文件提取，写入脏数据，导致含有内嵌 Framework（如 Electron、Chromium、Sparkle 等）的 macOS App 损坏。
   - **方案**：QuickdmgKit 深度修补了 7-Zip 的 `HfsHandler.cpp`，正确导出 `kpidSymLink` 属性并在解压端通过原生 POSIX `symlink()` 原子还原软链接。
2. **🔒 Apple 加密 DMG (`encrcdsa` V2) 流式解密**
   - 原生支持 AES-128 与 AES-256 加密 DMG 镜像（基于 PBKDF2-HMAC-SHA1 派生、3DES 解包、`koly` trailer 密码校验与 AES-CBC 逐扇区内存流式解密）。
   - 全程流式解密，零磁盘临时大文件开销。
3. **⚡️ 零内核挂载（Zero Mount Overhead）**
   - 完全在用户空间工作，无需调用 `hdiutil attach`，无须系统挂载守护进程参与，避免挂载等待与权限弹窗。
4. **📦 双架构 Universal XCFramework**
   - 包含 Apple Silicon (`arm64`) 与 Intel (`x86_64`) 完整原生支持。
   - 导出干净整洁的 C API 与 Clang 模块映射（`module.modulemap`），Swift 可直接 `import QuickdmgKit`。

---

## English

### 🌟 Overview

**QuickdmgKit** is a high-performance Apple Disk Image (DMG) parsing and extraction engine built on a custom-patched 7-Zip C/C++ core, packaged as a Universal XCFramework for macOS (`arm64` / `x86_64`).

Designed for zero-mount extraction workflows, it resolves fundamental shortcomings found in previous community utilities like Rapidmg and stock 7-Zip builds.

### ✨ Key Features

- **🔗 Framework Symlink Fidelity**: Custom-patched HFS+ engine accurately restores nested POSIX symbolic links for Electron, Sparkle, and modern multi-bundle frameworks.
- **🔒 Apple Encrypted DMG Support**: Memory-streaming AES-128/256-CBC decryption with PBKDF2 key derivation and `koly` trailer authentication.
- **⚡️ Non-Mounting Speed**: Fully userspace parsing without kernel mount operations or DiskArbitration delay.
- **📦 Clean C API & Swift Module**: Universal binary with ready-to-use Swift 6 async wrappers.

---

## 🚀 Installation & Swift Integration / 接入指南

### 1. Swift Package Manager (SPM) 远程二进制依赖

在你的 `Package.swift` 中添加：

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
            url: "https://github.com/steveshi/quickdmg-kit/releases/download/v1.0.0/QuickdmgKit.xcframework.zip",
            checksum: "<COMPUTED_SHA256_CHECKSUM>"
        ),
    ]
)
```

### 2. Swift 调用示例

```swift
import QuickdmgKit

// 打开 DMG 镜像（支持密码回调）
var archive: OpaquePointer?
let status = quickdmg_open("/path/to/app.dmg", { buf, maxLen, _ in
    let pass = "mypassword"
    strncpy(buf, pass, maxLen - 1)
    return true
}, nil, &archive)

guard status == QUICKDMG_OK, let handle = archive else {
    fatalError("Failed to open DMG")
}

// 极速免挂载解压至目标目录
quickdmg_extract_all(handle, "/Applications", { completed, total, currentPath, _ in
    let pct = total > 0 ? Double(completed) / Double(total) * 100 : 0
    print("Extracting: \(Int(pct))%")
}, nil)

quickdmg_close(handle)
```

---

## 🔨 Build from Source / 本地编译

```bash
# 运行自动化构建脚本生成 QuickdmgKit.xcframework
./Scripts/make_xcframework.sh

# 编译产物位于 output/QuickdmgKit.xcframework
```

---

## 🤖 CI / GitHub Actions 自动编译

本项目配置了完整的 GitHub Actions 工作流：
- **`build.yml`**：在每次 `push` 或 `pull_request` 到 `main` 分支时自动在 macOS runner 上编译并验证 XCFramework。
- **`release.yml`**：当发布新版本 Tag（如 `v1.0.0`）时，自动打包生成 `QuickdmgKit.xcframework.zip`、计算 SHA256 Checksum 并将其附加到 GitHub Release 资产中。

---

## 📄 License & Attribution / 许可证

- 7-Zip upstream engine: **GNU LGPL v2.1 or later** (Igor Pavlov)
- LZFSE Decompressor: **BSD 3-Clause** (Apple Inc.)
- Zstandard Decompressor: **BSD 3-Clause** (Facebook Inc.)
- QuickdmgKit modifications & Bridge: **GNU LGPL v2.1 or later** (Steve Shi / 轩楝)
