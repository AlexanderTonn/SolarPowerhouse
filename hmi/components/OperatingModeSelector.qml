import QtQuick

Item {
    id: selector
    property string mode: "AUTO"
    property string manualTarget: "MPPT"
    property var localization: null
    signal modeSelected(string value)
    signal manualTargetSelected(string value)

    height: mode === "MANUAL" ? 142 : 82

    Column {
        anchors.fill: parent
        spacing: 10

        Text { text: selector.localization.text("settings.operating.mode"); color: "#f2f4f7"; font.pixelSize: 20; font.bold: true }

        Row {
            width: parent.width
            height: 52
            spacing: 10
            TouchKey { width: (parent.width - 2 * 10) / 3; height: 52; label: selector.localization.text("settings.auto"); emphasized: selector.mode === "AUTO"; onPressed: selector.modeSelected("AUTO") }
            TouchKey { width: (parent.width - 2 * 10) / 3; height: 52; label: selector.localization.text("settings.manual"); emphasized: selector.mode === "MANUAL"; onPressed: selector.modeSelected("MANUAL") }
            TouchKey { width: (parent.width - 2 * 10) / 3; height: 52; label: selector.localization.text("settings.off"); emphasized: selector.mode === "OFF"; onPressed: selector.modeSelected("OFF") }
        }

        Row {
            visible: selector.mode === "MANUAL"
            width: parent.width
            height: 52
            spacing: 10
            TouchKey { width: (parent.width - 2 * 10) / 3; height: 52; label: selector.localization.text("settings.island.mppt"); emphasized: selector.manualTarget === "MPPT"; onPressed: selector.manualTargetSelected("MPPT") }
            TouchKey { width: (parent.width - 2 * 10) / 3; height: 52; label: selector.localization.text("settings.microinverter"); emphasized: selector.manualTarget === "MICROINVERTER"; onPressed: selector.manualTargetSelected("MICROINVERTER") }
            TouchKey { width: (parent.width - 2 * 10) / 3; height: 52; label: selector.localization.text("settings.all.off"); emphasized: selector.manualTarget === "OFF"; onPressed: selector.manualTargetSelected("OFF") }
        }
    }
}
