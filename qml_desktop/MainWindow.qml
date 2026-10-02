import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import App
import "themed"

Item {
    id: root
    property var m: MainController
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
                    placeholderText: "Search..."
                    text: m.searchText
                    onTextChanged: m.searchText = text
                    onAccepted: m.applySearch()
                }
                ThemedButton { text: "+ Фильтр"; onClicked: m.addFilter() }
            }
            FilterPanel { visible: m.filtersModel.count > 0 }

            DataTable {
            Layout.fillWidth: true
            Layout.fillHeight: true

            onAddColumnRequested: addColNameInput.open()
            }

            RowLayout {
                spacing: 8
                ThemedButton { text: "◀"; enabled: m.canPrev; onClicked: m.prevPage() }
                ThemedLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: "Страница " + (m.currentPage + 1) + " из " + m.totalPages + "  (всего " + m.totalRows + ")"
                }
                ThemedButton { text: "▶"; enabled: m.canNext; onClicked: m.nextPage() }
            }

           Console { Layout.fillWidth: true; Layout.preferredHeight: 92 }
        }

        ColumnLayout {
            Layout.preferredWidth: 360
            spacing: 8
            GridLayout {
                columns: 2
                ThemedButton { text: "Объединить\nтаблицы"; onClicked: m.requestMergeTables(tableList.selectedNames()) }
                ThemedButton { text: "Создать\nтаблицу";     onClicked: m.requestCreateTable() }
                ThemedButton { text: "Переименовать\nтаблицу"; onClicked: root.startRename(tableList.currentName) }
                ThemedButton { text: "Удалить\nтаблицу";    onClicked: root.startDrop(tableList.currentName) }
            }
            Notepad { Layout.preferredHeight: 220 }
            TableList { id: tableList; Layout.fillHeight: true; Layout.fillWidth: true }
        }
    }

    ConfirmPopup { id: confirmPop; property int rid: 0
        onConfirmed: function(acc) { m.confirmResult(rid, acc) } }
    InputPopup { id: inputPop; property int rid: 0
        onAccepted: function(t, ok) { m.inputResult(rid, t, ok) } }

    Connections {
        target: m
        function onOpenCreateDialog(types){ createDlg.types = types; createDlg.open() }
        function onOpenMergeDialog(cols){ mergeDlg.columnsPerTable = cols; mergeDlg.open() }
        function onOpenAddRowDialog(cols){ addRowDlg.columns = cols; addRowDlg.open() }
        function onOpenAddColDialog(types){ addColChoice.items = types; addColChoice.open() }
        function onAskConfirm(title, msg, rid) { confirmPop.title = title; confirmPop.message = msg; confirmPop.rid = rid; confirmPop.open() }
        function onAskInput(title, label, def, rid) { inputPop.title = title; inputPop.label = label; inputPop.defaultValue = def; inputPop.rid = rid; inputPop.open() }
    }

    function startRename(name) {
        if (!name) { m.showError("Таблица не выбрана"); return }
        renameInput.targetName = name; renameInput.defaultValue = ""; renameInput.open()
    }
    function startDrop(name) {
        if (!name) { m.showError("Таблица не выбрана"); return }
        dropConfirm.tgt = name
        dropConfirm.message = qsTr("Все данные таблицы '%1' удалятся! Вы уверены?").arg(name)
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
    onChosen: function(t, ok){ if (ok && t.length) m.addColumnFromDraft(root.pendingAddColName, t) }
    }

InputPopup    {
    id: addColNameInput
    title: "Новый столбец"; label: "Имя колонки"
    onAccepted: function(t, ok){
        if (ok && t.trim().length){
        root.pendingAddColName = t.trim();
        m.requestAddColumn(root.pendingAddColName)
        }
    }
}
    
    InputPopup { id: renameInput; property string targetName: ""
        title: "Новое имя таблицы"; label: "Ввод"
        onAccepted: function(t, ok) {
            if (!ok || t.trim().length === 0) return
            renameConfirm.oldName = targetName; renameConfirm.newName = t.trim()
            renameConfirm.message = qsTr("Имя '%1' изменится на '%2'. Вы уверены?").arg(targetName).arg(t.trim())
            renameConfirm.open()
        }
    }
    ConfirmPopup { id: renameConfirm; property string oldName: ""; property string newName: ""
        title: "Переименование"
        onConfirmed: function(acc) { if (acc) m.renameTable(oldName, newName); else m.showInfo("Операция отменена") }
    }
    ConfirmPopup { id: dropConfirm; property string tgt: ""
        title: "Предупреждение"
        onConfirmed: function(acc) { if (acc) m.dropTable(tgt); else m.showInfo("Операция отменена") }
    }
}