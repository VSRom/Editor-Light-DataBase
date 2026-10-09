import QtQuick
import QtQuick.Controls
import App

Button {
    id: control

    padding: 6
    leftPadding: 12
    rightPadding: 12
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize

    property bool danger: false

    background: Rectangle {
        radius: Theme.radius
        border.width: 1
        border.color: {
            if (!control.enabled)
                return Theme.border

            if (control.down)
                return Theme.accentDark

            if (control.hovered)
                return Theme.accent

            return Theme.buttonBorder
        }
        color: {
            if (!control.enabled)
                return Theme.panel

            if (control.down)
                return Theme.pressed

            if (control.hovered)
                return Theme.accentDark

            return Theme.buttonBg
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        color: {
            if (!control.enabled)
                return Theme.textDisabled

            if (control.danger) {
                if (control.down)
                    return Theme.dangerPressed

                if (control.hovered)
                    return Theme.dangerHover

                return Theme.danger
            }

            if (control.hovered || control.down)
                return Theme.selectionText

            return Theme.text
        }
    }
}