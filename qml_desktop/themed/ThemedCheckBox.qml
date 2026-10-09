import QtQuick
import QtQuick.Controls
import App

CheckBox {
    id: control

    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    spacing: 8

    contentItem: Text {
        text: control.text
        font: control.font
        color: Theme.text
        verticalAlignment: Text.AlignVCenter
    }

    indicator: Rectangle {
        implicitWidth: 16
        implicitHeight: 16
        radius: 3
        border.width: 1
        border.color: control.checked ? Theme.accentDark : Theme.border
        color: control.checked ? Theme.accent : Theme.input

        Text {
            anchors.centerIn: parent
            visible: control.checked
            text: "✓"
            color: Theme.selectionText
            font.pixelSize: 12
        }
    }
}