import QtQuick

QtObject {
    id: localization
    property var manager: null
    property string language: manager ? manager.language : "de"
    property string dateTimeFormat: manager ? manager.dateTimeFormat : "dd.MM.yyyy hh:mm"

    function text(key) {
        // Reading language creates a QML binding dependency, so all translated
        // text bindings are reevaluated immediately after the switch.
        var currentLanguage = localization.language
        return manager ? manager.text(key) : key
    }

    function setLanguage(code) {
        if (manager) manager.setLanguage(code)
    }

    function weatherCondition(condition) {
        var keys = {
            "Clear": "weather.clear",
            "Partly cloudy": "weather.partly.cloudy",
            "Fog": "weather.fog",
            "Drizzle": "weather.drizzle",
            "Rain": "weather.rain",
            "Snow": "weather.snow",
            "Rain showers": "weather.rain.showers",
            "Snow showers": "weather.snow.showers",
            "Thunderstorm": "weather.thunderstorm"
        }
        return keys[condition] ? text(keys[condition]) : condition
    }

    function weatherSummary(service) {
        if (!service) return ""
        return Number(service.temperature).toFixed(1) + " °C · " + weatherCondition(service.condition)
    }
}
