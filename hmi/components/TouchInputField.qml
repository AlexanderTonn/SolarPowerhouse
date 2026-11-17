import QtQuick

Rectangle {
    id: field
    property string label: ""
    property string value: ""
    property string placeholder: "Not commissioned"
    property string unit: ""
    property bool valid: true
    property color accentColor: "#5ca9e6"
    signal editRequested()

    width: parent ? parent.width : 0
    height: 78
    color: "transparent"

    Text {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        text: field.label
        color: "#e5e9ee"
        font.pixelSize: 16
    }

    Rectangle {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: Math.min(330, parent.width * 0.52)
        height: 56
        radius: 9
        color: valuePointer.containsMouse ? "#28323c" : "#202832"
        border.color: field.valid ? (valuePointer.containsMouse ? field.accentColor : "#3a4652") : "#e06262"
        border.width: valuePointer.containsMouse ? 2 : 1

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            text: field.value === "" ? field.placeholder : field.value
            color: field.value === "" ? "#8f9aa7" : "#f2f4f7"
            font.pixelSize: 17
        }
        Text {
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            text: field.value === "" ? "✎" : field.unit + "  ✎"
            color: field.accentColor
            font.pixelSize: 16
        }

        MouseArea {
            id: valuePointer
            anchors.fill: parent
            hoverEnabled: true
            onClicked: field.editRequested()
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: "#303a45"
    }
}
