import QtQuick

Item {
    id: header
    property string pageTitle: "SOLAR POWERHOUSE"
    property string softwareVersion: ""
    property bool showBack: false
    property var weatherService: null
    property var backendSupervisor: null
    property var localization: null
    property var activeAlarms: null
    signal menuRequested()
    signal backRequested()

    height: 72 + (header.activeAlarms && header.activeAlarms.count > 0 ? 64 : 0)

    Rectangle {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 72
        color: "#101317"
    }

    Rectangle {
        id: backButton
        visible: header.showBack
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.verticalCenter: topBar.verticalCenter
        width: 48
        height: 48
        radius: 10
        color: backPointer.containsMouse ? "#28323c" : "#1b2128"

        Text {
            anchors.centerIn: parent
            text: "‹"
            color: "#f2f4f7"
            font.pixelSize: 34
        }

        MouseArea {
            id: backPointer
            anchors.fill: parent
            hoverEnabled: true
            onClicked: header.backRequested()
        }
    }

    Rectangle {
        id: menuButton
        anchors.left: parent.left
        anchors.leftMargin: header.showBack ? 88 : 24
        anchors.verticalCenter: topBar.verticalCenter
        width: 48
        height: 48
        radius: 10
        color: menuPointer.containsMouse ? "#28323c" : "#1b2128"

        Text {
            anchors.centerIn: parent
            text: "☰"
            color: "#f2f4f7"
            font.pixelSize: 25
        }

        MouseArea {
            id: menuPointer
            anchors.fill: parent
            hoverEnabled: true
            onClicked: header.menuRequested()
        }
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: header.showBack ? 152 : 88
        anchors.right: headerControls.left
        anchors.rightMargin: 18
        anchors.verticalCenter: topBar.verticalCenter
        text: header.pageTitle
        color: "#f2f4f7"
        font.pixelSize: 22
        font.bold: true
        elide: Text.ElideRight
    }

    Row {
        id: headerControls
        anchors.right: parent.right
        anchors.rightMargin: 24
        anchors.verticalCenter: topBar.verticalCenter
        height: 42
        spacing: 12

        StatusIndicator {
            width: 132
            anchors.verticalCenter: parent.verticalCenter
            healthy: header.backendSupervisor && header.backendSupervisor.state === "Running"
            localization: header.localization
        }

        LanguageSelector {
            width: 94
            anchors.verticalCenter: parent.verticalCenter
            languageManager: header.localization
        }

        Text {
            width: 52
            text: "v" + header.softwareVersion
            color: "#8f9aa7"
            font.pixelSize: 13
            anchors.verticalCenter: parent.verticalCenter
            horizontalAlignment: Text.AlignRight
        }

        Text {
            width: 160
            text: header.weatherService && header.weatherService.loading
                ? header.localization.text("weather.updating")
                : (header.weatherService && header.weatherService.available
                    ? header.localization.text("weather.prefix") + header.localization.weatherSummary(header.weatherService)
                    : header.localization.text("weather.unavailable"))
            color: header.weatherService && header.weatherService.available ? "#c8d0d9" : "#8f9aa7"
            font.pixelSize: 14
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideRight
        }

        Text {
            id: clock
            width: 130
            color: "#aeb9c5"
            font.pixelSize: 14
            text: Qt.formatDateTime(new Date(), header.localization.dateTimeFormat)
            horizontalAlignment: Text.AlignRight
            anchors.verticalCenter: parent.verticalCenter
        }

        Timer {
            interval: 1000
            repeat: true
            running: true
            onTriggered: clock.text = Qt.formatDateTime(new Date(), header.localization.dateTimeFormat)
        }
    }

    ActiveAlarmBanner {
        anchors.top: topBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        alarms: header.activeAlarms
        localization: header.localization
    }
}
