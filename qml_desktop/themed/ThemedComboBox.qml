import QtQuick
import QtQuick.Controls
import App

ComboBox {
    id: control

    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    implicitHeight: 28
    leftPadding: 8
    rightPadding: 28

    background: Rectangle {
        radius: Theme.radius
        border.width: 1
        border.color: (control.activeFocus || control.hovered) ? Theme.accent : Theme.border
        color: Theme.input
    }

    contentItem: Text {
        text: control.displayText
        font: control.font
        color: Theme.text
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.leftPadding
        rightPadding: control.rightPadding
        elide: Text.ElideRight
    }

    indicator: Image {
        source: Theme.arrowDown
        width: 16; height: 16
        x: control.width - width - 5
        anchors.verticalCenter: parent.verticalCenter
        opacity: control.enabled ? 1.0 : 0.5
    }

    delegate: ItemDelegate {
        width: control.width
        height: 28

        contentItem: Text {
            text: modelData
            font.family: Theme.fontFamily
            font.pointSize: Theme.fontSize
            color: Theme.text
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle { color: highlighted ? Theme.selection : Theme.panel }
    }

    popup: Popup {
        y: control.height
        width: control.width
        implicitHeight: contentItem ? contentItem.implicitHeight : 0
        padding: 1

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator { }
        }

        background: Rectangle {
            radius: Theme.radius
            border.color: Theme.border
            color: Theme.panel
        }
    }
}