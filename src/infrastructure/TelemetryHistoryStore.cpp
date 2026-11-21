#include "infrastructure/TelemetryHistoryStore.hpp"

#include <sqlite3.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <utility>

namespace solar::infrastructure {

namespace {

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool parseUnsigned(const std::string& value, std::uint32_t& result) {
    try {
        std::size_t consumed = 0;
        const auto parsed = std::stoul(value, &consumed, 10);
        if (consumed != value.size() || parsed > 0xFFFFFFFFUL) {
            return false;
        }
        result = static_cast<std::uint32_t>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

std::string sqliteError(sqlite3* database, const char* prefix) {
    std::ostringstream message;
    message << prefix;
    if (database != nullptr) {
        message << ": " << sqlite3_errmsg(database);
    }
    return message.str();
}

} // namespace

TelemetryHistoryStore::TelemetryHistoryStore(
    std::string databasePath,
    std::string settingsPath)
    : databasePath_(std::move(databasePath)),
      settingsPath_(std::move(settingsPath)) {}

TelemetryHistoryStore::~TelemetryHistoryStore() {
    close();
}

bool TelemetryHistoryStore::open() {
    close();
    error_.clear();

    const auto databaseFile = std::filesystem::path(databasePath_);
    std::error_code filesystemError;
    if (!databaseFile.parent_path().empty()) {
        std::filesystem::create_directories(databaseFile.parent_path(), filesystemError);
        if (filesystemError) {
            setError("Cannot create telemetry database directory: "
                     + filesystemError.message());
            return false;
        }
    }

    const auto result = sqlite3_open_v2(
        databasePath_.c_str(),
        &database_,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
        nullptr);
    if (result != SQLITE_OK) {
        setError(sqliteError(database_, "Cannot open telemetry database"));
        close();
        return false;
    }

    if (!execute("PRAGMA journal_mode=WAL;")
        || !execute("PRAGMA synchronous=NORMAL;")
        || !createSchema()
        || !loadLatestSample()) {
        close();
        return false;
    }

    // Missing settings file means use safe defaults. It will be created by
    // the HMI only after an operator changes a value.
    reloadSettings();
    pendingSamples_.reserve(settings_.samplesPerBatch());
    return true;
}

bool TelemetryHistoryStore::reloadSettings() {
    const auto settingsFile = std::filesystem::path(settingsPath_);
    std::error_code filesystemError;
    if (!std::filesystem::exists(settingsFile, filesystemError) || filesystemError) {
        return false;
    }

    const auto timestamp = std::filesystem::last_write_time(settingsFile, filesystemError);
    if (filesystemError) {
        return false;
    }
    if (settingsFileTimestamp_.has_value() && *settingsFileTimestamp_ == timestamp) {
        return false;
    }

    TelemetryHistorySettings updated = settings_;
    if (!readSettingsFile(updated) || !updated.valid()) {
        setError("Invalid telemetry history settings");
        settingsFileTimestamp_ = timestamp;
        return false;
    }
    if (updated == settings_) {
        settingsFileTimestamp_ = timestamp;
        return false;
    }

    // Persist current RAM batch before changing its calculated capacity.
    if (!flush()) {
        return false;
    }
    settings_ = updated;
    settingsFileTimestamp_ = timestamp;
    pendingSamples_.shrink_to_fit();
    pendingSamples_.reserve(settings_.samplesPerBatch());
    error_.clear();
    return true;
}

bool TelemetryHistoryStore::append(
    const OptaTelemetry& telemetry,
    std::int64_t timestampMs) {
    if (database_ == nullptr || !telemetry.connected) {
        return false;
    }

    // Keep RAM bounded by the configured batch size, even if the database
    // becomes unavailable. A failed flush retains old samples and rejects the
    // new one rather than allowing unbounded growth.
    if (pendingSamples_.size() >= settings_.samplesPerBatch() && !flush()) {
        return false;
    }

    TelemetrySample sample{
        timestampMs,
        telemetry.mppt.batteryVoltage,
        static_cast<double>(telemetry.mppt.chargingPower),
        telemetry.mppt.batteryTemperature,
        telemetry.mppt.chargingCurrent};
    pendingSamples_.push_back(sample);
    latestSample_ = sample;
    lastSampleTimestampMs_ = timestampMs;
    return true;
}

bool TelemetryHistoryStore::flush() {
    if (database_ == nullptr || pendingSamples_.empty()) {
        return database_ != nullptr;
    }

    if (!execute("BEGIN IMMEDIATE TRANSACTION;")) {
        return false;
    }

    sqlite3_stmt* statement = nullptr;
    constexpr auto insertSql =
        "INSERT OR REPLACE INTO telemetry_samples "
        "(timestamp_ms, battery_voltage_v, charging_power_w, "
        "battery_temperature_c, charging_current_a) VALUES (?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(database_, insertSql, -1, &statement, nullptr) != SQLITE_OK) {
        setError(sqliteError(database_, "Cannot prepare telemetry insert"));
        execute("ROLLBACK;");
        return false;
    }

    for (const auto& sample : pendingSamples_) {
        sqlite3_bind_int64(statement, 1, sample.timestampMs);
        sqlite3_bind_double(statement, 2, sample.batteryVoltage);
        sqlite3_bind_double(statement, 3, sample.chargingPower);
        sqlite3_bind_double(statement, 4, sample.batteryTemperature);
        sqlite3_bind_double(statement, 5, sample.chargingCurrent);

        if (sqlite3_step(statement) != SQLITE_DONE) {
            setError(sqliteError(database_, "Cannot write telemetry batch"));
            sqlite3_finalize(statement);
            execute("ROLLBACK;");
            return false;
        }
        sqlite3_reset(statement);
        sqlite3_clear_bindings(statement);
    }

    sqlite3_finalize(statement);
    if (!execute("COMMIT;")) {
        execute("ROLLBACK;");
        return false;
    }

    lastFlushedTimestampMs_ = pendingSamples_.back().timestampMs;
    pendingSamples_.clear();
    error_.clear();
    return true;
}

void TelemetryHistoryStore::close() noexcept {
    if (database_ == nullptr) {
        return;
    }
    flush();
    sqlite3_close(database_);
    database_ = nullptr;
}

bool TelemetryHistoryStore::createSchema() {
    return execute(
        "CREATE TABLE IF NOT EXISTS telemetry_samples ("
        "timestamp_ms INTEGER PRIMARY KEY NOT NULL,"
        "battery_voltage_v REAL NOT NULL,"
        "charging_power_w REAL NOT NULL,"
        "battery_temperature_c REAL NOT NULL,"
        "charging_current_a REAL NOT NULL"
        ");")
        && execute(
            "CREATE INDEX IF NOT EXISTS idx_telemetry_samples_timestamp "
            "ON telemetry_samples(timestamp_ms);");
}

bool TelemetryHistoryStore::loadLatestSample() {
    constexpr auto querySql =
        "SELECT timestamp_ms, battery_voltage_v, charging_power_w, "
        "battery_temperature_c, charging_current_a "
        "FROM telemetry_samples ORDER BY timestamp_ms DESC LIMIT 1;";
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(database_, querySql, -1, &statement, nullptr) != SQLITE_OK) {
        setError(sqliteError(database_, "Cannot read telemetry history"));
        return false;
    }

