import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import App
import "themed"

ColumnLayout {
    id: root
    property var fm: MainController.filtersModel
    spacing: 4

    Repeater {
        model: fm
        delegate: RowLayout {
            spacing: 4
            ThemedComboBox {
                Layout.preferredWidth: 140
                model: fm.columnNames()
                currentIndex: Math.max(0, fm.columnNames().indexOf(model.column))
                onActivated: function(i) { fm.setColumn(index, fm.columnNames()[i]) }
            }
            ThemedComboBox {
                Layout.preferredWidth: 70
                model: fm.operators()
                currentIndex: Math.max(0, fm.operators().indexOf(model.op))
                onActivated: function(i) { fm.setOp(index, fm.operators()[i]) }
            }
            ThemedTextField {
                Layout.fillWidth: true
                text: model.value
                onTextChanged: fm.setValue(index, text)
            }
            ThemedButton {
                text: "X"
                danger: true
                Layout.preferredWidth: 28
                onClicked: fm.removeRow(index)
            }
        }
    }
}