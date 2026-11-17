import QtQuick

Item {
    id: page
    property var weatherService: null
    property var messageModel: null
    property var backendSupervisor: null
    property var modbusTelemetry: null
    property var activeAlarms: null
    property var localization: null
    property string softwareVersion: ""
    signal backRequested()
    signal menuRequested()

    Rectangle { anchors.fill: parent; color: "#101317" }
    AppHeader {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        weatherService: page.weatherService
        backendSupervisor: page.backendSupervisor
        softwareVersion: page.softwareVersion
        localization: page.localization
        activeAlarms: page.activeAlarms
        pageTitle: page.localization.text("page.grid.title")
        showBack: true
        onBackRequested: page.backRequested()
        onMenuRequested: page.menuRequested()
    }

    Flickable {
        anchors.top: parent.top
        anchors.topMargin: 98
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 24
        contentWidth: width
        contentHeight: gridContent.height
        clip: true

        Column {
            id: gridContent
            width: parent.width
            spacing: 18
            Text { text: page.localization.text("page.grid.description"); color: "#8f9aa7"; font.pixelSize: 16 }

            Grid {
                id: gridMetrics
                width: parent.width
                columns: page.width >= 980 ? 3 : 1
                property int cardColumns: columns
                spacing: 16
                MetricCard { width: (parent.width - (gridMetrics.cardColumns - 1) * gridMetrics.spacing) / gridMetrics.cardColumns; height: 140; label: page.localization.text("page.grid.power"); value: "--- W"; detail: page.localization.text("page.grid.no.meter"); accentColor: "#5ca9e6" }
                MetricCard { width: (parent.width - (gridMetrics.cardColumns - 1) * gridMetrics.spacing) / gridMetrics.cardColumns; height: 140; label: page.localization.text("page.dashboard.ac.production"); value: "-- kWh"; detail: page.localization.text("page.grid.no.meter"); accentColor: "#47d18c" }
                MetricCard { width: (parent.width - (gridMetrics.cardColumns - 1) * gridMetrics.spacing) / gridMetrics.cardColumns; height: 140; label: page.localization.text("page.grid.connection"); value: page.localization.text("page.grid.offline"); detail: page.localization.text("page.grid.pending"); accentColor: "#e06262" }
            }

            Rectangle {
                width: parent.width
                height: 180
                radius: 10
                color: "#1b2128"
                border.color: "#303a45"
                Text { anchors.left: parent.left; anchors.top: parent.top; anchors.margins: 20; text: page.localization.text("page.grid.history"); color: "#f2f4f7"; font.pixelSize: 20; font.bold: true }
                Text { anchors.centerIn: parent; text: page.localization.text("page.grid.not.measured"); color: "#8f9aa7"; font.pixelSize: 15 }
            }
        }
    }
}
