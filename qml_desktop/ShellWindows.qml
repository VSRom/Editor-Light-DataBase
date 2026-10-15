import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import App
import "themed"

Item {
    id: root

    implicitWidth:  1440
    implicitHeight: 900

    ColumnLayout {
        anchors.fill: parent

        TabBar {
            id: bar

            Layout.fillWidth: true
            position:         TabBar.Header

            Repeater {
                model: [qsTr("Tables"), qsTr("Console SQL"), qsTr("Services")]

                TabButton {
                    background: Rectangle {
                        color: index == bar.currentIndex ? Theme.selection : Theme.header
                    }

                    text:  modelData
                }
            }
        }

        StackLayout {
            id: stackBar

            Layout.fillWidth:  true
            Layout.fillHeight: true
            currentIndex:      bar.currentIndex

            Item {
                id: tabTables

                MainWindow {
                anchors.fill: parent
                }
            }

            Item {
                id: tabConsoleSQL

                ThemedLabel {
                    text:             qsTr("Empty for now, waiting ConsoleSQL")
                    Layout.fillWidth: true
                    anchors.centerIn: Overlay.overlay
                    font.pointSize:   32
                }
            }

            Item {
                id: tabServices

                ThemedLabel {
                    text:             qsTr("Empty for now, waiting Services")
                    Layout.fillWidth: true
                    anchors.centerIn: Overlay.overlay
                    font.pointSize:   32
                }
            }
        }
    }
}