import QtQuick
import QtQuick.Controls
import App

ScrollView {
    id: control

    padding: 0

    background: Rectangle {
        radius: Theme.radius
        border.width: 1
        border.color: Theme.border
        color: Theme.input
    }

    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded

        contentItem: Rectangle {
            implicitWidth: Theme.scrollBarThickness
            radius: Theme.scrollBarThickness / 2
            color: parent.pressed ? Theme.accent : Theme.border
        }
    }

    ScrollBar.horizontal: ScrollBar {
        policy: ScrollBar.AsNeeded

        contentItem: Rectangle {
            implicitHeight: Theme.scrollBarThickness
            radius: Theme.scrollBarThickness / 2
            color: parent.pressed ? Theme.accent : Theme.border
        }
    }
}