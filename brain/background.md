---
slug: background
title: Project background
role: project background
updated: "2026-08-21T06:38:38"
---

# Project background

`quickdmg-kit` is a high-performance, non-mounting Apple Disk Image (DMG) extraction and creation engine for macOS. It provides direct user-space parsing of UDIF disk image structures and APFS/HFS+ file systems.

## Goals
- Parse and extract DMG disk images without kernel mounting (`hdiutil`).
- Support multiple compression formats (zlib, bzip2, lzfse, lzma).
- Provide a clean Swift and C API for client applications.
