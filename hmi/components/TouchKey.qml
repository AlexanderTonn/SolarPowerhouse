import QtQuick

Rectangle {
    id: key
    property string label: ""
    property bool emphasized: false
    signal pressed()

    radius: 10
    opacity: key.enabled ? 1.0 : 0.4
    color: pointer.pressed ? "#354452" : (key.emphasized ? "#254c3b" : "#202832")
    border.color: key.emphasized ? "#47d18c" : "#3a4652"
    border.width: 1

    Text {
        anchors.centerIn: parent
        text: key.label
        color: "#f2f4f7"
        font.pixelSize: 25
        font.bold: key.emphasized
    }

    MouseArea {
        id: pointer
        anchors.fill: parent
        enabled: key.enabled
        onClicked: key.pressed()
    }
}
