import QtQuick

Rectangle {
    id: eventRow
    property string timestamp: ""
    property string severity: "INFO"
    property string title: ""
    property string detail: ""
    property string component: ""
    property var localization: null

    function severityLabel() {
        if (!localization) return eventRow.severity
        if (eventRow.severity === "INFO") return localization.text("messages.info")
        if (eventRow.severity === "WARNING") return localization.text("messages.warning")
        if (eventRow.severity === "CRITICAL") return localization.text("messages.critical")
        return eventRow.severity
    }

    function componentLabel() {
        if (!localization) return eventRow.component
        if (eventRow.component === "Core") return localization.text("messages.core")
        if (eventRow.component === "HMI") return localization.text("messages.hmi")
        if (eventRow.component === "HMI IPC") return localization.text("messages.hmi.ipc")
        return eventRow.component
    }

    width: parent ? parent.width : 0
    height: 92
    radius: 9
    color: "#1b2128"
    border.color: "#303a45"
    border.width: 1

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 5
        radius: 2
        color: eventRow.severity === "CRITICAL" ? "#e06262"
            : eventRow.severity === "WARNING" ? "#e5b85c" : "#5ca9e6"
    }

    Column {
        anchors.left: parent.left
        anchors.leftMargin: 22
        anchors.right: parent.right
        anchors.rightMargin: 18
        anchors.verticalCenter: parent.verticalCenter
        spacing: 5
        Item {
            width: parent.width
            height: 22
            Text {
                id: titleText
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                width: Math.max(1, parent.width - severityText.implicitWidth - timestampText.implicitWidth - 36)
                text: eventRow.title
                color: "#f2f4f7"
                font.pixelSize: 17
                font.bold: true
                elide: Text.ElideRight
            }
            Text {
                id: severityText
                anchors.left: titleText.right
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                text: eventRow.severityLabel()
                color: eventRow.severity === "CRITICAL" ? "#e06262" : eventRow.severity === "WARNING" ? "#e5b85c" : "#5ca9e6"
                font.pixelSize: 13
                font.bold: true
            }
            Text {
                id: timestampText
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: eventRow.timestamp
                color: "#8f9aa7"
                font.pixelSize: 13
            }
        }
        Text { text: eventRow.detail; color: "#c8d0d9"; font.pixelSize: 14; elide: Text.ElideRight; width: parent.width }
        Text { text: eventRow.componentLabel(); color: "#8f9aa7"; font.pixelSize: 12 }
    }
}
