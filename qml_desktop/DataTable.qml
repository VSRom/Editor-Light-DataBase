import QtQuick
import QtQuick.Controls
import App
import "themed"

Item {
    id: root

    focus: true
    property var tm: MainController.tableModel

    signal addColumnRequested()

    Column {
        anchors.fill: parent

        Rectangle {
            width: parent.width
            height: 28
            color: Theme.header

            Row {
                Repeater {
                    model: tm.cols
                    delegate: ThemedLabel {
                        width: 140
                        height: 28
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 4
                        text: tm.headerAt(index)
                        color: Theme.selectionText
                        font.bold: true
                        elide: Text.ElideRight
                    }
                }
            }
        }

        ThemedScrollView {
            width: parent.width
            height: parent.height - 28

            ListView {
                id: lv

                model: tm.rows
                clip: true

                delegate: Item {
                    width: ListView.view.width
                    height: 28
                    property int r: index

                    Row {
                        Repeater {
                            model: tm.cols

                            delegate: MouseArea {
                                width: 140
                                height: 28
                                property int c: index
                                onClicked: function(mo) {
                                    if (mo.modifiers & Qt.ControlModifier) tm.toggleSelected(r)
                                    else tm.selectOnly(r)
                                }
                                onDoubleClicked: MainController.updateCellRequested(r, c)

                                Rectangle {
                                    anchors.fill: parent
                                    border.color: Theme.gridline
                                    border.width: 1
                                    color: tm.isSelectedAt(r) ? Theme.selection : (r % 2 ? Theme.tableAlt : Theme.table)

                                    ThemedLabel {
                                        anchors.fill: parent
                                        anchors.margins: 4
                                        verticalAlignment: Text.AlignVCenter
                                        text: tm.isNullAt(r, c) ? "NULL" : tm.displayAt(r, c)
                                        color: tm.isSelectedAt(r) ? Theme.selectionText : Theme.text
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Keys.onDeletePressed: MainController.deleteSelectedRowsRequested()

    Menu {
        id: ctx
        MenuItem {
            text: "Добавить строку"
            onTriggered: MainController.requestAddRow()
        }

        MenuSeparator {}

        MenuItem {
            text: "Добавить столбец"
            onTriggered: root.addColumnRequested()
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: function(e) { ctx.popup(e.mouseX, e.mouseY) }
    }
}