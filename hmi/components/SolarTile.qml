import QtQuick

Rectangle {
    id: tile
    property string heading: ""
    property string secondaryText: ""
    property color accentColor: "#47d18c"
    property string callToAction: "Open details"
    default property alias contentData: detailColumn.data
    signal activated()

    color: "#1b2128"
    radius: 12
    border.color: pointer.containsMouse ? tile.accentColor : "#303a45"
    border.width: pointer.containsMouse ? 2 : 1
    scale: pointer.pressed ? 0.99 : 1.0
    Behavior on scale { NumberAnimation { duration: 100 } }

    Column {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 12

        Text {
            text: tile.heading
            color: "#f2f4f7"
            font.pixelSize: 24
            font.bold: true
        }
        Text {
            text: tile.secondaryText
            color: "#aeb9c5"
            font.pixelSize: 16
        }

        Column {
            id: detailColumn
            width: parent.width
            anchors.left: parent.left
            spacing: 8
        }

        Item { width: 1; height: 1 }

        Row {
            spacing: 8
            Text {
                text: tile.callToAction
                color: tile.accentColor
                font.pixelSize: 15
                font.bold: true
            }
            Text {
                text: "→"
                color: tile.accentColor
                font.pixelSize: 18
            }
        }
    }

    MouseArea {
        id: pointer
        anchors.fill: parent
        hoverEnabled: true
        onClicked: tile.activated()
    }
}
