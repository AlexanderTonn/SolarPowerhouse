import QtQuick

Rectangle {
    id: control
    property string label: ""
    property color curveColor: "#47d18c"
    property bool checked: false
    signal toggled()

    width: 220
    height: 48
    radius: 8
    color: control.checked ? "#202b34" : "#171c21"
    border.color: control.checked ? control.curveColor : "#3a4652"
    border.width: 1

    Rectangle {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        width: 18
        height: 18
        radius: 4
        color: control.checked ? control.curveColor : "transparent"
        border.color: control.curveColor
        border.width: 2
        Text {
            anchors.centerIn: parent
            text: "✓"
            visible: control.checked
            color: "#101317"
            font.pixelSize: 14
            font.bold: true
        }
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 42
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        text: control.label
        color: "#f2f4f7"
        font.pixelSize: 15
        elide: Text.ElideRight
    }

    MouseArea {
        anchors.fill: parent
        onClicked: control.toggled()
    }
}
