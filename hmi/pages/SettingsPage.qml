import QtQuick

Item {
    id: page
    property var weatherService: null
    property var messageModel: null
    property var backendSupervisor: null
    property var activeAlarms: null
    property var localization: null
    property string softwareVersion: ""
    property string operatingMode: "AUTO"
    property string manualTarget: "MPPT"
    property string lowThreshold: ""
    property string highThreshold: ""
    property string islandCutoffVoltage: ""
    property string islandEnableVoltage: ""
    property string confirmationTime: "30"
    property string interStepDelay: "1000"
    property string minimumModeDuration: "300"
    property string activeField: ""
    property string editorTitle: ""
    property string editorUnit: ""
    property string editorValue: ""
    property real editorMinimum: 0
    property real editorMaximum: 999999
    property int editorDecimals: 1
    signal backRequested()
    signal menuRequested()

    property bool thresholdOrderValid: lowThreshold === "" || highThreshold === ""
        || Number(lowThreshold) < Number(highThreshold)
    property bool islandProtectionOrderValid: islandCutoffVoltage === "" || islandEnableVoltage === ""
        || Number(islandCutoffVoltage) < Number(islandEnableVoltage)

    function openEditor(field, title, value, unit, minimum, maximum, decimals) {
        editorTitle = title
        editorValue = value
        editorUnit = unit
        editorMinimum = minimum
        editorMaximum = maximum
        editorDecimals = decimals
        activeField = field
    }

    function acceptEditor(value) {
        var changedField = activeField
        if (activeField === "low") lowThreshold = value
        if (activeField === "high") highThreshold = value
        if (activeField === "islandCutoff") islandCutoffVoltage = value
        if (activeField === "islandEnable") islandEnableVoltage = value
        if (activeField === "confirmation") confirmationTime = value
        if (activeField === "delay") interStepDelay = value
        if (activeField === "minimum") minimumModeDuration = value
        if (changedField !== "") logMessage("INFO", page.localization.text("messages.setting.changed"), changedField + " changed to " + value, "HMI")
        activeField = ""
    }

    function logMessage(severity, title, detail, component) {
        if (!messageModel) return
        messageModel.insert(0, {
            timestamp: Qt.formatDateTime(new Date(), page.localization.dateTimeFormat + ":ss"),
            severity: severity,
            title: title,
            detail: detail,
            component: component
        })
        if (messageModel.count > 500) messageModel.remove(500)
    }

    function setOperatingMode(value) {
        if (operatingMode === value) return
        operatingMode = value
        var modeLabel = value === "AUTO" ? page.localization.text("settings.auto")
            : value === "MANUAL" ? page.localization.text("settings.manual") : page.localization.text("settings.off")
        logMessage("INFO", page.localization.text("messages.operating.mode.changed"), page.localization.text("messages.operating.mode.set") + modeLabel, "HMI")
        if (value === "OFF") logMessage("WARNING", page.localization.text("messages.all.off.requested"), page.localization.text("messages.all.off.detail"), "HMI")
    }

    function setManualTarget(value) {
        if (manualTarget === value) return
        manualTarget = value
        var targetLabel = value === "MPPT" ? page.localization.text("settings.island.mppt")
            : value === "MICROINVERTER" ? page.localization.text("settings.microinverter") : page.localization.text("settings.all.off")
        logMessage("INFO", page.localization.text("messages.target.changed"), page.localization.text("messages.target.set") + targetLabel, "HMI")
        if (value === "OFF") logMessage("WARNING", page.localization.text("messages.all.off.requested"), page.localization.text("messages.all.off.detail"), "HMI")
    }

    function protectionEditorError(value) {
        if (value === "") return ""
        if (activeField === "islandCutoff" && islandEnableVoltage !== ""
            && Number(value) >= Number(islandEnableVoltage)) {
            return page.localization.text("settings.cutoff.must.be.lower")
        }
        if (activeField === "islandEnable" && islandCutoffVoltage !== ""
            && Number(value) <= Number(islandCutoffVoltage)) {
            return page.localization.text("settings.enable.must.be.higher")
        }
        return ""
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
        pageTitle: page.localization.text("page.settings.title")
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
        contentHeight: settingsContent.height
        clip: true

        Column {
            id: settingsContent
            width: parent.width
            spacing: 18

            Text { text: page.localization.text("page.settings.description"); color: "#8f9aa7"; font.pixelSize: 16 }

            Rectangle {
                width: parent.width
                height: page.operatingMode === "MANUAL" ? 202 : 142
                radius: 10
                color: "#1b2128"
                border.color: "#303a45"
                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    OperatingModeSelector {
                        width: parent.width
                        mode: page.operatingMode
                        manualTarget: page.manualTarget
                        localization: page.localization
                        onModeSelected: page.setOperatingMode(value)
                        onManualTargetSelected: page.setManualTarget(value)
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 492
                radius: 10
                color: "#1b2128"
                border.color: "#303a45"
                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    Text { text: page.localization.text("settings.switching"); color: "#f2f4f7"; font.pixelSize: 20; font.bold: true }
                    TouchInputField {
                        label: page.localization.text("settings.low.threshold")
                        value: page.lowThreshold
                        placeholder: page.localization.text("settings.not.commissioned")
                        unit: "V"
                        valid: page.thresholdOrderValid
                        onEditRequested: page.openEditor("low", page.localization.text("settings.low.threshold"), page.lowThreshold, "V", 0, 100, 1)
                    }
                    TouchInputField {
                        label: page.localization.text("settings.high.threshold")
                        value: page.highThreshold
                        placeholder: page.localization.text("settings.not.commissioned")
                        unit: "V"
                        valid: page.thresholdOrderValid
                        accentColor: "#5ca9e6"
                        onEditRequested: page.openEditor("high", page.localization.text("settings.high.threshold"), page.highThreshold, "V", 0, 100, 1)
                    }
                    TouchInputField {
                        label: page.localization.text("settings.confirmation.time")
                        value: page.confirmationTime
                        unit: "s"
                        accentColor: "#e5b85c"
                        onEditRequested: page.openEditor("confirmation", page.localization.text("settings.confirmation.time"), page.confirmationTime, "s", 1, 3600, 0)
                    }
                    TouchInputField {
                        label: page.localization.text("settings.inter.step.delay")
                        value: page.interStepDelay
                        unit: "ms"
                        accentColor: "#e5b85c"
                        onEditRequested: page.openEditor("delay", page.localization.text("settings.inter.step.delay"), page.interStepDelay, "ms", 100, 60000, 0)
                    }
                    TouchInputField {
                        label: page.localization.text("settings.minimum.mode.duration")
                        value: page.minimumModeDuration
                        unit: "s"
                        accentColor: "#e5b85c"
                        onEditRequested: page.openEditor("minimum", page.localization.text("settings.minimum.mode.duration"), page.minimumModeDuration, "s", 0, 86400, 0)
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 112
                radius: 10
                color: page.thresholdOrderValid ? "#18261f" : "#2b191b"
                border.color: page.thresholdOrderValid ? "#2d6c4b" : "#8c3e43"
                Column {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 6
                    Text {
                        text: page.thresholdOrderValid ? page.localization.text("settings.commissioning.required") : page.localization.text("settings.invalid.threshold.order")
                        color: page.thresholdOrderValid ? "#47d18c" : "#e06262"
                        font.pixelSize: 18
                        font.bold: true
                    }
                    Text {
                        width: parent.width
                        text: page.thresholdOrderValid
                            ? page.localization.text("settings.auto.inhibited")
                            : page.localization.text("settings.low.must.be.below.high")
                        color: page.thresholdOrderValid ? "#a8c9b5" : "#e5a4a8"
                        font.pixelSize: 15
                        wrapMode: Text.WordWrap
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 252
                radius: 10
                color: "#1b2128"
                border.color: "#303a45"
                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    Text { text: page.localization.text("settings.island.protection"); color: "#f2f4f7"; font.pixelSize: 20; font.bold: true }
                    Text { text: page.localization.text("settings.island.protection.description"); color: "#8f9aa7"; font.pixelSize: 14; wrapMode: Text.WordWrap; width: parent.width }
                    TouchInputField {
                        label: page.localization.text("settings.cutoff.voltage")
                        value: page.islandCutoffVoltage
                        placeholder: page.localization.text("settings.not.commissioned")
                        unit: "V"
                        valid: page.islandProtectionOrderValid
                        accentColor: "#e06262"
                        onEditRequested: page.openEditor("islandCutoff", page.localization.text("settings.cutoff.voltage"), page.islandCutoffVoltage, "V", 0, 100, 1)
                    }
                    TouchInputField {
                        label: page.localization.text("settings.enable.voltage")
                        value: page.islandEnableVoltage
                        placeholder: page.localization.text("settings.not.commissioned")
                        unit: "V"
                        valid: page.islandProtectionOrderValid
                        accentColor: "#47d18c"
                        onEditRequested: page.openEditor("islandEnable", page.localization.text("settings.enable.voltage"), page.islandEnableVoltage, "V", 0, 100, 1)
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 94
                radius: 10
                color: page.islandProtectionOrderValid ? "#18261f" : "#2b191b"
                border.color: page.islandProtectionOrderValid ? "#2d6c4b" : "#8c3e43"
                Text {
                    anchors.fill: parent
                    anchors.margins: 18
                    text: page.islandProtectionOrderValid
                        ? page.localization.text("settings.k4.inhibited")
                        : page.localization.text("settings.enable.must.be.higher")
                    color: page.islandProtectionOrderValid ? "#a8c9b5" : "#e5a4a8"
                    font.pixelSize: 15
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    TouchNumericPad {
        id: numericPad
        anchors.fill: parent
        open: page.activeField !== ""
        title: page.editorTitle
        unit: page.editorUnit
        initialValue: page.editorValue
        minimum: page.editorMinimum
        maximum: page.editorMaximum
        decimals: page.editorDecimals
        localization: page.localization
        externalValidationError: page.protectionEditorError(numericPad.valueText)
        onAccepted: page.acceptEditor(value)
        onCanceled: page.activeField = ""
    }
}
