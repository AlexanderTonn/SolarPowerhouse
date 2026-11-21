#pragma once

#include "infrastructure/OptaModbusTcp.hpp"
#include "shared/TelemetryHistorySettings.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct sqlite3;

namespace solar::infrastructure {

struct TelemetrySample final {
    std::int64_t timestampMs{0};
    double batteryVoltage{0.0};
    double chargingPower{0.0};
    double batteryTemperature{0.0};
    double chargingCurrent{0.0};
};

class TelemetryHistoryStore final {
public:
    TelemetryHistoryStore(std::string databasePath, std::string settingsPath);
    ~TelemetryHistoryStore();

    TelemetryHistoryStore(const TelemetryHistoryStore&) = delete;
    TelemetryHistoryStore& operator=(const TelemetryHistoryStore&) = delete;

    bool open();
    bool reloadSettings();
    bool append(const OptaTelemetry& telemetry, std::int64_t timestampMs);
    bool flush();
    void close() noexcept;

    [[nodiscard]] const TelemetryHistorySettings& settings() const noexcept { return settings_; }
    [[nodiscard]] bool available() const noexcept { return database_ != nullptr; }
    [[nodiscard]] const std::string& error() const noexcept { return error_; }
    [[nodiscard]] std::size_t pendingSamples() const noexcept { return pendingSamples_.size(); }
    [[nodiscard]] std::int64_t lastSampleTimestampMs() const noexcept { return lastSampleTimestampMs_; }
    [[nodiscard]] std::int64_t lastFlushedTimestampMs() const noexcept { return lastFlushedTimestampMs_; }
    [[nodiscard]] const std::optional<TelemetrySample>& latestSample() const noexcept { return latestSample_; }

private:
    bool createSchema();
    bool loadLatestSample();
    bool readSettingsFile(TelemetryHistorySettings& settings) const;
    bool execute(const char* sql);
    void setError(const std::string& error);

    std::string databasePath_;
    std::string settingsPath_;
    sqlite3* database_{nullptr};
    TelemetryHistorySettings settings_{};
    std::optional<std::filesystem::file_time_type> settingsFileTimestamp_{};
    std::vector<TelemetrySample> pendingSamples_{};
    std::optional<TelemetrySample> latestSample_{};
    std::int64_t lastSampleTimestampMs_{0};
    std::int64_t lastFlushedTimestampMs_{0};
    std::string error_{};
};

} // namespace solar::infrastructure
