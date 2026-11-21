#include <iostream>

void runBatteryMonitorTests();
void runEventManagerTests();
void runSwitchingControllerTests();
void runIslandInverterProtectionTests();
void runMqttContractTests();
void runOptaModbusTcpTests();
void runTelemetryHistoryStoreTests();

int main() {
    runBatteryMonitorTests();
    runEventManagerTests();
    runSwitchingControllerTests();
    runIslandInverterProtectionTests();
    runMqttContractTests();
    runOptaModbusTcpTests();
    runTelemetryHistoryStoreTests();
    std::cout << "All solar-core tests passed\n";
    return 0;
}
