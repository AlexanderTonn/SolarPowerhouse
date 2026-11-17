import QtQuick

Item {
    id: dashboard
    property var weatherService: null
    property var messageModel: null
    property var backendSupervisor: null
    property var modbusTelemetry: null
    property var activeAlarms: null
    property var localization: null
    property string softwareVersion: ""
    signal openIslandRequested()
    signal openGridRequested()
    signal openSettingsRequested()
    signal menuRequested()

    function value(key, decimals, unit) {
        if (!dashboard.modbusTelemetry || !dashboard.modbusTelemetry.connected) return "--"
        var raw = dashboard.modbusTelemetry.values[key]
        if (raw === undefined || raw === null) return "--"
        return Number(raw).toFixed(decimals) + (unit === "" ? "" : " " + unit)
    }

    function statusColor() {
        return dashboard.backendSupervisor && dashboard.backendSupervisor.state === "Running"
            && dashboard.modbusTelemetry && dashboard.modbusTelemetry.connected ? "#47d18c" : "#e5b85c"
    }

    Rectangle { anchors.fill: parent; color: "#101317" }

    AppHeader {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        weatherService: dashboard.weatherService
        backendSupervisor: dashboard.backendSupervisor
        softwareVersion: dashboard.softwareVersion
        localization: dashboard.localization
        activeAlarms: dashboard.activeAlarms
        onMenuRequested: dashboard.menuRequested()
    }

    Rectangle {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: "#303a45"
    }

    Flickable {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 24
        anchors.topMargin: 18
        contentWidth: width
        contentHeight: contentColumn.height
        clip: true

        Column {
            id: contentColumn
            width: parent.width
            spacing: 18

            Text {
                text: dashboard.localization.text("page.dashboard")
                color: "#f2f4f7"
                font.pixelSize: 28
                font.bold: true
            }
            Text {
                text: dashboard.localization.text("page.dashboard.description")
                color: "#8f9aa7"
                font.pixelSize: 16
            }

            Grid {
                id: tileGrid
                width: contentColumn.width
                columns: dashboard.width >= 1180 ? 3 : dashboard.width >= 760 ? 2 : 1
                spacing: 20
                property int cardColumns: columns
                property real cardWidth: (width - (cardColumns - 1) * spacing) / cardColumns
                property real cardHeight: Math.max(220, Math.min(300, dashboard.height - header.height - 250))

                SolarTile {
                    width: tileGrid.cardWidth
                    height: tileGrid.cardHeight
                    heading: dashboard.localization.text("page.dashboard.island")
                    secondaryText: dashboard.localization.text("page.dashboard.island.description")
                    accentColor: "#47d18c"
                    callToAction: dashboard.localization.text("page.dashboard.island.open")
                    onActivated: dashboard.openIslandRequested()
                    Row {
                        spacing: 28
                        ValueDisplay { label: dashboard.localization.text("page.dashboard.battery"); value: dashboard.value("battery_voltage_v", 1, "V") }
                        ValueDisplay { label: dashboard.localization.text("page.dashboard.pv.power"); value: dashboard.value("charging_power_w", 0, "W") }
                        ValueDisplay { label: dashboard.localization.text("page.dashboard.charge.current"); value: dashboard.value("charging_current_a", 2, "A") }
                    }
                }

                SolarTile {
                    width: tileGrid.cardWidth
                    height: tileGrid.cardHeight
                    heading: dashboard.localization.text("page.dashboard.grid")
                    secondaryText: dashboard.localization.text("page.dashboard.grid.description")
                    accentColor: "#5ca9e6"
                    callToAction: dashboard.localization.text("page.dashboard.grid.open")
                    onActivated: dashboard.openGridRequested()
                    Row {
                        spacing: 28
                        ValueDisplay { label: dashboard.localization.text("page.dashboard.microinverter"); value: "--- W" }
                        ValueDisplay { label: dashboard.localization.text("page.dashboard.ac.production"); value: "-- kWh" }
                        ValueDisplay { label: dashboard.localization.text("page.dashboard.status"); value: "---" }
                    }
                }

                Rectangle {
                    width: tileGrid.cardWidth
                    height: tileGrid.cardHeight
                    radius: 12
                    color: "#1b2128"
                    border.color: dashboard.statusColor()
                    border.width: 1

                    Column {
                        anchors.fill: parent
                        anchors.margins: 24
                        spacing: 12

                        Text {
                            text: dashboard.localization.text("page.dashboard.system.status")
                            color: "#f2f4f7"
                            font.pixelSize: 24
                            font.bold: true
                        }
                        Text {
                            text: dashboard.localization.text("page.dashboard.system.description")
                            color: "#aeb9c5"
                            font.pixelSize: 16
                        }
                        Rectangle {
                            width: parent.width
                            height: 48
                            radius: 8
                            color: "#202832"
                            Text {
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                anchors.rightMargin: 14
                                text: dashboard.backendSupervisor && dashboard.backendSupervisor.state === "Running"
                                    ? dashboard.localization.text("page.dashboard.backend.connected")
                                    : dashboard.localization.text("page.dashboard.backend.offline")
                                color: dashboard.backendSupervisor && dashboard.backendSupervisor.state === "Running" ? "#47d18c" : "#e5b85c"
                                font.pixelSize: 16
                                font.bold: true
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                        Text {
                            width: parent.width
                            text: dashboard.modbusTelemetry && dashboard.modbusTelemetry.connected
                                ? dashboard.localization.text("page.dashboard.communication.connected")
                                : dashboard.localization.text("page.dashboard.communication.offline")
                            color: dashboard.modbusTelemetry && dashboard.modbusTelemetry.connected ? "#47d18c" : "#e06262"
                            font.pixelSize: 15
                        }
                        Row {
                            spacing: 26
                            Text { text: dashboard.localization.text("page.dashboard.mode") + "\nAUTO"; color: "#c8d0d9"; font.pixelSize: 16 }
                            Text { text: dashboard.localization.text("page.dashboard.path") + "\n---"; color: "#c8d0d9"; font.pixelSize: 16 }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: dashboard.openSettingsRequested()
                    }
                }
            }

            Rectangle {
                width: contentColumn.width
                height: 42
                color: "transparent"
                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: dashboard.localization.text("page.dashboard.configure")
                    color: "#8f9aa7"
                    font.pixelSize: 15
                }
                MouseArea { anchors.fill: parent; onClicked: dashboard.openSettingsRequested() }
            }
        }
    }
}
