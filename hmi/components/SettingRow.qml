import QtQuick

Rectangle {
    id: setting
    property string label: ""
    property string value: "--"
    property string detail: ""

    width: parent ? parent.width : 0
    height: 68
    color: "transparent"

    Text {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        text: setting.label
        color: "#e5e9ee"
        font.pixelSize: 16
    }
    Column {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: 2
        Text { anchors.right: parent.right; text: setting.value; color: "#f2f4f7"; font.pixelSize: 16; font.bold: true }
        Text { anchors.right: parent.right; text: setting.detail; color: "#8f9aa7"; font.pixelSize: 13 }
    }
    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: "#303a45" }
}
