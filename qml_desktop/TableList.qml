import QtQuick
import QtQuick.Controls
import App
import "themed"

ThemedScrollView {
    id: root
    property var tm: MainController.tablesModel
    property string currentName: MainController.currentTable
    function selectedNames() { return tm.selectedStrings() }

    ListView {
        model: tm
        clip: true
        delegate: ItemDelegate {
            width: ListView.view.width
            height: 28
            property int r: index
            contentItem: ThemedLabel {
                text: model.display
                elide: Text.ElideRight
                color: (MainController.currentTable === model.display || model.selected)
                       ? Theme.selectionText : Theme.text
            }
            background: Rectangle {
                color: MainController.currentTable === model.display ? Theme.selection
                       : (model.selected ? Theme.accentDark : (hovered ? Theme.accentDark : Theme.table))
            }
            MouseArea {
                anchors.fill: parent
                onClicked: function(mo) {
                    if (mo.modifiers & Qt.ControlModifier) tm.toggleSelected(r)
                    else { tm.selectOnly(r); MainController.selectTable(model.display) }
                }
            }
        }
    }
}