import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import App
import "themed"

Popup {
    id: control

    property string title: ""
    property var items: []
    property bool ok: false

    signal chosen(string text, bool ok)

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    anchors.centerIn: Overlay.overlay
    width: 360
    padding: 20

    background: Rectangle {
        radius: Theme.radius
        color: Theme.panel
        border.width: 1
        border.color: Theme.border
    }

    contentItem: ColumnLayout {
        spacing: 12
        ThemedLabel {
            text: control.title
            font.bold: true
        }

        ThemedComboBox {
            id: cb

            Layout.fillWidth: true
            model: control.items
            Component.onCompleted: cb.currentIndex = 0
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Item {
                Layout.fillWidth: true
            }

            ThemedButton {
                text: "Отмена"
                onClicked: control.finish(false)
            }
            ThemedButton {
                text: "OK"
                onClicked: control.finish(true)
            }
        }
    }

    function finish(accepted) {
        control.ok = accepted
        control.close()
    }
    onClosed: control.chosen(cb.currentText, control.ok)
}