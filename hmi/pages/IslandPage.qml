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
    property var visibleCurveKeys: ["battery_voltage_v", "charging_power_w", "battery_temperature_c", "charging_current_a"]
    property var chartCurves: [
        { key: "battery_voltage_v", labelKey: "telemetry.battery.voltage", color: "#47d18c", unit: "V", decimals: 1, address: "PDU 0101H" },
        { key: "charging_power_w", labelKey: "telemetry.charging.power", color: "#5ca9e6", unit: "W", decimals: 0, address: "PDU 0109H" },
        { key: "battery_temperature_c", labelKey: "telemetry.battery.temperature", color: "#e5b85c", unit: "°C", decimals: 1, address: "PDU 0103H" },
        { key: "charging_current_a", labelKey: "telemetry.battery.current", color: "#d477e8", unit: "A", decimals: 2, address: "PDU 0102H" }
    ]
    property var readOnlyPoints: [
        { key: "maximum_system_voltage_v", labelKey: "telemetry.maximum.system.voltage", unit: "V", decimals: 0, category: "device", address: "PDU 000AH" },
        { key: "rated_charging_current_a", labelKey: "telemetry.rated.charging.current", unit: "A", decimals: 0, category: "device", address: "PDU 000AH" },
        { key: "rated_discharging_current_a", labelKey: "telemetry.rated.discharging.current", unit: "A", decimals: 0, category: "device", address: "PDU 000BH" },
        { key: "product_type", labelKey: "telemetry.product.type", unit: "", decimals: 0, category: "device", address: "PDU 000BH" },
        { key: "product_model", labelKey: "telemetry.product.model", unit: "", decimals: 0, category: "device", address: "PDU 000CH-0013H" },
        { key: "software_version", labelKey: "telemetry.mppt.software", unit: "", decimals: 0, category: "device", address: "PDU 0014H-0015H" },
        { key: "hardware_version", labelKey: "telemetry.mppt.hardware", unit: "", decimals: 0, category: "device", address: "PDU 0016H-0017H" },
        { key: "serial_number", labelKey: "telemetry.mppt.serial", unit: "", decimals: 0, category: "device", address: "PDU 0018H-0019H" },
        { key: "device_address", labelKey: "telemetry.mppt.address", unit: "", decimals: 0, category: "device", address: "PDU 001AH" },
        { key: "soc_percent", labelKey: "telemetry.state.of.charge", unit: "%", decimals: 0, category: "current", address: "PDU 0100H" },
        { key: "battery_voltage_v", labelKey: "telemetry.battery.voltage", unit: "V", decimals: 1, category: "current", address: "PDU 0101H" },
        { key: "charging_current_a", labelKey: "telemetry.battery.current", unit: "A", decimals: 2, category: "current", address: "PDU 0102H" },
        { key: "controller_temperature_c", labelKey: "telemetry.controller.temperature", unit: "°C", decimals: 1, category: "current", address: "PDU 0103H" },
        { key: "battery_temperature_c", labelKey: "telemetry.battery.temperature", unit: "°C", decimals: 1, category: "current", address: "PDU 0103H" },
        { key: "load_voltage_v", labelKey: "telemetry.load.voltage", unit: "V", decimals: 1, category: "current", address: "PDU 0104H" },
        { key: "load_current_a", labelKey: "telemetry.load.current", unit: "A", decimals: 2, category: "current", address: "PDU 0105H" },
        { key: "load_power_w", labelKey: "telemetry.load.power", unit: "W", decimals: 0, category: "current", address: "PDU 0106H" },
        { key: "solar_voltage_v", labelKey: "telemetry.solar.voltage", unit: "V", decimals: 1, category: "current", address: "PDU 0107H" },
        { key: "solar_current_a", labelKey: "telemetry.solar.current", unit: "A", decimals: 2, category: "current", address: "PDU 0108H" },
        { key: "charging_power_w", labelKey: "telemetry.charging.power", unit: "W", decimals: 0, category: "current", address: "PDU 0109H" },
        { key: "daily_minimum_battery_voltage_v", labelKey: "telemetry.daily.minimum.battery", unit: "V", decimals: 1, category: "daily", address: "PDU 010BH" },
        { key: "daily_maximum_battery_voltage_v", labelKey: "telemetry.daily.maximum.battery", unit: "V", decimals: 1, category: "daily", address: "PDU 010CH" },
        { key: "daily_maximum_charging_current_a", labelKey: "telemetry.daily.maximum.charging", unit: "A", decimals: 2, category: "daily", address: "PDU 010DH" },
        { key: "daily_maximum_discharging_current_a", labelKey: "telemetry.daily.maximum.discharging", unit: "A", decimals: 2, category: "daily", address: "PDU 010EH" },
        { key: "daily_maximum_charging_power_w", labelKey: "telemetry.daily.maximum.charge.power", unit: "W", decimals: 0, category: "daily", address: "PDU 010FH" },
        { key: "daily_maximum_discharging_power_w", labelKey: "telemetry.daily.maximum.load.power", unit: "W", decimals: 0, category: "daily", address: "PDU 0110H" },
        { key: "daily_charging_amp_hours", labelKey: "telemetry.daily.charged", unit: "Ah", decimals: 0, category: "daily", address: "PDU 0111H" },
        { key: "daily_discharging_amp_hours", labelKey: "telemetry.daily.discharged", unit: "Ah", decimals: 0, category: "daily", address: "PDU 0112H" },
        { key: "daily_energy_generation_kwh", labelKey: "telemetry.daily.generation", unit: "kWh", decimals: 4, category: "daily", address: "PDU 0113H" },
        { key: "daily_energy_consumption_kwh", labelKey: "telemetry.daily.consumption", unit: "kWh", decimals: 4, category: "daily", address: "PDU 0114H" },
        { key: "total_operating_days", labelKey: "telemetry.operating.days", unit: "d", decimals: 0, category: "total", address: "PDU 0115H" },
        { key: "total_battery_over_discharges", labelKey: "telemetry.over.discharges", unit: "", decimals: 0, category: "total", address: "PDU 0116H" },
        { key: "total_battery_full_charges", labelKey: "telemetry.full.charges", unit: "", decimals: 0, category: "total", address: "PDU 0117H" },
        { key: "total_charging_amp_hours", labelKey: "telemetry.total.charged", unit: "Ah", decimals: 0, category: "total", address: "PDU 0118H-0119H" },
        { key: "total_discharging_amp_hours", labelKey: "telemetry.total.discharged", unit: "Ah", decimals: 0, category: "total", address: "PDU 011AH-011BH" },
        { key: "cumulative_energy_generation_kwh", labelKey: "telemetry.cumulative.generation", unit: "kWh", decimals: 4, category: "total", address: "PDU 011CH-011DH" },
        { key: "cumulative_energy_consumption_kwh", labelKey: "telemetry.cumulative.consumption", unit: "kWh", decimals: 4, category: "total", address: "PDU 011EH-011FH" },
        { key: "street_light_on", labelKey: "telemetry.load.output", unit: "", decimals: 0, category: "status", address: "PDU 0120H" },
        { key: "street_light_brightness_percent", labelKey: "telemetry.load.brightness", unit: "%", decimals: 0, category: "status", address: "PDU 0120H" },
        { key: "charging_state", labelKey: "telemetry.charging.state", unit: "", decimals: 0, category: "status", address: "PDU 0120H" },
        { key: "controller_faults", labelKey: "telemetry.controller.faults", unit: "", decimals: 0, category: "status", address: "PDU 0121H-0122H" },
        { key: "digital_inputs", labelKey: "telemetry.opta.inputs", unit: "", decimals: 0, category: "gateway", address: "Opta input 0" },
        { key: "contactor_state", labelKey: "telemetry.contactor.state", unit: "", decimals: 0, category: "gateway", address: "Opta input 7" },
        { key: "gateway_faults", labelKey: "telemetry.gateway.faults", unit: "", decimals: 0, category: "gateway", address: "Opta input 8" },
        { key: "gateway_status", labelKey: "telemetry.gateway.status", unit: "", decimals: 0, category: "gateway", address: "Opta input 11" },
        { key: "rtu_error_count", labelKey: "telemetry.rtu.errors", unit: "", decimals: 0, category: "gateway", address: "Opta input 12" }
    ]
    property var valueSections: [
        { key: "device", titleKey: "telemetry.section.device" },
        { key: "current", titleKey: "telemetry.section.current" },
        { key: "daily", titleKey: "telemetry.section.daily" },
        { key: "total", titleKey: "telemetry.section.total" },
        { key: "status", titleKey: "telemetry.section.status" },
        { key: "gateway", titleKey: "telemetry.section.gateway" }
    ]
    signal backRequested()
    signal menuRequested()

    function isCurveVisible(key) {
        return page.visibleCurveKeys.indexOf(key) >= 0
    }

    function toggleCurve(key) {
        var next = page.visibleCurveKeys.slice(0)
        var index = next.indexOf(key)
        if (index >= 0) {
            next.splice(index, 1)
        } else if (next.length < 4) {
            next.push(key)
        } else {
            logMessage("WARNING", page.localization.text("chart.limit.title"), page.localization.text("chart.limit.detail"), "HMI")
            return
        }
        page.visibleCurveKeys = next
    }

    function pointsForCategory(category) {
        var result = []
        for (var index = 0; index < page.readOnlyPoints.length; ++index) {
            if (page.readOnlyPoints[index].category === category) result.push(page.readOnlyPoints[index])
        }
        return result
    }

    function formatValue(point) {
        if (!page.modbusTelemetry || !page.modbusTelemetry.connected) return "--"
        var raw = page.modbusTelemetry.values[point.key]
        if (raw === undefined || raw === null) return "--"
        if (point.key === "street_light_on") return raw ? page.localization.text("telemetry.on") : page.localization.text("telemetry.off")
        if (point.key === "charging_state") {
            var states = [page.localization.text("telemetry.deactivated"), page.localization.text("telemetry.activated"), "MPPT", page.localization.text("telemetry.equalizing"), page.localization.text("telemetry.boost"), page.localization.text("telemetry.float"), page.localization.text("telemetry.limiting")]
            return states[Number(raw)] || String(raw)
        }
        var numeric = Number(raw)
        if (isNaN(numeric)) return String(raw)
        return numeric.toFixed(point.decimals) + (point.unit === "" ? "" : " " + point.unit)
    }

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
        pageTitle: page.localization.text("page.island.title")
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
        contentHeight: islandContent.height
        clip: true

        Column {
            id: islandContent
            width: parent.width
            spacing: 18

            Text { text: page.localization.text("page.island.description"); color: "#8f9aa7"; font.pixelSize: 16 }

            Rectangle {
                width: parent.width
                height: 500
                radius: 10
                color: "#1b2128"
                border.color: "#303a45"
                Column {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 12
                    Text { text: page.localization.text("page.island.history"); color: "#f2f4f7"; font.pixelSize: 20; font.bold: true }
                    Text {
                        text: page.modbusTelemetry && page.modbusTelemetry.connected
                            ? page.localization.text("page.island.chart.help")
                            : page.localization.text("page.island.waiting")
                        color: "#8f9aa7"
                        font.pixelSize: 14
                    }
                    Flow {
                        width: parent.width
                        height: 50
                        spacing: 8
                        Repeater {
                            model: page.chartCurves
                            delegate: CurveToggle {
                                width: Math.min(220, (parent.width - 24) / 4)
                                curveColor: modelData.color
                                label: page.localization.text(modelData.labelKey)
                                checked: page.isCurveVisible(modelData.key)
                                onToggled: page.toggleCurve(modelData.key)
                            }
                        }
                    }
                    TelemetryChart {
                        width: parent.width
                        height: 340
                        samples: page.modbusTelemetry ? page.modbusTelemetry.history : []
                        visibleCurves: page.visibleCurveKeys
                        curveDefinitions: page.chartCurves
                        localization: page.localization
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 58
                radius: 10
                color: page.modbusTelemetry && page.modbusTelemetry.connected ? "#18261f" : "#2b191b"
                border.color: page.modbusTelemetry && page.modbusTelemetry.connected ? "#2d6c4b" : "#8c3e43"
                Text {
                    anchors.fill: parent
                    anchors.margins: 16
                    text: page.modbusTelemetry && page.modbusTelemetry.connected
                        ? page.localization.text("page.island.connected") + page.modbusTelemetry.updatedAt
                        : page.localization.text("page.island.offline") + (page.modbusTelemetry ? page.modbusTelemetry.error : page.localization.text("page.island.no.telemetry"))
                    color: page.modbusTelemetry && page.modbusTelemetry.connected ? "#a8c9b5" : "#e5a4a8"
                    font.pixelSize: 14
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Text { text: page.localization.text("page.island.values"); color: "#f2f4f7"; font.pixelSize: 22; font.bold: true }
            Text { text: page.localization.text("page.island.values.description"); color: "#8f9aa7"; font.pixelSize: 14 }

            Column {
                id: valueSectionsColumn
                width: parent.width
                spacing: 24
                Repeater {
                    model: page.valueSections
                    delegate: Column {
                        id: sectionColumn
                        property var sectionDefinition: modelData
                        width: valueSectionsColumn.width
                        spacing: 8
                        Text {
                            text: page.localization.text(sectionColumn.sectionDefinition.titleKey)
                            color: "#47d18c"
                            font.pixelSize: 19
                            font.bold: true
                        }
                        Repeater {
                            model: page.pointsForCategory(sectionColumn.sectionDefinition.key)
                            delegate: TelemetryValueRow {
                                width: sectionColumn.width
                                label: page.localization.text(modelData.labelKey)
                                value: page.formatValue(modelData)
                                unit: modelData.unit
                            }
                        }
                    }
                }
            }
        }
    }
}
