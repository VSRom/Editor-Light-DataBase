import QtQuick
import QtQuick.Controls
import App
import "themed"

Item {
    id: root

    focus: true

    signal addColumnRequested()

        ThemedScrollView {
            width:  parent.width
            height: parent.height

            TableView {
                id: table

                clip:         true
                anchors {
                    fill:    parent
                    margins: 10
                }

                model: MainController.tableModel

                delegate: Rectangle {
                    implicitWidth:  150
                    implicitHeight: 35
                    color:          model.selected ? Theme.selection : (row % 2 ? Theme.tableAlt : Theme.table)

                    Text {
                        anchors.fill: parent
                        text:         model.isNull ? "NULL" : model.display
                        color:        model.selected ? Theme.selectionText : Theme.text
                    }

                    MouseArea {
                        anchors.fill:    parent
                        acceptedButtons: Qt.RightButton
                        onClicked: (mouse) => clickToTable.popup()
                    }
                }
            }
        }

    Keys.onDeletePressed: MainController.deleteSelectedRowsRequested()

    Menu {
        id: clickToTable

        MenuItem {
            text: qsTr("Add row")
            onTriggered: MainController.requestAddRow()
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Add column")
            onTriggered: root.addColumnRequested()
        }
    }
}
