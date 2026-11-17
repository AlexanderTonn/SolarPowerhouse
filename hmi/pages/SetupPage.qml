import QtQuick

Item {
    id: page
    property var weatherService: null
    property var messageModel: null
    property var backendSupervisor: null
    property var modbusTelemetry: null
    property var userManager: null
    property var activeAlarms: null
    property var localization: null
    property string softwareVersion: ""
    property string pendingMappingKey: ""
    property bool mappingReloadPending: false
    property string mappingReloadError: ""
    property string loginUser: ""
    property string loginPassword: ""
    signal backRequested()
    signal menuRequested()

    function logMessage(severity, title, detail, component) {
        if (!page.messageModel) return
        page.messageModel.insert(0, {
            timestamp: Qt.formatDateTime(new Date(), page.localization.dateTimeFormat + ":ss"),
            severity: severity,
            title: title,
            detail: detail,
            component: component
        })
        if (page.messageModel.count > 500) page.messageModel.remove(500)
    }

    function hasPermission(permission) {
        return page.userManager && page.userManager.hasPermission(permission)
    }

    function localizedAccessMessage() {
        if (!page.userManager) return ""
        var message = page.userManager.accessMessage
        if (message === "Commissioning access active; password policy is not enabled") return page.localization.text("setup.access.active")
        if (message === "Password authentication is not provisioned") return page.localization.text("setup.password.not.provisioned")
        if (message === "Setup cannot be locked before password policy provisioning") return page.localization.text("setup.cannot.lock")
        if (message === "Setup locked") return page.localization.text("setup.locked")
        return message
    }

    function requestMappingChange(key) {
        if (!hasPermission("modbus.mapping.change") || !page.modbusTelemetry || key === page.modbusTelemetry.mappingKey) return
        pendingMappingKey = key
        mappingReloadError = ""
        mappingReloadPending = true
    }

    function confirmMappingChange() {
        if (!hasPermission("modbus.mapping.change") || !page.modbusTelemetry || pendingMappingKey === "") return
        if (!page.modbusTelemetry.selectMapping(pendingMappingKey)) {
            mappingReloadError = page.localization.text("setup.mapping.unavailable")
            return
        }
        logMessage("WARNING", page.localization.text("messages.mapping.reload"), page.localization.text("messages.mapping.reload.detail"), "HMI")
        mappingReloadPending = false
        if (page.backendSupervisor) page.backendSupervisor.restart()
    }

    function requestBackendAction(action) {
        if (!hasPermission("backend.control") || !page.backendSupervisor) return
        var actionLabel = action === "start" ? page.localization.text("setup.start")
            : action === "stop" ? page.localization.text("setup.stop")
                : action === "restart" ? page.localization.text("setup.restart") : page.localization.text("setup.refresh")
        logMessage(action === "stop" ? "WARNING" : "INFO", page.localization.text("messages.backend.action"), actionLabel + " solar-core.service", "HMI")
        if (action === "start") page.backendSupervisor.start()
        if (action === "stop") page.backendSupervisor.stop()
        if (action === "restart") page.backendSupervisor.restart()
        if (action === "refresh") page.backendSupervisor.refresh()
    }

    function updateWeatherLocation() {
        if (page.weatherService) {
            page.weatherService.setLocation(locationInput.text)
            page.weatherService.refresh()
        }
    }

    function authenticate() {
        if (!page.userManager) return
        if (page.userManager.authenticate(loginUser, loginPassword)) {
            loginPassword = ""
            logMessage("INFO", page.localization.text("messages.setup.granted"), page.localization.text("messages.setup.granted.detail"), "HMI")
        } else {
            logMessage("WARNING", page.localization.text("messages.setup.denied"), page.localizedAccessMessage(), "HMI")
        }
    }

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
        pageTitle: page.localization.text("page.setup.title")
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
        contentHeight: setupContent.height
        clip: true

        Column {
            id: setupContent
            width: parent.width
            spacing: 18

            Text { text: page.localization.text("page.setup.description"); color: "#8f9aa7"; font.pixelSize: 16 }

            Rectangle {
                width: parent.width
                height: page.userManager && page.userManager.passwordProtectionEnabled ? 194 : 142
                radius: 10
                color: "#1b2128"
                border.color: page.userManager && page.userManager.passwordProtectionEnabled ? "#47d18c" : "#e5b85c"
                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 7
                    Text { text: page.localization.text("setup.user.access"); color: "#f2f4f7"; font.pixelSize: 20; font.bold: true }
                    Text {
                        text: page.userManager
                            ? page.localization.text("setup.user") + page.userManager.activeUser + page.localization.text("setup.role") + page.userManager.activeRole
                            : page.localization.text("setup.manager.unavailable")
                        color: "#d6dde5"
                        font.pixelSize: 15
                    }
                    Text {
                        text: page.userManager
                            ? (page.userManager.passwordProtectionEnabled ? page.localization.text("setup.password.enabled") : page.localization.text("setup.commissioning.mode"))
                            : page.localization.text("setup.access.unavailable")
                        color: page.userManager && page.userManager.passwordProtectionEnabled ? "#47d18c" : "#e5b85c"
                        font.pixelSize: 14
                    }
                    Text {
                        text: page.localizedAccessMessage()
                        color: "#8f9aa7"
                        font.pixelSize: 13
                    }
                    Row {
                        width: parent.width
                        height: 42
                        visible: page.userManager && page.userManager.passwordProtectionEnabled
                        TouchKey {
                            width: 150
                            height: 42
                            label: page.localization.text("setup.lock")
                            onPressed: page.userManager.lockSetup()
                        }
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 188
                radius: 10
                color: "#1b2128"
                border.color: "#303a45"
                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 10
                    Text { text: page.localization.text("setup.mapping"); color: "#f2f4f7"; font.pixelSize: 20; font.bold: true }
                    Text {
                        width: parent.width
                        text: page.localization.text("setup.mapping.description")
                        color: "#8f9aa7"
                        font.pixelSize: 14
                        wrapMode: Text.WordWrap
                    }
                    Row {
                        width: parent.width
                        height: 52
                        spacing: 10
                        Repeater {
                            model: page.modbusTelemetry ? page.modbusTelemetry.availableMappings : []
                            delegate: TouchKey {
                                width: 230
                                height: 52
                                label: page.modbusTelemetry.labelForMapping(modelData)
                                emphasized: page.modbusTelemetry.mappingKey === modelData
                                enabled: page.hasPermission("modbus.mapping.change") && !page.mappingReloadPending
                                onPressed: page.requestMappingChange(modelData)
                            }
                        }
                    }
                    Text {
                        text: page.modbusTelemetry ? page.localization.text("setup.mapping.active") + page.modbusTelemetry.mappingLabel : page.localization.text("setup.mapping.unavailable")
                        color: "#47d18c"
                        font.pixelSize: 14
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 214
                radius: 10
                color: "#1b2128"
                border.color: "#303a45"
                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 10
                    Text { text: page.localization.text("setup.backend"); color: "#f2f4f7"; font.pixelSize: 20; font.bold: true }
                    Text {
                        text: page.backendSupervisor
                            ? page.localization.text("setup.status")
                                + (page.backendSupervisor.state === "Running" ? page.localization.text("state.running")
                                    : page.backendSupervisor.state === "Stopped" ? page.localization.text("state.stopped")
                                        : page.backendSupervisor.state === "Error" ? page.localization.text("state.error")
                                            : page.backendSupervisor.state === "Unavailable" ? page.localization.text("state.unavailable") : page.backendSupervisor.state)
                            : page.localization.text("setup.status.unavailable")
                        color: page.backendSupervisor && page.backendSupervisor.state === "Running" ? "#47d18c" : "#e5b85c"
                        font.pixelSize: 16
                    }
                    Row {
                        width: parent.width
                        height: 52
                        spacing: 10
                        TouchKey { width: (parent.width - 3 * 10) / 4; height: 52; label: page.localization.text("setup.start"); enabled: page.hasPermission("backend.control") && page.backendSupervisor && !page.backendSupervisor.busy; onPressed: page.requestBackendAction("start") }
                        TouchKey { width: (parent.width - 3 * 10) / 4; height: 52; label: page.localization.text("setup.stop"); enabled: page.hasPermission("backend.control") && page.backendSupervisor && !page.backendSupervisor.busy; onPressed: page.requestBackendAction("stop") }
                        TouchKey { width: (parent.width - 3 * 10) / 4; height: 52; label: page.localization.text("setup.restart"); enabled: page.hasPermission("backend.control") && page.backendSupervisor && !page.backendSupervisor.busy; onPressed: page.requestBackendAction("restart") }
                        TouchKey { width: (parent.width - 3 * 10) / 4; height: 52; label: page.localization.text("setup.refresh"); enabled: page.hasPermission("setup.view") && page.backendSupervisor && !page.backendSupervisor.busy; onPressed: page.requestBackendAction("refresh") }
                    }
                    Text {
                        width: parent.width
                        text: page.backendSupervisor && page.backendSupervisor.error !== "" ? page.backendSupervisor.error : page.localization.text("setup.fixed.unit")
                        color: page.backendSupervisor && page.backendSupervisor.error !== "" ? "#e06262" : "#8f9aa7"
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                    }
                    Text { text: page.localization.text("setup.software.version") + page.softwareVersion; color: "#8f9aa7"; font.pixelSize: 13 }
                }
            }

            Rectangle {
                width: parent.width
                height: 178
                radius: 10
                color: "#1b2128"
                border.color: "#303a45"
                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 10
                    Text { text: page.localization.text("setup.weather"); color: "#f2f4f7"; font.pixelSize: 20; font.bold: true }
                    Row {
                        width: parent.width
                        height: 56
                        spacing: 12
                        Rectangle {
                            width: parent.width - 124
                            height: 56
                            radius: 9
                            color: "#202832"
                            border.color: locationInput.activeFocus ? "#5ca9e6" : "#3a4652"
                            border.width: locationInput.activeFocus ? 2 : 1
                            TextInput {
                                id: locationInput
                                anchors.fill: parent
                                anchors.leftMargin: 16
                                anchors.rightMargin: 16
                                verticalAlignment: TextInput.AlignVCenter
                                color: "#f2f4f7"
                                font.pixelSize: 17
                                text: page.weatherService ? page.weatherService.location : ""
                                inputMethodHints: Qt.ImhPreferLatin
                                selectByMouse: true
                                onAccepted: page.updateWeatherLocation()
                            }
                            Text {
                                visible: locationInput.text === "" && !locationInput.activeFocus
                                anchors.left: parent.left
                                anchors.leftMargin: 16
                                anchors.verticalCenter: parent.verticalCenter
                                text: page.localization.text("weather.city.placeholder")
                                color: "#8f9aa7"
                                font.pixelSize: 17
                            }
                        }
                        TouchKey { width: 112; height: 56; label: page.localization.text("setup.update"); emphasized: true; onPressed: page.updateWeatherLocation() }
                    }
                    Text {
                        width: parent.width
                        text: page.weatherService && page.weatherService.loading
                            ? page.localization.text("weather.updating.details")
                            : page.weatherService && page.weatherService.available
                                ? page.weatherService.resolvedLocation + " · " + page.localization.weatherSummary(page.weatherService)
                                : page.weatherService && page.weatherService.error !== ""
                                    ? page.weatherService.error
                                    : page.localization.text("weather.enter.location")
                        color: page.weatherService && page.weatherService.available ? "#47d18c" : "#8f9aa7"
                        font.pixelSize: 14
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }

    Rectangle {
        visible: page.userManager && !page.userManager.setupAccessGranted
        anchors.fill: parent
        z: 30
        color: "#d0101317"
        Rectangle {
            anchors.centerIn: parent
            width: Math.min(parent.width - 80, 560)
            height: 300
            radius: 14
            color: "#1b2128"
            border.color: "#e5b85c"
            border.width: 2
            Column {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 12
                Text { text: page.localization.text("setup.setup.locked"); color: "#f2f4f7"; font.pixelSize: 24; font.bold: true }
                Text { width: parent.width; text: page.localization.text("setup.authenticate.description"); color: "#d6dde5"; font.pixelSize: 16; wrapMode: Text.WordWrap }
                TextInput { width: parent.width; height: 46; text: page.loginUser; onTextChanged: page.loginUser = text; color: "#f2f4f7"; font.pixelSize: 16; clip: true }
                TextInput { width: parent.width; height: 46; text: page.loginPassword; onTextChanged: page.loginPassword = text; echoMode: TextInput.Password; color: "#f2f4f7"; font.pixelSize: 16; clip: true }
                TouchKey { width: parent.width; height: 52; label: page.localization.text("setup.authenticate"); emphasized: true; onPressed: page.authenticate() }
            }
        }
    }

    Rectangle {
        visible: page.mappingReloadPending
        anchors.fill: parent
        z: 40
        color: "#b0101317"
        Rectangle {
            anchors.centerIn: parent
            width: Math.min(parent.width - 80, 700)
            height: 290
            radius: 14
            color: "#1b2128"
            border.color: "#e5b85c"
            border.width: 2
            Column {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 14
                Text { text: page.localization.text("setup.confirm.mapping"); color: "#f2f4f7"; font.pixelSize: 23; font.bold: true }
                Text { width: parent.width; text: page.localization.text("setup.confirm.mapping.description"); color: "#d6dde5"; font.pixelSize: 16; wrapMode: Text.WordWrap }
                Text { visible: page.mappingReloadError !== ""; text: page.mappingReloadError; color: "#e06262"; font.pixelSize: 14 }
                Row {
                    width: parent.width
                    height: 58
                    spacing: 14
                    TouchKey { width: (parent.width - 14) / 2; height: 58; label: page.localization.text("setup.cancel"); onPressed: { page.mappingReloadPending = false; page.mappingReloadError = "" } }
                    TouchKey { width: (parent.width - 14) / 2; height: 58; label: page.localization.text("setup.confirm.reload"); emphasized: true; onPressed: page.confirmMappingChange() }
                }
            }
        }
    }
}
