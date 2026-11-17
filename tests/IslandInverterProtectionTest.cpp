#include "TestSupport.hpp"
#include "MockHardwareInterface.hpp"

#include "domain/IslandInverterProtectionController.hpp"

void runIslandInverterProtectionTests() {
    using namespace std::chrono_literals;
    using solar::Clock;
    using solar::Contactor;
    using solar::FaultCode;
    using solar::IslandInverterProtectionController;
    using solar::IslandInverterProtectionState;
    using solar::SolarSettings;

    const auto start = Clock::time_point{};
    SolarSettings settings;
    settings.islandInverterCutoffVoltage = 24.0;
    settings.islandInverterEnableVoltage = 26.0;
    settings.islandInverterVoltageConfirmTime = 5s;

    MockHardwareInterface hardware;
    hardware.setFeedback({false, false, false, true});
    IslandInverterProtectionController controller(hardware, {5s, 2s});
    controller.reconcile();
    REQUIRE(controller.state() == IslandInverterProtectionState::Enabled);

    controller.updateBatteryMeasurement({23.9, 0.0, true}, start);
    controller.evaluate(settings, start);
    controller.updateBatteryMeasurement({23.9, 0.0, true}, start + 5s);
    controller.evaluate(settings, start + 5s);
    REQUIRE(controller.busy());
    REQUIRE(hardware.commands().size() == 1);
    REQUIRE(hardware.commands().at(0).contactor == Contactor::K4IslandInverterDc);
    REQUIRE(!hardware.commands().at(0).closed);

    hardware.setFeedback({false, false, false, false});
    controller.tick(start + 6s);
    REQUIRE(controller.state() == IslandInverterProtectionState::CutOff);

    controller.updateBatteryMeasurement({26.1, 0.0, true}, start + 10s);
    controller.evaluate(settings, start + 10s);
    controller.updateBatteryMeasurement({26.1, 0.0, true}, start + 15s);
    controller.evaluate(settings, start + 15s);
    REQUIRE(hardware.commands().size() == 2);
    REQUIRE(hardware.commands().at(1).contactor == Contactor::K4IslandInverterDc);
    REQUIRE(hardware.commands().at(1).closed);

    hardware.setFeedback({false, false, false, true});
    controller.tick(start + 16s);
    REQUIRE(controller.state() == IslandInverterProtectionState::Enabled);

    MockHardwareInterface timeoutHardware;
    timeoutHardware.setFeedback({false, false, false, true});
    IslandInverterProtectionController timeoutController(timeoutHardware, {5s, 1s});
    timeoutController.reconcile();
    timeoutController.updateBatteryMeasurement({23.9, 0.0, true}, start);
    timeoutController.evaluate(settings, start);
    timeoutController.updateBatteryMeasurement({23.9, 0.0, true}, start + 5s);
    timeoutController.evaluate(settings, start + 5s);
    timeoutController.tick(start + 6s);
    REQUIRE(timeoutController.state() == IslandInverterProtectionState::Fault);
    REQUIRE(timeoutController.fault() == FaultCode::IslandInverterProtectionTimeout);

    MockHardwareInterface notConfiguredHardware;
    notConfiguredHardware.setFeedback({false, false, false, true});
    IslandInverterProtectionController notConfigured(notConfiguredHardware);
    notConfigured.reconcile();
    notConfigured.updateBatteryMeasurement({10.0, 0.0, true}, start);
    notConfigured.evaluate(SolarSettings{}, start);
    REQUIRE(notConfigured.state() == IslandInverterProtectionState::Enabled);
    REQUIRE(notConfiguredHardware.commands().empty());
}
