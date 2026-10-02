pragma Singleton

import QtQuick

QtObject {
    readonly property color window:  "#1e1e1e"
    readonly property color panel:   "#2d2d2d"
    readonly property color input:   "#1e1e1e"

    readonly property color border:        "#404040"
    readonly property color buttonBorder:  "#505050"
    readonly property color text:          "#e0e0e0"
    readonly property color textDisabled:  "#606060"
    readonly property color placeholder:   "#808080"

    readonly property color accent:      "#249D45"
    readonly property color accentDark:  "#143E1D"
    readonly property color pressed:     "#0f2e16"

    readonly property color table:        "#252525"
    readonly property color tableAlt:     "#2a2a2a"
    readonly property color gridline:     "#404040"
    readonly property color selection:    "#3a5a3a"
    readonly property color selectionMuted:"#3a3a3a"
    readonly property color selectionText:"#ffffff"
    readonly property color header:       "#143E1D"

    readonly property color buttonBg:     "#3c3f41"

    readonly property color danger:        "#ff4444"
    readonly property color dangerHover:   "#ff6666"
    readonly property color dangerPressed: "#cc0000"

    readonly property color groupTitle:   "#249D45"
    readonly property color progressStart:"#143E1D"
    readonly property color progressEnd:  "#249D45"
    readonly property color tooltipBg:    "#143E1D"
    readonly property color tooltipText:  "#ffffff"
    readonly property color tooltipBorder:"#249D45"
    readonly property color menuHover:    "#3a5a3a"
    readonly property color splitter:     "#404040"
    readonly property color splitterHover:"#249D45"

    readonly property string fontFamily: "Segoe UI"
    readonly property real   fontSize:   9
    readonly property real   radius:     4
    readonly property real   radiusSmall:2
    readonly property real   radiusLarge:8
    readonly property real   scrollBarThickness: 12

    readonly property url arrowDown: "qrc:/icons/arr-down.png"
}