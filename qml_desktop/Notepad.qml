import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import App
import "themed"

ColumnLayout {
    id: root

    spacing: 4

    ThemedComboBox {
        Layout.fillWidth: true
        model: MainController.fonts
        currentIndex: Math.max(0, MainController.fonts.indexOf(MainController.noteFont))
        onActivated: function(i) {
            MainController.setUserNoteFont(MainController.fonts[i])
        }
    }

    ThemedScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        TextArea {
            id: ta

            text: MainController.noteText
            wrapMode: TextArea.Wrap
            font.family: MainController.noteFont
            font.pointSize: MainController.noteSize
            onTextChanged: {
                if (ta.activeFocus || ta.pressed) MainController.setUserNoteText(text)
            }
            background: null
        }
    }
}