import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import App
import "themed"

Popup {
    id: control

    property var columns: []
    readonly property var formColumns: columns.filter(function(c){ return !c.isPrimaryKey })

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape
    anchors.centerIn: Overlay.overlay
    width: 480
    padding: 20
    background: Rectangle {
        radius: Theme.radius
        color: Theme.panel
        border.width: 1
        border.color: Theme.border
    }

    contentItem: ColumnLayout {
        spacing: 10
        ThemedLabel {
            text: "Добавить строку"
            font.bold: true
        }

        Flickable {
            Layout.fillWidth: true
            Layout.preferredHeight: 360
            contentHeight: col.implicitHeight
            clip: true

            ColumnLayout {
                id: col
                width: parent.width
                spacing: 6

                Repeater {
                    model: control.formColumns
                    delegate: RowLayout {
                        spacing: 6
                        ThemedLabel {
                            Layout.preferredWidth: 140
                            text: modelData.name
                            elide: Text.ElideRight
                        }

                        ThemedTextField {
                            id: val
                            Layout.fillWidth: true
                            placeholderText: modelData.type
                        }

                        Component.onDestruction: {}
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Item {
                Layout.fillWidth: true
            }

            ThemedButton {
                text: "Отмена"
                onClicked: control.close()
            }

            ThemedButton { text: "OK"
                onClicked: control.submit()
            }
        }
    }

    function submit() {
        var map = {}

        for (var i = 0; i < col.children.length; ++i) {
            var row = col.children[i]
            var field = row.children[1]

            map[control.formColumns[i].name] = field.text
        }

        MainController.addRowFromDraft(map)
        control.close()
    }
}