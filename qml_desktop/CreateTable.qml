import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import App
import "themed"

Popup {
    id: control

    property var types: []
    property var rows: [{ name: "", type: "" }]

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape
    anchors.centerIn: Overlay.overlay
    width: 760
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
            text: qsTr("Create table")
            font.bold: true
        }

        RowLayout {
            spacing: 6
            ThemedLabel {
                text: qsTr("Table name:")
            }

            ThemedTextField {
                id: nameField

                Layout.fillWidth: true
                placeholderText: qsTr("Input field")

            }
        }
        Flickable {
            Layout.fillWidth: true
            Layout.preferredHeight: 420
            contentHeight: col.implicitHeight
            clip: true

            ColumnLayout {
                id: col

                width: parent.width
                spacing: 6

                Repeater {
                    model: control.rows
                    delegate: RowLayout {
                        spacing: 6

                        ThemedTextField {
                            Layout.preferredWidth: 220
                            placeholderText: qsTr("Column name")
                            text: modelData.name
                            onTextChanged: control.rows[index].name = text
                        }

                        ThemedComboBox {
                            Layout.preferredWidth: 200
                            editable: true
                            model: control.types
                            currentIndex: control.types.indexOf(modelData.type)
                            onActivated: function(i){ control.rows[index].type = control.types[i] }
                            onEditTextChanged: control.rows[index].type = editText
                        }
                        ThemedButton {
                            text: qsTr("X")
                            danger: true
                            Layout.preferredWidth: 28

                            onClicked: {
                                if (control.rows.length <= 1) {
                                    MainController.showError(qsTr("Cannot delete the last row"))
                                    return
                                }
                                control.rows = control.rows.filter(function(_, i){ return i !== index })
                            }
                        }
                    }
                }
            }
        }
        RowLayout {
            spacing: 8
            ThemedButton {
                text: qsTr("+ Column")
                onClicked: control.rows = control.rows.concat([{ name: "", type: (control.types[0] || "") }]) }

            Item {
                Layout.fillWidth: true
            }

            ThemedButton {
                text: qsTr("Cancel")
                onClicked: control.close()
            }

            ThemedButton {
                text: qsTr("OK")
                onClicked: control.submit()
            }
        }
    }

    function submit() {
        var nm = nameField.text.trim()
        if (nm.length === 0) {
            MainController.showError(qsTr("Enter table name"))
            return
        }
        var cols = []

        for (var i = 0; i < control.rows.length; ++i) {
            var c = control.rows[i]
            var cn = c.name.trim()
            if (cn.length === 0) {
                MainController.showError(qsTr("Column name #%1 is empty").arg(i + 1))
                return
            }
            if (cn.toLowerCase() === "id") {
                MainController.showError(qsTr("Column 'id' is created automatically"))
                return
            }
            if (c.type.trim().length === 0) {
                MainController.showError(qsTr("Type for '%1' is not selected").arg(cn))
                return
            }
            cols.push({name: cn, type: c.type.trim() })
        }
        MainController.createTableFromDraft({ tableName: nm, columns: cols })
        control.close()
    }
}