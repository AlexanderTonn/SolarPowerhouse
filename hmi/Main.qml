import QtQuick
import QtQuick.Window

Window {
    id: applicationWindow
    visible: true
    width: 1280
    height: 800
    minimumWidth: 900
    minimumHeight: 600
    title: "Solar Powerhouse"
    color: "#101317"

    property string currentPage: "dashboard"
    property bool drawerOpen: false
    property var weatherController: weatherService
    property var backendController: backendSupervisor
    property var modbusController: modbusTelemetry
    property var userAccessController: userManager
    property var localization: appLocalization
    property string softwareVersion: appVersion
    property var messageModel: eventModel
    property var activeAlarms: activeAlarmModel
    property string islandCutoffVoltage: ""
    property string islandEnableVoltage: ""
    property bool telemetryStateKnown: false
    property bool lastTelemetryConnected: false

    Localization {
        id: appLocalization
        manager: languageManager
    }

    function formatTimestamp() {
        return Qt.formatDateTime(new Date(), applicationWindow.localization.dateTimeFormat + ":ss")
    }

    function refreshInitialMessages() {
        eventModel.setProperty(0, "title", localization.text("messages.application.started"))
        eventModel.setProperty(0, "detail", localization.text("messages.application.started.detail"))
        eventModel.setProperty(1, "title", localization.text("messages.backend.pending"))
        eventModel.setProperty(1, "detail", localization.text("messages.backend.pending.detail"))
        eventModel.setProperty(2, "title", localization.text("messages.switching.inhibited"))
        eventModel.setProperty(2, "detail", localization.text("messages.switching.inhibited.detail"))
    }

    ListModel {
        id: eventModel
        ListElement { timestamp: ""; severity: "INFO"; title: ""; detail: ""; component: "HMI" }
        ListElement { timestamp: ""; severity: "WARNING"; title: ""; detail: ""; component: "HMI IPC" }
        ListElement { timestamp: ""; severity: "WARNING"; title: ""; detail: ""; component: "Core" }
    }

    ListModel {
        id: activeAlarmModel
    }

    function activeAlarmIndex(key) {
        for (var index = 0; index < activeAlarmModel.count; ++index) {
            if (activeAlarmModel.get(index).key === key) return index
        }
        return -1
    }

    function setActiveCriticalAlarm(key, active, titleKey, detailKey, component) {
        var index = activeAlarmIndex(key)
        if (!active) {
            if (index >= 0) activeAlarmModel.remove(index)
            return
        }

        var title = localization.text(titleKey)
        var detail = localization.text(detailKey)
        if (index < 0) {
            // Keep the newest active alarm at index 0.  The header can then
            // show one alarm and naturally fall back to the next older one
            // when the current alarm is cleared.
            activeAlarmModel.insert(0, {
                key: key,
                title: title,
                detail: detail,
                timestamp: formatTimestamp(),
                component: component
            })
            return
        }

        // Keep the original occurrence timestamp while refreshing translated text.
        activeAlarmModel.setProperty(index, "title", title)
        activeAlarmModel.setProperty(index, "detail", detail)
        activeAlarmModel.setProperty(index, "component", component)
    }

    function optionalBoolean(value) {
        return value === true || value === 1 || value === "1" || value === "true" || value === "TRUE"
    }

    function updateCriticalAlarms() {
        var modbusConnected = applicationWindow.modbusController && applicationWindow.modbusController.connected
        setActiveCriticalAlarm(
            "mppt-offline",
            !modbusConnected,
            "alarms.mppt.offline",
            "alarms.mppt.offline.detail",
            "Core")

        var backendState = applicationWindow.backendController ? applicationWindow.backendController.state : "Unavailable"
        var backendUnavailable = backendState === "Stopped" || backendState === "Error" || backendState === "Unavailable"
        setActiveCriticalAlarm(
            "backend-unavailable",
            backendUnavailable,
            "alarms.backend",
            "alarms.backend.detail",
            "HMI")

        var values = modbusConnected ? applicationWindow.modbusController.values : {}
        var mpptFaultActive = modbusConnected && Number(values["controller_faults"] || 0) !== 0
        setActiveCriticalAlarm(
            "mppt-fault",
            mpptFaultActive,
            "alarms.mppt.fault",
            "alarms.mppt.fault.detail",
            "Core")

        // The optional protection keys are accepted now so the banner can use
        // the core/control-state contract without coupling the HMI to a
        // particular Modbus register bit assignment.
        var protectionState = String(values["island_inverter_protection_state"] || values["k4_protection_state"] || "").toUpperCase()
        var configuredCutoff = Number(applicationWindow.islandCutoffVoltage)
        var configuredEnable = Number(applicationWindow.islandEnableVoltage)
        var batteryVoltage = Number(values["battery_voltage_v"])
        var configuredProtectionActive = applicationWindow.islandCutoffVoltage !== ""
            && applicationWindow.islandEnableVoltage !== ""
            && isFinite(configuredCutoff)
            && isFinite(configuredEnable)
            && configuredCutoff < configuredEnable
            && isFinite(batteryVoltage)
            && batteryVoltage <= configuredCutoff
        var undervoltageActive = configuredProtectionActive
            || optionalBoolean(values["undervoltage_protection_active"])
            || optionalBoolean(values["undervoltage_protection"])
            || optionalBoolean(values["k4_undervoltage_protection_active"])
            || protectionState === "CUTOFF"
            || protectionState === "CUT_OFF"
            || protectionState === "UNDERVOLTAGE"
        setActiveCriticalAlarm(
            "undervoltage-protection",
            undervoltageActive,
            "alarms.undervoltage",
            configuredProtectionActive
                ? "alarms.undervoltage.voltage.detail"
                : "alarms.undervoltage.detail",
            "Core")
    }

    Component.onCompleted: {
        refreshInitialMessages()
        for (var index = 0; index < eventModel.count; ++index) {
            eventModel.setProperty(index, "timestamp", formatTimestamp())
        }
        updateCriticalAlarms()
    }

    Connections {
        target: applicationWindow.localization
        function onLanguageChanged() {
            applicationWindow.refreshInitialMessages()
            applicationWindow.updateCriticalAlarms()
        }
    }

    function navigate(page) {
        currentPage = page
        drawerOpen = false
    }

    function addMessage(severity, title, detail, component) {
        eventModel.insert(0, {
            timestamp: formatTimestamp(),
            severity: severity,
            title: title,
            detail: detail,
            component: component
        })
        if (eventModel.count > 500) eventModel.remove(500)
    }

    Connections {
        target: applicationWindow.backendController
        function onStateChanged() {
            var state = applicationWindow.backendController.state
            applicationWindow.updateCriticalAlarms()
            if (state === "Running") applicationWindow.addMessage("INFO", applicationWindow.localization.text("messages.backend.available"), applicationWindow.localization.text("messages.backend.available.detail"), "HMI")
            if (state === "Stopped") applicationWindow.addMessage("CRITICAL", applicationWindow.localization.text("messages.backend.stopped"), applicationWindow.localization.text("messages.backend.stopped.detail"), "HMI")
            if (state === "Error" || state === "Unavailable") applicationWindow.addMessage("CRITICAL", applicationWindow.localization.text("messages.backend.error"), applicationWindow.backendController.error, "HMI")
        }
    }

    Connections {
        target: applicationWindow.modbusController
        function onChanged() {
            var connected = applicationWindow.modbusController.connected
            applicationWindow.updateCriticalAlarms()
            if (!applicationWindow.telemetryStateKnown) {
                applicationWindow.telemetryStateKnown = true
                applicationWindow.lastTelemetryConnected = connected
                return
            }
            if (connected === applicationWindow.lastTelemetryConnected) return
            applicationWindow.lastTelemetryConnected = connected
            applicationWindow.addMessage(
                connected ? "INFO" : "CRITICAL",
                connected ? applicationWindow.localization.text("messages.opta.restored") : applicationWindow.localization.text("messages.opta.lost"),
                connected ? applicationWindow.localization.text("messages.opta.available") : applicationWindow.modbusController.error,
                "Core")
        }
    }

    Loader {
        anchors.fill: parent
        sourceComponent: applicationWindow.currentPage === "dashboard"
            ? dashboardComponent
            : applicationWindow.currentPage === "island"
                ? islandComponent
                : applicationWindow.currentPage === "grid"
                    ? gridComponent
                : applicationWindow.currentPage === "settings"
                    ? settingsComponent
                    : applicationWindow.currentPage === "setup"
                        ? setupComponent
                    : applicationWindow.currentPage === "messages"
                        ? messageListComponent
                        : dashboardComponent
    }

    Component {
        id: dashboardComponent
        Dashboard {
            weatherService: applicationWindow.weatherController
            backendSupervisor: applicationWindow.backendController
            modbusTelemetry: applicationWindow.modbusController
            softwareVersion: applicationWindow.softwareVersion
            messageModel: applicationWindow.messageModel
            activeAlarms: applicationWindow.activeAlarms
            localization: applicationWindow.localization
            onOpenIslandRequested: applicationWindow.navigate("island")
            onOpenGridRequested: applicationWindow.navigate("grid")
            onOpenSettingsRequested: applicationWindow.navigate("settings")
            onMenuRequested: applicationWindow.drawerOpen = true
        }
    }

    Component {
        id: islandComponent
        IslandPage {
            weatherService: applicationWindow.weatherController
            backendSupervisor: applicationWindow.backendController
            modbusTelemetry: applicationWindow.modbusController
            softwareVersion: applicationWindow.softwareVersion
            messageModel: applicationWindow.messageModel
            activeAlarms: applicationWindow.activeAlarms
            localization: applicationWindow.localization
            onBackRequested: applicationWindow.navigate("dashboard")
            onMenuRequested: applicationWindow.drawerOpen = true
        }
    }

    Component {
        id: gridComponent
        GridPage {
            weatherService: applicationWindow.weatherController
            backendSupervisor: applicationWindow.backendController
            modbusTelemetry: applicationWindow.modbusController
            softwareVersion: applicationWindow.softwareVersion
            messageModel: applicationWindow.messageModel
            activeAlarms: applicationWindow.activeAlarms
            localization: applicationWindow.localization
            onBackRequested: applicationWindow.navigate("dashboard")
            onMenuRequested: applicationWindow.drawerOpen = true
        }
    }

    Component {
        id: settingsComponent
        SettingsPage {
            weatherService: applicationWindow.weatherController
            backendSupervisor: applicationWindow.backendController
            islandCutoffVoltage: applicationWindow.islandCutoffVoltage
            islandEnableVoltage: applicationWindow.islandEnableVoltage
            softwareVersion: applicationWindow.softwareVersion
            messageModel: applicationWindow.messageModel
            activeAlarms: applicationWindow.activeAlarms
            localization: applicationWindow.localization
            onIslandCutoffVoltageChanged: {
                applicationWindow.islandCutoffVoltage = islandCutoffVoltage
                applicationWindow.updateCriticalAlarms()
            }
            onIslandEnableVoltageChanged: {
                applicationWindow.islandEnableVoltage = islandEnableVoltage
                applicationWindow.updateCriticalAlarms()
            }
            onBackRequested: applicationWindow.navigate("dashboard")
            onMenuRequested: applicationWindow.drawerOpen = true
        }
    }

    Component {
        id: setupComponent
        SetupPage {
            weatherService: applicationWindow.weatherController
            backendSupervisor: applicationWindow.backendController
            modbusTelemetry: applicationWindow.modbusController
            userManager: applicationWindow.userAccessController
            softwareVersion: applicationWindow.softwareVersion
            messageModel: applicationWindow.messageModel
            activeAlarms: applicationWindow.activeAlarms
            localization: applicationWindow.localization
            onBackRequested: applicationWindow.navigate("dashboard")
            onMenuRequested: applicationWindow.drawerOpen = true
        }
    }

    Component {
        id: messageListComponent
        MessageListPage {
            messageModel: applicationWindow.messageModel
            backendSupervisor: applicationWindow.backendController
            modbusTelemetry: applicationWindow.modbusController
            softwareVersion: applicationWindow.softwareVersion
            activeAlarms: applicationWindow.activeAlarms
            localization: applicationWindow.localization
            onBackRequested: applicationWindow.navigate("dashboard")
            onMenuRequested: applicationWindow.drawerOpen = true
        }
    }

    NavigationDrawer {
        anchors.fill: parent
        open: applicationWindow.drawerOpen
        currentPage: applicationWindow.currentPage
        backendSupervisor: applicationWindow.backendController
        userManager: applicationWindow.userAccessController
        localization: applicationWindow.localization
        softwareVersion: applicationWindow.softwareVersion
        onPageRequested: applicationWindow.navigate(page)
        onCloseRequested: applicationWindow.drawerOpen = false
    }
}
