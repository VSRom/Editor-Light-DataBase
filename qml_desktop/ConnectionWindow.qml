import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import App
import "themed"

Item {
    id: root

    property string placeholderConfigName: "Выберите конфигурационные настройки"
    property string pendingSaveName:       ""
    property var controller:               ConnectionController

    function updateConfigIndex() {
        if (!controller.currentConfig.length) {
            configCombo.currentIndex = 0
            return;
        }

        const idx = configCombo.find(controller.currentConfig)
        configCombo.currentIndex = idx >= 0 ? idx : 0
    }

    Component.onCompleted: updateConfigIndex()

    Connections {
        target: controller

        function onConfigsChanged()       { root.updateConfigIndex(); }
        function onCurrentConfigChanged() { root.updateConfigIndex(); }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 25

        GridLayout {
            columns: 4
            rowSpacing: 25
            columnSpacing: 5

            ThemedLabel {
                text: "DataBase:"
            }

            ThemedComboBox {
                Layout.columnSpan: 3
                model: ["SQLite", "MySQL", "PostgreSQL", "Access", "Oracle"]
                currentIndex: controller.dbTypeIndex
                onActivated: function(index) {
                    controller.dbTypeIndex = index;
                }
            }

            ThemedLabel {
                text: "Address:"
            }

            ThemedTextField {
                Layout.columnSpan: controller.isRemote ? 1 : 3
                placeholderText: controller.isRemote ? "IP или Hostname..." : "Путь к файлу БД..."
                text: controller.address
                onTextChanged: controller.address = text
            }

            ThemedLabel {
                text: "Port:"
                visible: controller.isRemote
            }

            ThemedTextField {
                visible: controller.isRemote
                placeholderText: "Port..."
                text: controller.port
                onTextChanged: controller.port = text
            }

            ThemedLabel {
                text: "Login:"
                visible: controller.isRemote
            }

            ThemedTextField {
                visible: controller.isRemote
                text: controller.login
                onTextChanged: controller.login = text
            }

            ThemedLabel {
                text: "Password:"
                visible: controller.isRemote
            }

            ThemedTextField {
                visible: controller.isRemote
                echoMode: TextInput.Password
                text: controller.password
                onTextChanged: controller.password = text
            }

            ThemedCheckBox {
                Layout.columnSpan: 4
                text: "Сетевое подключение"
                checked: controller.isRemote
                onToggled: controller.isRemote = checked
            }

            ThemedComboBox {
                id: configCombo

                Layout.columnSpan: 4
                model: [root.placeholderConfigName].concat(controller.configs)

                onActivated: function(index) {
                    if (index <= 0) {
                        controller.currentConfig = ""
                        return
                    }

                    const name = configCombo.model[index];

                    controller.currentConfig = name;
                    controller.loadConfig(name);
                }
            }
        }

        RowLayout {
            spacing: 8

            ThemedButton {
                text: "Очистка ввода"
                onClicked: controller.resetForm()
            }

            ThemedButton {
                text: "Сохранение конфигурации"
                onClicked: root.saveRequested()
            }

            ThemedButton {
                text: "Проверка соединения"
                onClicked: controller.checkConnection()
            }

            ThemedButton {
                text: "Соединение"
                onClicked: controller.requestConnect()
            }
        }

        ThemedScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TextArea {
                readOnly: true
                text: controller.logText
                wrapMode: TextArea.Wrap
                color: Theme.text
                font.family: Theme.fontFamily
                font.pointSize: Theme.fontSize
                background: null
            }
        }
    }

    function saveRequested() {
        const name = configCombo.currentIndex > 0 ? configCombo.currentText : ""
        if (name === "" || name === root.placeholderConfigName) {
            inputNew.title = "Новая конфигурация"
            inputNew.label = "Введите имя"
            inputNew.defaultValue = ""
            inputNew.open()
            return
        }
        root.pendingSaveName   = name
        confirmOverwrite.title = "Перезапись"
        confirmOverwrite.message = qsTr("Перезаписать конфигурацию '%1'?").arg(name);
        confirmOverwrite.open()
    }

    ConfirmPopup {
        id: confirmOverwrite

        onConfirmed: function(accepted) {
            if (accepted)
                controller.saveConfig(root.pendingSaveName)
        }
    }

    InputPopup {
        id: inputNew

        onAccepted: function(text, ok) {
            const trimmed = text.trim()

            if (ok && trimmed.length > 0)
                controller.saveConfig(trimmed)
        }
    }
}