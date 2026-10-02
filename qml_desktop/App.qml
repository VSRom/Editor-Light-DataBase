import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts 
import App
import "themed"  

ApplicationWindow {
    id: app

    visible: true
    width: 1440; height: 900
    minimumWidth: 1280; minimumHeight: 860
    title: "Manager DataBase"
    color: Theme.window

    property bool connected: false
    property bool forceClose: false

    onClosing: function(event) {
        if (app.forceClose) { app.forceClose = false; return }
        if (MainController.noteDirty) { event.ignore(); noteClose.open() }
    }

    Loader {
        id: pageLoader

        anchors.fill: parent
        sourceComponent: app.connected ? mainComponent : connectionComponent
    }

    Component { id: connectionComponent; ConnectionWindow { } }
    Component { id: mainComponent;       MainWindow { } }

    Connections {
        target: ConnectionController

        function onConnected(driver, dbType, host, port, login, password, dbPath) {
            MainController.beginSession(driver, dbType, host, port, login, password, dbPath);
            app.connected = true;
        }

        function onConnectionFailed(error) { app.connected = false; }
    }

    Connections {
        target: MainController

        function onSessionFailed(error) {
            app.connected = false;
            ConnectionController.appendLogMessage("Ошибка worker: " + error);
        }
    }

    Popup {
        id: noteClose

        modal: true; focus: true
        closePolicy: Popup.NoAutoClose
        anchors.centerIn: Overlay.overlay
        width: 420; padding: 20
        background: Rectangle { radius: Theme.radius; color: Theme.panel; border.width: 1; border.color: Theme.border }
        contentItem: ColumnLayout {
            spacing: 16
            ThemedLabel { text: "Заметки изменены"; font.bold: true }
            ThemedLabel { text: "Сохранить изменения в заметках перед выходом?"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            RowLayout {
                Layout.fillWidth: true; spacing: 8
                Item { Layout.fillWidth: true }
                ThemedButton { text: "Отмена"; onClicked: noteClose.close() }
                ThemedButton { text: "Не сохранять"; danger: true
                    onClicked: { noteClose.close(); app.forceClose = true; app.close() } }
                ThemedButton { text: "Сохранить"
                    onClicked: { MainController.saveNote(); noteClose.close(); app.forceClose = true; app.close() } }
            }
        }
    }
}