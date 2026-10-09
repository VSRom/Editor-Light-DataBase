import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import App
import "themed"

Popup {
    id: control

    property string title: ""
    property string message: ""
    property bool acceptedResult: false

    signal confirmed(bool accepted)

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    anchors.centerIn: Overlay.overlay
    width: 420
    padding: 20
    background: Rectangle {
        radius: Theme.radius
        color: Theme.panel
        border.width: 1
        border.color: Theme.border
    }

    contentItem: ColumnLayout {
        spacing: 16
        ThemedLabel {
            text: control.title
            font.bold: true
        }

        ThemedLabel {
            text: control.message
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Item {
                Layout.fillWidth: true
            }

            ThemedButton {
                text: "Нет"
                onClicked: control.finish(false)
            }
            ThemedButton {
                text: "Да"
                onClicked: control.finish(true)
            }
        }
    }
    function finish(accepted) {
        acceptedResult = accepted
        control.close();
    }

    onClosed: control.confirmed(control.acceptedResult)
}