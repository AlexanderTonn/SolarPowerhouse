#include "TestSupport.hpp"
#include "MockHardwareInterface.hpp"

#include "domain/InterlockManager.hpp"
#include "domain/SwitchingController.hpp"

void runSwitchingControllerTests() {
    using namespace std::chrono_literals;
    using solar::Clock;
    using solar::Contactor;
    using solar::ContactorState;
    using solar::FaultCode;
    using solar::InterlockManager;
    using solar::PhysicalState;
    using solar::SwitchingController;

    const auto start = Clock::time_point{};
    MockHardwareInterface hardware;
    InterlockManager interlocks;
    SwitchingController controller(hardware, interlocks, {1s, 2s});

    hardware.setFeedback({false, false, false});
    controller.reconcile(start);
    REQUIRE(controller.state() == PhysicalState::AllOff);

    REQUIRE(controller.request(PhysicalState::MpptActive, start));
    REQUIRE(hardware.commands().size() == 1);
    REQUIRE(hardware.commands().at(0).contactor == Contactor::K3MpptDc);
    REQUIRE(hardware.commands().at(0).closed);

    hardware.setFeedback({false, false, true});
    controller.tick(start + 100ms);
    REQUIRE(controller.state() == PhysicalState::MpptActive);

    REQUIRE(controller.request(PhysicalState::MicroinverterActive, start + 100ms));
    REQUIRE(hardware.commands().size() == 2);
    REQUIRE(hardware.commands().at(1).contactor == Contactor::K3MpptDc);
    REQUIRE(!hardware.commands().at(1).closed);

    hardware.setFeedback({false, false, false});
    controller.tick(start + 200ms);
    controller.tick(start + 1200ms);
    REQUIRE(hardware.commands().size() == 3);
    REQUIRE(hardware.commands().at(2).contactor == Contactor::K2MicroinverterDc);
    REQUIRE(hardware.commands().at(2).closed);

    hardware.setFeedback({false, true, false});
    controller.tick(start + 1300ms);
    controller.tick(start + 2300ms);
    REQUIRE(hardware.commands().size() == 4);
    REQUIRE(hardware.commands().at(3).contactor == Contactor::K1MicroinverterAc);
    REQUIRE(hardware.commands().at(3).closed);

    hardware.setFeedback({true, true, false});
    controller.tick(start + 2400ms);
    REQUIRE(controller.state() == PhysicalState::MicroinverterActive);

    MockHardwareInterface timeoutHardware;
    SwitchingController timeoutController(timeoutHardware, interlocks, {1s, 1s});
    timeoutHardware.setFeedback({false, false, false});
    timeoutController.reconcile(start);
    REQUIRE(timeoutController.request(PhysicalState::MpptActive, start));
    timeoutController.tick(start + 1100ms);
    REQUIRE(timeoutController.state() == PhysicalState::Fault);
    REQUIRE(timeoutController.fault() == FaultCode::FeedbackTimeout);

    MockHardwareInterface invalidHardware;
    SwitchingController invalidController(invalidHardware, interlocks);
    invalidHardware.setFeedback({false, true, true});
    invalidController.reconcile(start);
    REQUIRE(invalidController.fault() == FaultCode::InterlockViolation);
}
