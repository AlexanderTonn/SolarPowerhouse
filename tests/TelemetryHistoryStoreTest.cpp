#include "TestSupport.hpp"

#include "infrastructure/TelemetryHistoryStore.hpp"

#include <filesystem>
#include <fstream>

void runTelemetryHistoryStoreTests() {
    using solar::infrastructure::OptaTelemetry;
    using solar::infrastructure::TelemetryHistoryStore;

    const auto databasePath = std::filesystem::temp_directory_path()
        / "solarpowerhouse-telemetry-history-test.sqlite";
    const auto settingsPath = std::filesystem::temp_directory_path()
        / "solarpowerhouse-telemetry-history-test.conf";
    std::error_code error;
    std::filesystem::remove(databasePath, error);
    std::filesystem::remove(databasePath.string() + "-wal", error);
    std::filesystem::remove(databasePath.string() + "-shm", error);
    std::filesystem::remove(settingsPath, error);

    {
        std::ofstream settings(settingsPath);
        settings << "sample_interval_s=10\n"
                    "batch_interval_s=30\n";
    }

    OptaTelemetry telemetry{};
    telemetry.connected = true;
    telemetry.mppt.batteryVoltage = 25.4;
    telemetry.mppt.chargingPower = 123;
    telemetry.mppt.batteryTemperature = 21.5;
    telemetry.mppt.chargingCurrent = 4.2;

    {
        TelemetryHistoryStore store(databasePath.string(), settingsPath.string());
        REQUIRE(store.open());
        REQUIRE(store.settings().sampleIntervalSeconds == 10);
        REQUIRE(store.settings().batchIntervalSeconds == 30);
        REQUIRE(store.settings().samplesPerBatch() == 3);
        REQUIRE(store.settings().estimatedBufferBytes() == 120);
        REQUIRE(store.append(telemetry, 1700000000000LL));
        REQUIRE(store.pendingSamples() == 1);
        REQUIRE(store.flush());
        REQUIRE(store.pendingSamples() == 0);
        REQUIRE(store.lastFlushedTimestampMs() == 1700000000000LL);
    }

    {
        TelemetryHistoryStore store(databasePath.string(), settingsPath.string());
        REQUIRE(store.open());
        REQUIRE(store.latestSample().has_value());
        REQUIRE(store.latestSample()->timestampMs == 1700000000000LL);
        REQUIRE(store.latestSample()->batteryVoltage == 25.4);
        REQUIRE(store.latestSample()->chargingPower == 123.0);
    }

    std::filesystem::remove(databasePath, error);
    std::filesystem::remove(databasePath.string() + "-wal", error);
    std::filesystem::remove(databasePath.string() + "-shm", error);
    std::filesystem::remove(settingsPath, error);
}
