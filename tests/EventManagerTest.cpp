#include "TestSupport.hpp"

#include "domain/EventManager.hpp"

void runEventManagerTests() {
    solar::EventManager events(3);
    const auto timestamp = std::chrono::system_clock::time_point{};

    events.record(
        solar::EventType::SystemStarted,
        solar::EventSeverity::Info,
        "Application",
        "Application started",
        timestamp);
    events.recordSettingChanged("HMI", "voltage_low", "null", "24.0");
    events.recordBackendConnection(false);
    events.recordBackendConnection(false);

    REQUIRE(events.size() == 3);
    REQUIRE(events.events().back().type == solar::EventType::BackendConnectionLost);
    REQUIRE(events.events().back().severity == solar::EventSeverity::Critical);
    REQUIRE(solar::toString(events.events().front().severity) == "INFO");
}
