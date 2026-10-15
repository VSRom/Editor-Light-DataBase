import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import App
import "themed"

Item {
    id: root

    property var mainController: MainController
    property string pendingRenameOld: ""
    property string pendingAddColName: ""

    RowLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4

            RowLayout {
                ThemedTextField {
                    Layout.fillWidth: true
                    placeholderText: qsTr("Search...")
                    text: mainController.searchText
                    onTextChanged: mainController.searchText = text
                    onAccepted: mainController.applySearch()
                }
                ThemedButton {
                    text: qsTr("+ Filter")
                    onClicked: mainController.addFilter()
                }
            }
            FilterPanel {
                visible: mainController.filtersModel.count > 0
            }

            DataTable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            onAddColumnRequested: addColNameInput.open()
            }

            RowLayout {
                spacing: 8

                ThemedButton {
                    text: qsTr("◀")
                    enabled: mainController.canPrev
                    onClicked: mainController.prevPage()
                }

                ThemedLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Page ") + (mainController.currentPage + 1) + qsTr(" from ")
                          + mainController.totalPages + qsTr("  (total ") + mainController.totalRows + ")"
                }
                ThemedButton {
                    text: qsTr("▶")
                    enabled: mainController.canNext
                    onClicked: mainController.nextPage()
                }
            }

           Console {
               Layout.fillWidth: true
               Layout.preferredHeight: 92
           }
        }

        ColumnLayout {
            Layout.preferredWidth: 360
            spacing: 8

            GridLayout {
                columns: 2
                ThemedButton {
                    text: qsTr("Merge\ntables")
                    onClicked: mainController.requestMergeTables(tableList.selectedNames())
                }

                ThemedButton {
                    text: qsTr("Create\ntable")
                    onClicked: mainController.requestCreateTable()
                }

                ThemedButton {
                    text: qsTr("Rename\ntable")
                    onClicked: root.startRename(tableList.currentName)
                }

                ThemedButton {
                    text: qsTr("Delete\ntable")
                    onClicked: root.startDrop(tableList.currentName)
                }
            }

            Notepad {
                Layout.preferredHeight: 220
            }

            TableList {
                id: tableList
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
    }

    ConfirmPopup {
        id: confirmPop

        property int rid: 0

        onConfirmed: function(acc) {
            mainController.confirmResult(rid, acc)
        }
    }

    InputPopup {
        id: inputPop

        property int rid: 0

        onAccepted: function(t, ok) {
            mainController.inputResult(rid, t, ok)
        }
    }

    Connections {
        target: mainController

        function onOpenCreateDialog(types){
            createDlg.types = types
            createDlg.open()
        }

        function onOpenMergeDialog(cols){
            mergeDlg.columnsPerTable = cols
            mergeDlg.open()
        }

        function onOpenAddRowDialog(cols){
            addRowDlg.columns = cols
            addRowDlg.open()
        }

        function onOpenAddColDialog(types){
            addColChoice.items = types
            addColChoice.open()
        }

        function onAskConfirm(title, msg, rid) {
            confirmPop.title = title
            confirmPop.message = msg
            confirmPop.rid = rid
            confirmPop.open()
        }

        function onAskInput(title, label, def, rid) {
            inputPop.title = title
            inputPop.label = label
            inputPop.defaultValue = def
            inputPop.rid = rid
            inputPop.open()
        }
    }

    function startRename(name) {
        if (!name) {
            mainController.showError(qsTr("No table selected"))
            return
        }

        renameInput.targetName = name
        renameInput.defaultValue = ""
        renameInput.open()
    }

    function startDrop(name) {
        if (!name) {
            mainController.showError(qsTr("No table selected"))
            return
        }

        dropConfirm.tgt = name
        dropConfirm.message = qsTr("All data in table '%1' will be deleted! Are you sure?").arg(name)
        dropConfirm.open()
    }

    CreateTable   {
    id: createDlg
    }

    MergeTables   {
    id: mergeDlg
    }

    AddRowDialog  {
    id: addRowDlg
    }

    ChoicePopup   {
    id: addColChoice

    onChosen: function(t, ok){
        if (ok && t.length)
            mainController.addColumnFromDraft(root.pendingAddColName, t)
        }
    }

InputPopup    {
    id: addColNameInput

    title: "Новый столбец"
    label: "Имя колонки"
    onAccepted: function(t, ok){
        if (ok && t.trim().length){
            root.pendingAddColName = t.trim();
            mainController.requestAddColumn(root.pendingAddColName)
        }
    }
}
    
    InputPopup {
        id: renameInput

        property string targetName: ""

        title: qsTr("New name table")
        label: qsTr("Enter")
        onAccepted: function(t, ok) {
            if (!ok || t.trim().length === 0)
                return

            renameConfirm.oldName = targetName; renameConfirm.newName = t.trim()
            renameConfirm.message = qsTr("The name '%1' will change to '%2'. Are you sure?").arg(targetName).arg(t.trim())
            renameConfirm.open()
        }
    }
    ConfirmPopup {
        id: renameConfirm

        property string oldName: ""
        property string newName: ""

        title: qsTr("Rename")
        onConfirmed: function(acc) {
            if (acc) mainController.renameTable(oldName, newName)
            else
                mainController.showInfo(qsTr("Operation cancelled"))
        }
    }
    ConfirmPopup {
        id: dropConfirm

        property string tgt: ""

        title: qsTr("Warning")
        onConfirmed: function(acc) {
            if (acc) mainController.dropTable(tgt)
            else
                mainController.showInfo(qsTr("Operation cancelled"))
        }
    }
}