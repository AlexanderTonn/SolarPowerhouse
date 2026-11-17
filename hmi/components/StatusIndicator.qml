import QtQuick

Row {
    property bool healthy: false
    property var localization: null
    spacing: 8

    Rectangle {
        width: 12
        height: 12
        radius: 6
        anchors.verticalCenter: parent.verticalCenter
        color: healthy ? "#47d18c" : "#e06262"
    }
    Text {
        width: Math.max(1, parent.width - 20)
        text: localization ? (healthy ? localization.text("status.connected") : localization.text("status.unavailable")) : (healthy ? "Connected" : "Unavailable")
        color: "#c8d0d9"
        font.pixelSize: 14
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }
}
