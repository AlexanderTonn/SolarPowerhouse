import QtQuick

Item {
    id: pad
    property bool open: false
    property string title: "Enter value"
    property string unit: ""
    property string initialValue: ""
    property string valueText: ""
    property real minimum: 0
    property real maximum: 999999
    property int decimals: 1
    property string errorText: ""
    property string externalValidationError: ""
    property var localization: null
    signal accepted(string value)
    signal canceled()

    visible: open
    z: 200

    onOpenChanged: {
        if (open) {
            valueText = initialValue
            errorText = ""
        }
    }

    function appendDigit(digit) {
        var decimalPosition = valueText.indexOf(".")
        if (decimalPosition >= 0 && valueText.length - decimalPosition - 1 >= decimals) return
        if (valueText === "0") {
            valueText = digit
        } else if (valueText.length < 12) {
            valueText += digit
        }
        errorText = ""
    }

    function appendDecimal() {
        if (decimals > 0 && valueText.indexOf(".") < 0) {
            valueText = valueText === "" ? "0." : valueText + "."
        }
        errorText = ""
    }

    function removeLast() {
        if (valueText.length > 0) valueText = valueText.slice(0, -1)
        errorText = ""
    }

    function clearValue() {
        valueText = ""
        errorText = ""
    }

    function commitValue() {
        var parsed = Number(valueText)
        if (valueText === "" || !isFinite(parsed)) {
            errorText = pad.localization.text("numeric.valid.number")
            return
        }
        if (parsed < minimum || parsed > maximum) {
            errorText = pad.localization.text("numeric.allowed.range") + minimum + "–" + maximum + " " + unit
            return
        }
        if (externalValidationError !== "") {
            errorText = externalValidationError
            return
        }
        var normalized = decimals > 0 ? parsed.toFixed(decimals) : String(Math.round(parsed))
        accepted(normalized)
    }

    Rectangle {
        anchors.fill: parent
        color: "#b3000000"
        MouseArea { anchors.fill: parent; onClicked: pad.canceled() }
    }

    Rectangle {
        id: panel
        anchors.centerIn: parent
        width: Math.min(620, parent.width - 48)
        height: Math.min(700, parent.height - 48)
        radius: 16
        color: "#171c22"
        border.color: "#3a4652"
        border.width: 1

        MouseArea { anchors.fill: parent }

        Column {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 14

            Row {
                width: parent.width
                height: 38
                Text { text: pad.title; color: "#f2f4f7"; font.pixelSize: 23; font.bold: true }
                Item { width: parent.width - 190; height: 1 }
                Text { text: pad.unit; color: "#8f9aa7"; font.pixelSize: 17; anchors.verticalCenter: parent.verticalCenter }
            }

            Rectangle {
                width: parent.width
                height: 78
                radius: 10
                color: "#202832"
                border.color: pad.errorText === "" && pad.externalValidationError === "" ? "#3a4652" : "#e06262"
                border.width: 1
                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                    text: pad.valueText === "" ? "—" : pad.valueText
                    color: "#f2f4f7"
                    font.pixelSize: 34
                    font.bold: true
                }
            }

            Text {
                width: parent.width
                height: 22
                text: pad.errorText !== "" ? pad.errorText : pad.externalValidationError
                color: "#e06262"
                font.pixelSize: 14
                horizontalAlignment: Text.AlignRight
            }

            Grid {
                id: keyGrid
                width: parent.width
                columns: 3
                rowSpacing: 10
                columnSpacing: 10

                TouchKey { width: (keyGrid.width - 2 * keyGrid.columnSpacing) / 3; height: 62; label: "1"; onPressed: pad.appendDigit("1") }
                TouchKey { width: (keyGrid.width - 2 * keyGrid.columnSpacing) / 3; height: 62; label: "2"; onPressed: pad.appendDigit("2") }
                TouchKey { width: (keyGrid.width - 2 * keyGrid.columnSpacing) / 3; height: 62; label: "3"; onPressed: pad.appendDigit("3") }
                TouchKey { width: (keyGrid.width - 2 * keyGrid.columnSpacing) / 3; height: 62; label: "4"; onPressed: pad.appendDigit("4") }
                TouchKey { width: (keyGrid.width - 2 * keyGrid.columnSpacing) / 3; height: 62; label: "5"; onPressed: pad.appendDigit("5") }
                TouchKey { width: (keyGrid.width - 2 * keyGrid.columnSpacing) / 3; height: 62; label: "6"; onPressed: pad.appendDigit("6") }
                TouchKey { width: (keyGrid.width - 2 * keyGrid.columnSpacing) / 3; height: 62; label: "7"; onPressed: pad.appendDigit("7") }
                TouchKey { width: (keyGrid.width - 2 * keyGrid.columnSpacing) / 3; height: 62; label: "8"; onPressed: pad.appendDigit("8") }
                TouchKey { width: (keyGrid.width - 2 * keyGrid.columnSpacing) / 3; height: 62; label: "9"; onPressed: pad.appendDigit("9") }
                TouchKey { width: (keyGrid.width - 2 * keyGrid.columnSpacing) / 3; height: 62; label: "."; enabled: pad.decimals > 0; onPressed: pad.appendDecimal() }
                TouchKey { width: (keyGrid.width - 2 * keyGrid.columnSpacing) / 3; height: 62; label: "0"; onPressed: pad.appendDigit("0") }
                TouchKey { width: (keyGrid.width - 2 * keyGrid.columnSpacing) / 3; height: 62; label: "⌫"; onPressed: pad.removeLast() }
            }

            TouchKey {
                width: parent.width
                height: 44
                label: pad.localization.text("numeric.clear")
                onPressed: pad.clearValue()
            }

            Row {
                width: parent.width
                spacing: 12
                TouchKey { width: (parent.width - 12) / 2; height: 62; label: pad.localization.text("numeric.cancel"); onPressed: pad.canceled() }
                TouchKey { width: (parent.width - 12) / 2; height: 62; label: pad.localization.text("numeric.save"); emphasized: true; onPressed: pad.commitValue() }
            }
        }
    }
}
