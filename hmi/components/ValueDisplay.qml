import QtQuick

Column {
    property string label: ""
    property string value: "--"
    spacing: 2

    Text {
        text: label
        color: "#8f9aa7"
        font.pixelSize: 15
    }
    Text {
        text: value
        color: "#f2f4f7"
        font.pixelSize: 21
    }
}
