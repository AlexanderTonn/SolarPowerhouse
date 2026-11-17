import QtQuick

Rectangle {
    id: row
    property string label: ""
    property string value: "--"
    property string unit: ""

    width: 420
    height: 68
    radius: 8
    color: "#1b2128"
    border.color: "#303a45"

    Column {
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - 170
        spacing: 2
        Text {
            text: row.label
            color: "#d6dde5"
            font.pixelSize: 18
            font.bold: true
            elide: Text.ElideRight
            width: parent.width
        }
    }

    Text {
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        text: row.value
        color: "#f2f4f7"
        font.pixelSize: 19
        font.bold: true
        horizontalAlignment: Text.AlignRight
        width: 145
        elide: Text.ElideRight
    }
}
