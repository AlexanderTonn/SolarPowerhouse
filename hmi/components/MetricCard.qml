import QtQuick

Rectangle {
    id: card
    property string label: ""
    property string value: "--"
    property string detail: "Waiting for data"
    property color accentColor: "#47d18c"

    color: "#1b2128"
    radius: 10
    border.color: "#303a45"
    border.width: 1

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 4
        radius: 2
        color: card.accentColor
    }

    Column {
        anchors.fill: parent
        anchors.margins: 18
        anchors.leftMargin: 24
        spacing: 6

        Text { text: card.label; color: "#8f9aa7"; font.pixelSize: 15 }
        Text { text: card.value; color: "#f2f4f7"; font.pixelSize: 28; font.bold: true }
        Text { text: card.detail; color: "#aeb9c5"; font.pixelSize: 14 }
    }
}
