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
            path: "output/QuickdmgKit.xcframework"
        )
    ]
)
