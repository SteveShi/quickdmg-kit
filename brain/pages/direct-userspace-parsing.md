---
id: direct-userspace-parsing
title: Direct userspace disk image parsing
category: decision
status: active
created: "2026-08-21T06:38:39"
updated: "2026-08-21T06:38:39"
---

<!-- compiled_truth -->
quickdmg-kit parses DMG chunk tables and filesystem structures directly in userspace memory, achieving orders-of-magnitude faster extraction than macOS kernel mounting.


## Timeline

- time: 2026-08-21T06:38:39
  kind: decision
  summary: "Created this page: Direct userspace disk image parsing"
  source: git log
  affects: [direct-userspace-parsing]

- time: 2026-08-21T06:38:39
  kind: decision
  summary: Engineered userspace DMG parsing engine.
  source: git log
  affects: [direct-userspace-parsing]
