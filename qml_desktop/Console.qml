import QtQuick
import QtQuick.Layouts
import App
import "themed"

Rectangle {
    id: root

    radius: Theme.radius
    border.width: 1
    border.color: Theme.border
    color: Theme.input

    property var m: MainController

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4

        ThemedLabel {
            Layout.fillWidth: true
            elide: Text.ElideRight
            opacity: 0.85
            text: qsTr("Table: %1   |   Page %2/%3   |   Total: %4")
                    .arg(m.currentTable.length ? m.currentTable : "—")
                    .arg(m.currentPage + 1)
                    .arg(m.totalPages)
                    .arg(m.totalRows)

        }
        ThemedLabel {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.accent
            text: m.statusText
        }
        ThemedLabel {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            visible: m.errorMessage.length > 0
            color: Theme.danger
            text: m.errorMessage
        }
    }
}