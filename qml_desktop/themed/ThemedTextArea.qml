import QtQuick
import QtQuick.Controls
import App

TextArea {
    id: control

    color: Theme.text
    selectionColor: Theme.accent
    placeholderTextColor: Theme.placeholder
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    padding: 8
    background: Rectangle {
        radius: Theme.radius
        border.width: 1
        border.color: control.activeFocus ? Theme.accent : Theme.border
        color: Theme.input
    }
}