# QuickdmgKit

<p align="center">
  <b>专为 macOS 平台打造的高性能免挂载 DMG 极速解压与沙盒化 DMG 制作引擎。</b>
</p>

<p align="center">
  <a href="README.md">English</a> | <b>简体中文</b>
</p>

---

## 🌟 项目简介

**QuickdmgKit** 是一个基于 7-Zip 原生 C/C++ 核心改造、并内嵌纯 C 语言 `.DS_Store` 二进制编码器的高性能 Apple 磁盘镜像（DMG）解析、提取与创建引擎，编译为标准的 Universal XCFramework（支持 macOS `arm64` 与 `x86_64`）。

它专为 macOS 平台设计，一方面支持**免内核挂载（Zero-Mount）**的极速应用提取与完整符号链接还原，另一方面支持**沙盒安全（App Sandbox Compliant）**的 DMG 自定义布局制作，彻底告别脆弱的 AppleScript Finder 自动化与跨进程权限限制。

---

## ✨ 核心特性与技术优势

### 1. 🔗 完美还原 Framework 符号链接（解压引擎）
- **痛点**：原生 7-Zip 在处理 macOS HFS+ 镜像时，会将符号链接节点当作普通文件提取并写入脏数据，导致含有内嵌 Framework（如 Electron、Chromium、Sparkle 等）的 macOS App 损坏。
- **方案**：QuickdmgKit 深度修补了 7-Zip 的 `HfsHandler.cpp`，正确导出 `kpidSymLink` 属性并在解压端通过原生 POSIX `symlink()` 原子还原软链接。

### 2. 🎨 纯 C 语言 `.DS_Store` 二进制生成器（制作引擎）
- 使用纯标准 C 完整实现 Apple Buddy Allocator 内存分配器与 B-tree 二进制结构。
- 直接在内存中生成 `Iloc`（应用与快捷方式图标坐标）、`bwsp`（Finder 窗口尺寸与隐藏侧边栏/工具栏）、`icvo`（图标大小 48~128pt）、`BKGD`（背景图片路径映射）。
- **零 Finder 进程依赖、零 AppleScript 跨进程 IPC**，完全符合 macOS App Sandbox 与 Mac App Store 审核准则。

### 3. 🔒 Apple 加密 DMG (`encrcdsa` V2) 流式解密
- 原生支持 AES-128 与 AES-256 加密 DMG 镜像（基于 PBKDF2-HMAC-SHA1 派生、3DES 解包、`koly` trailer 密码校验与 AES-CBC 逐扇区内存流式解密）。
- 全程流式解密，零磁盘临时明文开销。

### 4. ⚡️ 零内核挂载（Zero Mount Overhead）
- 提取过程完全在用户空间工作，无需调用 `hdiutil attach`，无须系统挂载守护进程参与，避免挂载等待与权限弹窗。

### 5. 📦 双架构 Universal XCFramework
- 包含 Apple Silicon (`arm64`) 与 Intel (`x86_64`) 完整原生支持。
- 导出规范的 C API 与 Clang 模块映射（`module.modulemap`），Swift 6 可直接 `import QuickdmgKit`。

---

## 🚀 Swift 接入指南

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
            url: "https://github.com/SteveShi/quickdmg-kit/releases/download/v1.0.0/QuickdmgKit.xcframework.zip",
            checksum: "d4f0279c19068c9b2ba356653591762294cf72ea6a1900b29f14dd15b7f75b81"
        ),
    ]
)
```

### 2. Swift 调用示例

#### A. 解压 DMG 镜像
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
    print("解压进度: \(Int(pct))%")
}, nil)

quickdmg_close(handle)
```

#### B. 制作排版精美的 DMG 安装镜像
```swift
import QuickdmgKit

var appPos = quickdmg_icon_position(filename: "MyApp.app", x: 150, y: 200)
var appsPos = quickdmg_icon_position(filename: "Applications", x: 450, y: 200)
var positions = [appPos, appsPos]

var config = quickdmg_create_config(
    volume_name: "MyApp 安装包",
    source_dir: "/path/to/source_dir",
    background_image: "/path/to/background.png",
    window_width: 600,
    window_height: 400,
    icon_size: 128,
    icon_positions: &positions,
    num_icon_positions: 2
)

let status = quickdmg_create_dmg(&config, "/path/to/output.dmg", { completed, total, _, _ in
    print("当前步骤: \(completed)/\(total)")
}, nil)
```

---

## 🔨 本地编译构建

```bash
# 运行自动化构建脚本生成 QuickdmgKit.xcframework
./Scripts/make_xcframework.sh

# 编译产物位于 output/QuickdmgKit.xcframework
```

---

## 🤖 CI / GitHub Actions 自动编译

本项目配置了完整的 GitHub Actions 工作流：
- **`build.yml`**：在每次 `push` 或 `pull_request` 到 `main` 分支时自动在 `macos-14` runner 上编译并验证 Universal XCFramework。
- **`release.yml`**：当发布新版本 Tag（如 `v1.0.0`）时，自动打包生成 `QuickdmgKit.xcframework.zip`、计算 SHA256 Checksum 并将其发布至 GitHub Release 资产中。

---

## 📄 许可证与开源归属 (License & Attribution)

- 7-Zip 上游引擎：**GNU LGPL v2.1 or later** (Igor Pavlov)
- LZFSE 解压模块：**BSD 3-Clause** (Apple Inc.)
- Zstandard 解压模块：**BSD 3-Clause** (Facebook Inc.)
- QuickdmgKit 深度修改、C Bridge 接口及 `.DS_Store` 生成器：**GNU LGPL v2.1 or later** (Steve Shi / 轩楝)
