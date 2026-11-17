import QtQuick

Item {
    id: drawerRoot
    property bool open: false
    property string currentPage: "dashboard"
    property var backendSupervisor: null
    property var userManager: null
    property var localization: null
    property string softwareVersion: ""
    signal pageRequested(string page)
    signal closeRequested()

    visible: open
    z: 100

    Rectangle {
        anchors.fill: parent
        color: "#99000000"

        MouseArea {
            anchors.fill: parent
            onClicked: drawerRoot.closeRequested()
        }
    }

    Rectangle {
        id: panel
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: Math.min(360, parent.width * 0.82)
        color: "#171c22"
        border.color: "#303a45"
        border.width: 1

        Column {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 12

            Text {
                text: "SOLAR POWERHOUSE"
                color: "#f2f4f7"
                font.pixelSize: 20
                font.bold: true
            }
            Text {
                text: drawerRoot.localization.text("nav.navigation")
                color: "#8f9aa7"
                font.pixelSize: 14
            }

            Rectangle { width: parent.width; height: 1; color: "#303a45" }

            Rectangle {
                width: parent.width
                height: 54
                radius: 8
                color: drawerRoot.currentPage === "dashboard" ? "#28323c" : "transparent"
                Text { anchors.left: parent.left; anchors.leftMargin: 16; anchors.verticalCenter: parent.verticalCenter; text: "⌂   " + drawerRoot.localization.text("nav.dashboard"); color: "#f2f4f7"; font.pixelSize: 17 }
                MouseArea { anchors.fill: parent; onClicked: drawerRoot.pageRequested("dashboard") }
            }
            Rectangle {
                width: parent.width
                height: 54
                radius: 8
                color: drawerRoot.currentPage === "island" ? "#28323c" : "transparent"
                Text { anchors.left: parent.left; anchors.leftMargin: 16; anchors.verticalCenter: parent.verticalCenter; text: "▣   " + drawerRoot.localization.text("nav.island"); color: "#f2f4f7"; font.pixelSize: 17 }
                MouseArea { anchors.fill: parent; onClicked: drawerRoot.pageRequested("island") }
            }
            Rectangle {
                width: parent.width
                height: 54
                radius: 8
                color: drawerRoot.currentPage === "grid" ? "#28323c" : "transparent"
                Text { anchors.left: parent.left; anchors.leftMargin: 16; anchors.verticalCenter: parent.verticalCenter; text: "⌁   " + drawerRoot.localization.text("nav.grid"); color: "#f2f4f7"; font.pixelSize: 17 }
                MouseArea { anchors.fill: parent; onClicked: drawerRoot.pageRequested("grid") }
            }
            Rectangle {
                width: parent.width
                height: 54
                radius: 8
                color: drawerRoot.currentPage === "settings" ? "#28323c" : "transparent"
                Text { anchors.left: parent.left; anchors.leftMargin: 16; anchors.verticalCenter: parent.verticalCenter; text: "⚙   " + drawerRoot.localization.text("nav.settings"); color: "#f2f4f7"; font.pixelSize: 17 }
                MouseArea { anchors.fill: parent; onClicked: drawerRoot.pageRequested("settings") }
            }
            Rectangle {
                width: parent.width
                height: 54
                radius: 8
                color: drawerRoot.currentPage === "setup" ? "#28323c" : "transparent"
                Text { anchors.left: parent.left; anchors.leftMargin: 16; anchors.verticalCenter: parent.verticalCenter; text: "🛠   " + drawerRoot.localization.text("nav.setup"); color: "#f2f4f7"; font.pixelSize: 17 }
                MouseArea { anchors.fill: parent; onClicked: drawerRoot.pageRequested("setup") }
            }
            Rectangle {
                width: parent.width
                height: 54
                radius: 8
                color: drawerRoot.currentPage === "messages" ? "#28323c" : "transparent"
                Text { anchors.left: parent.left; anchors.leftMargin: 16; anchors.verticalCenter: parent.verticalCenter; text: "⚠   " + drawerRoot.localization.text("nav.messages"); color: "#f2f4f7"; font.pixelSize: 17 }
                MouseArea { anchors.fill: parent; onClicked: drawerRoot.pageRequested("messages") }
            }

            Item { width: 1; height: 20 }

            Text {
                text: drawerRoot.localization.text("nav.system.status")
                color: "#8f9aa7"
                font.pixelSize: 14
            }
            Text {
                text: drawerRoot.localization.text("nav.core") + ": "
                    + (drawerRoot.backendSupervisor
                        ? (drawerRoot.backendSupervisor.state === "Running"
                            ? drawerRoot.localization.text("state.running")
                            : drawerRoot.backendSupervisor.state === "Stopped"
                                ? drawerRoot.localization.text("state.stopped")
                                : drawerRoot.backendSupervisor.state === "Error"
                                    ? drawerRoot.localization.text("state.error")
                                    : drawerRoot.backendSupervisor.state === "Unavailable"
                                        ? drawerRoot.localization.text("state.unavailable")
                                : drawerRoot.localization.text("nav.unknown"))
                        : drawerRoot.localization.text("nav.unknown"))
                    + "\n" + drawerRoot.localization.text("nav.hardware.pending")
                color: drawerRoot.backendSupervisor && drawerRoot.backendSupervisor.state === "Running" ? "#47d18c" : "#e5b85c"
                font.pixelSize: 15
                lineHeight: 1.3
            }
            Text { text: drawerRoot.localization.text("nav.version") + drawerRoot.softwareVersion; color: "#8f9aa7"; font.pixelSize: 13 }
        }
    }
}
