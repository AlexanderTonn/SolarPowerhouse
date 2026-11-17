import QtQuick

Item {
    id: page
    property var messageModel: null
    property var backendSupervisor: null
    property var modbusTelemetry: null
    property var activeAlarms: null
    property var localization: null
    property string softwareVersion: ""
    property string severityFilter: "ALL"
    property string sourceFilter: "ALL"
    signal backRequested()
    signal menuRequested()

    function messageSource(component) {
        if (component === "HMI" || component === "HMI IPC") return component
        return "Core"
    }

    function matchesMessage(severity, component) {
        return (severityFilter === "ALL" || severity === severityFilter)
            && (sourceFilter === "ALL" || messageSource(component) === sourceFilter)
    }

    function severityLabel(severity) {
        if (severity === "ALL") return page.localization.text("messages.all")
        if (severity === "INFO") return page.localization.text("messages.info")
        if (severity === "WARNING") return page.localization.text("messages.warning")
        if (severity === "CRITICAL") return page.localization.text("messages.critical")
        return severity
    }

    function sourceLabel(source) {
        if (source === "ALL") return page.localization.text("messages.all")
        if (source === "Core") return page.localization.text("messages.core")
        if (source === "HMI") return page.localization.text("messages.hmi")
        if (source === "HMI IPC") return page.localization.text("messages.hmi.ipc")
        return source
    }

    function filteredEntryCount() {
        if (!messageModel) return 0
        var count = 0
        for (var index = 0; index < messageModel.count; ++index) {
            var message = messageModel.get(index)
            if (matchesMessage(message.severity, message.component)) ++count
        }
        return count
    }

    Rectangle { anchors.fill: parent; color: "#101317" }
    AppHeader {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        pageTitle: page.localization.text("page.messages.title")
        showBack: true
        backendSupervisor: page.backendSupervisor
        softwareVersion: page.softwareVersion
        localization: page.localization
        activeAlarms: page.activeAlarms
        onBackRequested: page.backRequested()
        onMenuRequested: page.menuRequested()
    }

    Column {
        anchors.top: parent.top
        anchors.topMargin: 98
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 24
        spacing: 16

        Row {
            id: titleRow
            width: parent.width
            height: 34
            Text { text: page.localization.text("messages.history"); color: "#f2f4f7"; font.pixelSize: 28; font.bold: true }
            Item { width: Math.max(1, parent.width - 360); height: 1 }
            Text {
                text: page.filteredEntryCount() + " / " + (page.messageModel ? page.messageModel.count : 0) + page.localization.text("messages.entries")
                color: "#8f9aa7"
                font.pixelSize: 15
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Text {
            id: descriptionText
            text: page.localization.text("messages.description")
            color: "#8f9aa7"
            font.pixelSize: 16
        }

        Rectangle {
            id: filterPanel
            width: parent.width
            height: 96
            radius: 9
            color: "#1b2128"
            border.color: "#303a45"

            Row {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 24

                Column {
                    width: (parent.width - 24) / 2
                    spacing: 6
                    Text { text: page.localization.text("messages.type"); color: "#8f9aa7"; font.pixelSize: 13 }
                    Row {
                        id: severityFilterRow
                        width: parent.width
                        height: 48
                        spacing: 6
                        Repeater {
                            model: ["ALL", "INFO", "WARNING", "CRITICAL"]
                            delegate: Rectangle {
                                width: (severityFilterRow.width - 3 * severityFilterRow.spacing) / 4
                                height: severityFilterRow.height
                                radius: 7
                                color: page.severityFilter === modelData ? "#254c3b" : "#202832"
                                border.color: page.severityFilter === modelData ? "#47d18c" : "#3a4652"
                                Text {
                                    anchors.centerIn: parent
                                    text: page.severityLabel(modelData)
                                    color: "#f2f4f7"
                                    font.pixelSize: 14
                                    font.bold: page.severityFilter === modelData
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: page.severityFilter = modelData
                                }
                            }
                        }
                    }
                }

                Column {
                    width: (parent.width - 24) / 2
                    spacing: 6
                    Text { text: page.localization.text("messages.source"); color: "#8f9aa7"; font.pixelSize: 13 }
                    Row {
                        id: sourceFilterRow
                        width: parent.width
                        height: 48
                        spacing: 6
                        Repeater {
                            model: ["ALL", "Core", "HMI", "HMI IPC"]
                            delegate: Rectangle {
                                width: (sourceFilterRow.width - 3 * sourceFilterRow.spacing) / 4
                                height: sourceFilterRow.height
                                radius: 7
                                color: page.sourceFilter === modelData ? "#254c3b" : "#202832"
                                border.color: page.sourceFilter === modelData ? "#47d18c" : "#3a4652"
                                Text {
                                    anchors.centerIn: parent
                                    text: page.sourceLabel(modelData)
                                    color: "#f2f4f7"
                                    font.pixelSize: 14
                                    font.bold: page.sourceFilter === modelData
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: page.sourceFilter = modelData
                                }
                            }
                        }
                    }
                }
            }
        }

        ListView {
            id: eventList
            width: parent.width
            height: Math.max(100, parent.height - titleRow.height - descriptionText.height - filterPanel.height - 4 * parent.spacing)
            spacing: 10
            clip: true
            model: page.messageModel
            delegate: EventRow {
                width: eventList.width
                timestamp: model.timestamp
                severity: model.severity
                title: model.title
                detail: model.detail
                component: model.component
                localization: page.localization
                property bool included: page.matchesMessage(severity, component)
                visible: included
                height: included ? 92 : 0
            }
        }
    }
}
