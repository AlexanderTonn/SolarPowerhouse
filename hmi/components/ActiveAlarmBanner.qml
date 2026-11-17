import QtQuick

Item {
    id: banner
    property var alarms: null
    property var localization: null
    property int rowHeight: 54
    property int visibleAlarmCount: alarms && alarms.count > 0 ? 1 : 0

    visible: visibleAlarmCount > 0
    height: visible ? rowHeight + 10 : 0

    Rectangle {
        anchors.fill: parent
        color: "#101317"
    }

    Column {
        anchors.fill: parent
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        spacing: 6

        Repeater {
            model: banner.alarms
            delegate: Rectangle {
                width: banner.width - 48
                visible: index === 0
                height: visible ? banner.rowHeight : 0
                radius: 8
                color: "#421d22"
                border.color: "#e06262"
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    spacing: 10

                    Rectangle {
                        width: 26
                        height: 26
                        radius: 13
                        color: "#e06262"
                        anchors.verticalCenter: parent.verticalCenter
                        Text {
                            anchors.centerIn: parent
                            text: "!"
                            color: "#ffffff"
                            font.pixelSize: 18
                            font.bold: true
                        }
                    }

                    Column {
                        width: Math.max(1, parent.width - 220)
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2
                        Text {
                            width: parent.width
                            text: model.title
                            color: "#ffffff"
                            font.pixelSize: 16
                            font.bold: true
                            elide: Text.ElideRight
                        }
                        Text {
                            width: parent.width
                            text: model.detail
                            color: "#f0c7ca"
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                    }

                    Text {
                        width: 170
                        text: model.timestamp
                        color: "#f0c7ca"
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.NoWrap
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }
    }
}