    if (sqlite3_step(statement) == SQLITE_ROW) {
        latestSample_ = TelemetrySample{
            sqlite3_column_int64(statement, 0),
            sqlite3_column_double(statement, 1),
            sqlite3_column_double(statement, 2),
            sqlite3_column_double(statement, 3),
            sqlite3_column_double(statement, 4)};
        lastSampleTimestampMs_ = latestSample_->timestampMs;
        lastFlushedTimestampMs_ = latestSample_->timestampMs;
    }
    sqlite3_finalize(statement);
    return true;
}

bool TelemetryHistoryStore::readSettingsFile(TelemetryHistorySettings& settings) const {
    std::ifstream file(settingsPath_);
    if (!file) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line.front() == '#') {
            continue;
        }
        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            return false;
        }
        const auto key = trim(line.substr(0, separator));
        const auto value = trim(line.substr(separator + 1));
        std::uint32_t parsed = 0;
        if (!parseUnsigned(value, parsed)) {
            return false;
        }
        if (key == "sample_interval_s") {
            settings.sampleIntervalSeconds = parsed;
        } else if (key == "batch_interval_s") {
            settings.batchIntervalSeconds = parsed;
        } else {
            return false;
        }
    }
    return true;
}

bool TelemetryHistoryStore::execute(const char* sql) {
    char* errorMessage = nullptr;
    const auto result = sqlite3_exec(database_, sql, nullptr, nullptr, &errorMessage);
    if (result == SQLITE_OK) {
        sqlite3_free(errorMessage);
        return true;
    }

    std::string message = errorMessage != nullptr ? errorMessage : "unknown SQLite error";
    sqlite3_free(errorMessage);
    setError("SQLite error: " + message);
    return false;
}

void TelemetryHistoryStore::setError(const std::string& error) {
    error_ = error;
}

} // namespace solar::infrastructure
