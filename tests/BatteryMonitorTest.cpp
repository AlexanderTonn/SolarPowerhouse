#include "TestSupport.hpp"

#include "domain/BatteryMonitor.hpp"
#include "domain/EnergyManager.hpp"

void runBatteryMonitorTests() {
    using namespace std::chrono_literals;
    using solar::BatteryMeasurements;
    using solar::BatteryMonitor;
    using solar::Clock;
    using solar::EnergyManager;
    using solar::PhysicalState;
    using solar::SolarSettings;

    const auto start = Clock::time_point{};
    SolarSettings settings;
    settings.chargeBelowVoltage = 26.0;
    settings.batteryFullAboveVoltage = 28.0;
    settings.voltageConfirmTime = 30s;

    BatteryMonitor monitor;
    monitor.update({25.8, 1.0, true}, start);
    REQUIRE(!monitor.evaluateAutomaticTarget(PhysicalState::MicroinverterActive, settings, start));
    monitor.update({25.8, 1.0, true}, start + 29s);
    REQUIRE(!monitor.evaluateAutomaticTarget(PhysicalState::MicroinverterActive, settings, start + 29s));
    monitor.update({25.8, 1.0, true}, start + 30s);
    REQUIRE(monitor.evaluateAutomaticTarget(PhysicalState::MicroinverterActive, settings, start + 30s)
            == PhysicalState::MpptActive);

    monitor.update({28.2, 0.1, true}, start + 31s);
    REQUIRE(!monitor.evaluateAutomaticTarget(PhysicalState::MpptActive, settings, start + 31s));
    monitor.update({28.2, 0.1, true}, start + 61s);
    REQUIRE(monitor.evaluateAutomaticTarget(PhysicalState::MpptActive, settings, start + 61s)
            == PhysicalState::MicroinverterActive);

    settings.chargeBelowVoltage = 29.0;
    REQUIRE(!monitor.evaluateAutomaticTarget(PhysicalState::MpptActive, settings, start + 62s));

    settings.chargeBelowVoltage = 26.0;
    settings.batteryFullAboveVoltage = 28.0;
    settings.minimumModeDuration = 300s;
    EnergyManager manager(monitor);
    manager.recordStableState(PhysicalState::MpptActive, start);
    monitor.update({28.2, 0.1, true}, start + 1s);
    REQUIRE(!manager.selectTarget(solar::OperatingMode::Auto, PhysicalState::MpptActive, settings, start + 1s));
    monitor.update({28.2, 0.1, true}, start + 301s);
    REQUIRE(manager.selectTarget(solar::OperatingMode::Auto, PhysicalState::MpptActive, settings, start + 301s)
            == PhysicalState::MicroinverterActive);
    REQUIRE(manager.selectTarget(
                solar::OperatingMode::Manual,
                solar::ManualTarget::Microinverter,
                PhysicalState::MpptActive,
                settings,
                start + 302s)
            == PhysicalState::MicroinverterActive);
    REQUIRE(manager.selectTarget(
                solar::OperatingMode::Manual,
                solar::ManualTarget::Off,
                PhysicalState::MicroinverterActive,
                settings,
                start + 303s)
            == PhysicalState::AllOff);
}
