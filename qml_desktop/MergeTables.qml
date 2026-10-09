import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import App
import "themed"

Popup {
    id: control
    property var columnsPerTable: ({})
    property var tableNames: []
    property var checked: ({})
    property var conds: []

    readonly property var joins: ["INNER", "LEFT", "RIGHT", "FULL"]

    function colsOf(t) {
        var a = columnsPerTable[t] || []
        return a.map(function(x){
            return x.name })
    }

    function aliasOf(t) {
        return "t" + tableNames.indexOf(t)
    }

    onAboutToShow: {
        tableNames = Object.keys(columnsPerTable)

        var c = {}

        for (var i = 0; i < tableNames.length; ++i) {
            var t = tableNames[i]; c[t] = {}
            var arr = columnsPerTable[t] || []
            for (var j = 0; j < arr.length; ++j) c[t][arr[j].name] = true
        }

        checked = c
        conds = tableNames.length >= 2 ? [{ lt: tableNames[0], lc: "", op: "=", rt: tableNames[1], rc: "" }] : []
    }

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape
    anchors.centerIn: Overlay.overlay
    width: 820
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
            text: "Объединение таблиц"
            font.bold: true
        }

        RowLayout {
            spacing: 6

            ThemedLabel {
                text: "Имя новой таблицы:"
            }

            ThemedTextField {
                id: nameField
                Layout.fillWidth: true
            }

            ThemedLabel {
                text: "JOIN:"
            }

            ThemedComboBox {
                id: joinCb
                Layout.preferredWidth: 110
                model: control.joins
            }
        }

        Flickable {
            Layout.fillWidth: true
            Layout.preferredHeight: 280
            contentHeight: tabsCol.implicitHeight
            clip: true

            ColumnLayout {
                id: tabsCol

                width: parent.width
                spacing: 8
                Repeater {
                    model: control.tableNames
                    delegate: Rectangle {
                        id: tableCard

                        property string tbl: modelData

                        Layout.fillWidth: true
                        radius: Theme.radius
                        color: Theme.input
                        border.width: 1
                        border.color: Theme.border
                        implicitHeight: cardCol.implicitHeight + 16

                        ColumnLayout {
                            id: cardCol

                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.margins: 8
                            spacing: 4

                            ThemedLabel {
                                text: tableCard.tbl + "  (как " + control.aliasOf(tableCard.tbl) + ")"
                                font.bold: true
                            }

                            Repeater {
                                model: control.colsOf(tableCard.tbl)
                                delegate: ThemedCheckBox {
                                    text: modelData
                                    checked: true
                                    onToggled: {
                                        if (control.checked[tableCard.tbl])
                                            control.checked[tableCard.tbl][modelData] = checked
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        ThemedLabel {
            text: "Условия связи ON"
            font.bold: true
        }

        Repeater {
            model: control.conds

            delegate: RowLayout {
                id: condRow

                property string lTbl: modelData.lt
                property string rTbl: modelData.rt

                spacing: 4

                ThemedComboBox {
                    Layout.preferredWidth: 130
                    model: control.tableNames
                    currentIndex: Math.max(0, control.tableNames.indexOf(condRow.lTbl))
                    onActivated: function(i) {
                        condRow.lTbl = control.tableNames[i]
                        control.conds[index].lt = condRow.lTbl
                    }
                }

                ThemedLabel {
                    text: "."
                }

                ThemedComboBox {
                    Layout.preferredWidth: 130
                    model: control.colsOf(condRow.lTbl)
                    currentIndex: Math.max(0, control.colsOf(condRow.lTbl).indexOf(modelData.lc))
                    onActivated: function(i) {
                        control.conds[index].lc = model[i]
                    }
                }

                ThemedComboBox {
                    Layout.preferredWidth: 60
                    model: ["=", "<>", "<", ">", "<=", ">="]
                    currentIndex: Math.max(0, ["=", "<>", "<", ">", "<=", ">="].indexOf(modelData.op))
                    onActivated: function(i) {
                        control.conds[index].op = model[i]
                    }
                }

                ThemedComboBox {
                    Layout.preferredWidth: 130
                    model: control.tableNames
                    currentIndex: Math.max(0, control.tableNames.indexOf(condRow.rTbl))
                    onActivated: function(i) {
                        condRow.rTbl = control.tableNames[i]
                        control.conds[index].rt = condRow.rTbl
                    }
                }
                ThemedLabel {
                    text: "."
                }

                ThemedComboBox {
                    Layout.preferredWidth: 130
                    model: control.colsOf(condRow.rTbl)
                    currentIndex: Math.max(0, control.colsOf(condRow.rTbl).indexOf(modelData.rc))
                    onActivated: function(i) {
                        control.conds[index].rc = model[i]
                    }
                }

                ThemedButton {
                    text: "X"
                    danger: true
                    onClicked: control.conds = control.conds.filter(function(_, i){ return i !== index })
                }
            }
        }

        ThemedButton {
            text: "+ Добавить условие"
            onClicked: control.conds = control.conds.concat([
                { lt: control.tableNames[0] || "", lc: "", op: "=", rt: control.tableNames[1] || "", rc: "" }
            ])
        }

        RowLayout {
            spacing: 8

            Item {
                Layout.fillWidth: true
            }

            ThemedButton {
                text: "Отмена"
                onClicked: control.close()
            }

            ThemedButton {
                text: "OK"
                onClicked: control.submit()
            }
        }
    }

    function submit() {
        var nm = nameField.text.trim()

        if (nm.length === 0) { MainController.showError("Введите имя таблицы")
            return
        }

        var tables = []
        var anyCol = false

        for (var i = 0; i < tableNames.length; ++i) {
            var t = tableNames[i]
            var cols = []
            var keys = Object.keys(checked[t] || {})
            for (var k = 0; k < keys.length; ++k)
                if (checked[t][keys[k]]) {
                    cols.push(keys[k])
                    anyCol = true
                }
            tables.push({ table: t, alias: aliasOf(t), columns: cols })
        }
        if (!anyCol) {
            MainController.showError("Не выбрана ни одна колонка")
            return
        }
        var conditions = []

        for (var c = 0; c < conds.length; ++c) {
            var o = conds[c]
            if (!o.lc || !o.rc) {
                MainController.showError("Заполните колонки в условии #" + (c + 1))
                return
            }
            conditions.push({ leftTable: aliasOf(o.lt), leftCol: o.lc, op: o.op, rightTable: aliasOf(o.rt), rightCol: o.rc })
        }

        if (conditions.length === 0) {
            MainController.showError("Добавьте хотя бы одно условие ON")
            return
        }

        MainController.mergeTablesFromDraft({
            tableName: nm, join: joinCb.currentText, tables: tables, conditions: conditions
        })

        control.close()
    }
}