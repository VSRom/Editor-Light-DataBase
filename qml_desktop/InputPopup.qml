import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import App
import "themed"

Popup {
    id: control

    property string title: ""
    property string label: ""
    property string defaultValue: ""
    property string textValue: ""
    property bool okFlag: false

    signal accepted(string text, bool ok)

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
            text: control.label
        }

        ThemedTextField {
            id: field
            Layout.fillWidth: true
            text: control.defaultValue
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

    function finish(ok) {
        okFlag = ok; textValue = field.text
        control.close()
    }

    onClosed: control.accepted(control.textValue, control.okFlag)
}