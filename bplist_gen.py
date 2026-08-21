import plistlib
import struct

bwsp = {
    'ShowStatusBar': False,
    'WindowBounds': '{{0, 0}, {400, 300}}',
    'ContainerShowSidebar': False,
    'PreviewPaneVisibility': False,
    'SidebarWidth': 0,
    'ShowTabView': False,
    'ShowToolbar': False,
    'ShowPathbar': False
}

icvo = {
    'backgroundType': 2,
    'backgroundColorBlue': 1.0,
    'backgroundColorGreen': 1.0,
    'backgroundColorRed': 1.0,
    'iconSize': 128.0,
    'gridOffsetX': 0.0,
    'gridOffsetY': 0.0,
    'gridSpacing': 100.0,
    'arrangeBy': 'none',
    'showIconPreview': True,
    'showItemInfo': False,
    'labelOnBottom': True,
    'textSize': 12.0
}

with open('bwsp.bplist', 'wb') as f:
    plistlib.dump(bwsp, f, fmt=plistlib.FMT_BINARY)

with open('icvo.bplist', 'wb') as f:
    plistlib.dump(icvo, f, fmt=plistlib.FMT_BINARY)
