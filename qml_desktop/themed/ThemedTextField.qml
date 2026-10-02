import QtQuick
import QtQuick.Controls
import App

TextField {
    id: control

    color: Theme.text
    selectionColor: Theme.accent
    placeholderTextColor: Theme.placeholder
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    leftPadding: 8; rightPadding: 8; topPadding: 4; bottomPadding: 4

    background: Rectangle {
        radius: Theme.radius
        border.width: 1
        border.color: control.activeFocus ? Theme.accent : Theme.border
        color: Theme.input
    }
}