#include "TestSupport.hpp"

#include "infrastructure/MqttTopics.hpp"
#include "shared/RegisterMap.hpp"

void runMqttContractTests() {
    REQUIRE(solar::mqtt_topics::Prefix == "solarpowerhouse/v1");
    REQUIRE(solar::mqtt_topics::OperatingModeCommand.ends_with("/command/operating_mode"));
    REQUIRE(solar::mqtt_topics::ConfirmedVoltageHigh.ends_with("/config/voltage_high"));
    REQUIRE(solar::register_map::RequestedContactors == 100);
    REQUIRE(solar::register_map::IpcHeartbeat == 101);
}
