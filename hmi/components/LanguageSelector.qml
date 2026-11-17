import QtQuick
import QtQuick.Controls

ComboBox {
    id: selector
    property var languageManager: null
    property var languageCodes: ["de", "en"]
    implicitWidth: 94
    implicitHeight: 42
    model: selector.languageCodes
    currentIndex: selector.languageManager && selector.languageManager.language === "en" ? 1 : 0

    onActivated: {
        if (selector.languageManager) {
            selector.languageManager.setLanguage(index === 1 ? "en" : "de")
        }
    }

    contentItem: Row {
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        spacing: 6

        Image {
            width: 24
            height: 17
            source: selector.currentIndex === 1
                ? "qrc:/qt/qml/SolarPowerhouse/hmi/assets/flag-gb.svg"
                : "qrc:/qt/qml/SolarPowerhouse/hmi/assets/flag-de.svg"
            fillMode: Image.PreserveAspectFit
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: selector.currentIndex === 1 ? "EN" : "DE"
            color: "#f2f4f7"
            font.pixelSize: 13
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    indicator: Text {
        anchors.right: parent.right
        anchors.rightMargin: 9
        anchors.verticalCenter: parent.verticalCenter
        text: "⌄"
        color: "#c8d0d9"
        font.pixelSize: 16
    }

    background: Rectangle {
        radius: 8
        color: "#202832"
        border.color: selector.activeFocus || selector.popup.visible ? "#47d18c" : "#3a4652"
        border.width: 1
    }

    delegate: ItemDelegate {
        width: selector.popup.width - selector.popup.leftPadding - selector.popup.rightPadding
        height: 40
        highlighted: selector.highlightedIndex === index

        contentItem: Row {
            spacing: 8
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.verticalCenter: parent.verticalCenter

            Image {
                width: 24
                height: 17
                source: index === 1
                    ? "qrc:/qt/qml/SolarPowerhouse/hmi/assets/flag-gb.svg"
                    : "qrc:/qt/qml/SolarPowerhouse/hmi/assets/flag-de.svg"
                fillMode: Image.PreserveAspectFit
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: index === 1 ? "EN — English" : "DE — Deutsch"
                color: "#f2f4f7"
                font.pixelSize: 13
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        background: Rectangle {
            radius: 5
            color: parent.highlighted ? "#2c3947" : "#202832"
        }
    }

    popup: Popup {
        y: selector.height
        width: selector.width + 58
        padding: 4
        contentItem: ListView {
            implicitHeight: contentHeight
            model: selector.delegateModel
            currentIndex: selector.highlightedIndex
            clip: true
        }

        background: Rectangle {
            radius: 8
            color: "#202832"
            border.color: "#3a4652"
            border.width: 1
        }
    }

    Accessible.name: selector.languageManager
        ? selector.languageManager.text(currentIndex === 1 ? "language.english" : "language.german")
        : (currentIndex === 1 ? "English" : "Deutsch")
}
