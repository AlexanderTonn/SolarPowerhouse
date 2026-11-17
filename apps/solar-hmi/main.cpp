#include <QGuiApplication>
#include <QQuickStyle>
#include <QQmlContext>
#include <QQmlApplicationEngine>

#include "hmi/BackendSupervisor.hpp"
#include "hmi/LanguageManager.hpp"
#include "hmi/ModbusTelemetry.hpp"
#include "hmi/UserManager.hpp"
#include "hmi/WeatherService.hpp"

int main(int argc, char* argv[]) {
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    QGuiApplication application(argc, argv);
    solar::BackendSupervisor backendSupervisor;
    solar::LanguageManager languageManager;
    solar::ModbusTelemetry modbusTelemetry;
    solar::UserManager userManager;
    solar::WeatherService weatherService;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("backendSupervisor", &backendSupervisor);
    engine.rootContext()->setContextProperty("languageManager", &languageManager);
    engine.rootContext()->setContextProperty("modbusTelemetry", &modbusTelemetry);
    engine.rootContext()->setContextProperty("userManager", &userManager);
    engine.rootContext()->setContextProperty("weatherService", &weatherService);
    engine.rootContext()->setContextProperty("appVersion", QStringLiteral(SOLAR_POWERHOUSE_VERSION));
    engine.loadFromModule("SolarPowerhouse", "Main");
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }
    return application.exec();
}
