// swift-tools-version: 6.0
import PackageDescription

let package = Package(
    name: "QuickdmgKit",
    platforms: [
        .macOS(.v14)
    ],
    products: [
        .library(
            name: "QuickdmgKit",
            targets: ["QuickdmgKit"]
        )
    ],
    targets: [
        .binaryTarget(
            name: "QuickdmgKit",
            url: "https://github.com/SteveShi/quickdmg-kit/releases/download/v1.0.0/QuickdmgKit.xcframework.zip",
            checksum: "d4f0279c19068c9b2ba356653591762294cf72ea6a1900b29f14dd15b7f75b81"
        )
    ]
)
