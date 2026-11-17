#include "application/ApplicationController.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace solar {

namespace {

constexpr auto defaultMappingFileLinux = "/var/lib/solarpowerhouse/modbus-mapping.conf";
constexpr auto defaultStateFileLinux = "/var/lib/solarpowerhouse/modbus-state.json";
constexpr auto defaultMappingFileDevelopment = "/tmp/solarpowerhouse-modbus-mapping.conf";
constexpr auto defaultStateFileDevelopment = "/tmp/solarpowerhouse-modbus-state.json";
constexpr auto pollInterval = std::chrono::seconds{1};

std::string configuredPath(const char* variable, const char* linuxDefault, const char* developmentDefault) {
    if (const auto* value = std::getenv(variable); value != nullptr && *value != '\0') {
        return value;
    }
#ifdef __linux__
    return linuxDefault;
#else
    static_cast<void>(linuxDefault);
    return developmentDefault;
#endif
}

std::string mappingFilePath() {
    return configuredPath(
        "SOLAR_POWERHOUSE_MAPPING_FILE",
        defaultMappingFileLinux,
        defaultMappingFileDevelopment);
}

std::string stateFilePath() {
    return configuredPath(
        "SOLAR_POWERHOUSE_STATE_FILE",
        defaultStateFileLinux,
        defaultStateFileDevelopment);
}

infrastructure::ModbusMappingId configuredMapping() {
    std::ifstream file(mappingFilePath());
    std::string key;
    if (file && std::getline(file, key)) {
        while (!key.empty() && (key.back() == '\r' || key.back() == '\n' || key.back() == ' ' || key.back() == '\t')) {
            key.pop_back();
        }
        const auto first = key.find_first_not_of(" \t");
        if (first != std::string::npos) {
            key.erase(0, first);
        }
        if (const auto mapping = infrastructure::mappingFromKey(key); mapping.has_value()) {
            return *mapping;
        }
    }
    return infrastructure::ModbusMappingId::ToyoRenogy;
}

std::string jsonString(std::string_view value) {
    std::string escaped;
    escaped.reserve(value.size() + 2);
    for (const auto character : value) {
        if (character == '\\' || character == '"') {
            escaped.push_back('\\');
        }
        escaped.push_back(character);
    }
    return escaped;
}

void appendNumber(std::ostringstream& output, double value) {
    output << std::fixed << std::setprecision(3) << value;
}

std::string timestampUtc() {
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm calendar{};
#ifdef _WIN32
    gmtime_s(&calendar, &now);
#else
    gmtime_r(&now, &calendar);
#endif
    std::ostringstream output;
    output << std::put_time(&calendar, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}

} // namespace

ApplicationController::ApplicationController()
    : mapping_(configuredMapping()),
      modbusClient_(mapping_) {}

bool ApplicationController::start() {
    running_ = true;
    lastPoll_ = std::chrono::steady_clock::now() - pollInterval;
    eventManager_.record(
        EventType::SystemStarted,
        EventSeverity::Info,
        "ApplicationController",
        "solar-core started with " + std::string(infrastructure::mappingLabel(mapping_)) + " mapping");
    publishTelemetryState();
    return running_;
}

void ApplicationController::tick() {
    if (!running_) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    if (now - lastPoll_ < pollInterval) {
        return;
    }
    lastPoll_ = now;

    infrastructure::OptaTelemetry updated{};
    const auto connected = modbusClient_.poll(updated);
    telemetry_ = std::move(updated);
    eventManager_.recordBackendConnection(connected, "Opta ModbusTCP");
    publishTelemetryState();
}

void ApplicationController::stop() noexcept {
    if (!running_) {
        return;
    }

    // The only write in this read-only milestone is the lifecycle safe state:
    // command all Opta outputs to zero before closing the Modbus connection.
    if (!modbusClient_.safeShutdown()) {
        eventManager_.record(
            EventType::FaultRaised,
            EventSeverity::Critical,
            "Opta ModbusTCP",
            "Safe output shutdown command failed; gateway watchdog must keep outputs de-energized");
    }
    modbusClient_.stop();
    telemetry_.connected = false;
    telemetry_.error = "solar-core stopped";
    eventManager_.record(
        EventType::AllContactorsDeenergized,
        EventSeverity::Warning,
        "ApplicationController",
        "All Opta outputs commanded to zero during shutdown");
    running_ = false;
    publishTelemetryState();
}

void ApplicationController::publishTelemetryState() const {
    const auto path = std::filesystem::path(stateFilePath());
    std::error_code error;
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path(), error);
    }

    std::ostringstream json;
    const auto& mppt = telemetry_.mppt;
    json << "{\n"
         << "  \"schema_version\": 1,\n"
         << "  \"mapping\": \"" << jsonString(infrastructure::mappingKey(mapping_)) << "\",\n"
         << "  \"mapping_label\": \"" << jsonString(infrastructure::mappingLabel(mapping_)) << "\",\n"
         << "  \"connected\": " << (telemetry_.connected ? "true" : "false") << ",\n"
         << "  \"updated_at\": \"" << timestampUtc() << "\",\n"
         << "  \"error\": \"" << jsonString(telemetry_.error) << "\",\n"
         << "  \"values\": {\n"
         << "    \"digital_inputs\": " << telemetry_.digitalInputs << ",\n"
         << "    \"contactor_state\": " << telemetry_.contactorState << ",\n"
         << "    \"gateway_faults\": " << telemetry_.gatewayFaults << ",\n"
         << "    \"gateway_status\": " << telemetry_.gatewayStatus << ",\n"
         << "    \"rtu_error_count\": " << telemetry_.rtuErrorCount << ",\n"
         << "    \"maximum_system_voltage_v\": " << mppt.maximumSystemVoltage << ",\n"
         << "    \"rated_charging_current_a\": " << mppt.ratedChargingCurrent << ",\n"
         << "    \"rated_discharging_current_a\": " << mppt.ratedDischargingCurrent << ",\n"
         << "    \"product_type\": " << static_cast<unsigned>(mppt.productType) << ",\n"
         << "    \"product_model\": \"" << jsonString(mppt.productModel) << "\",\n"
         << "    \"software_version\": \"" << jsonString(mppt.softwareVersion) << "\",\n"
         << "    \"hardware_version\": \"" << jsonString(mppt.hardwareVersion) << "\",\n"
         << "    \"serial_number\": \"" << jsonString(mppt.serialNumber) << "\",\n"
         << "    \"device_address\": " << static_cast<unsigned>(mppt.deviceAddress) << ",\n"
         << "    \"soc_percent\": " << mppt.stateOfChargePercent << ",\n"
         << "    \"battery_voltage_v\": "; appendNumber(json, mppt.batteryVoltage); json << ",\n"
    << "    \"charging_current_a\": "; appendNumber(json, mppt.chargingCurrent); json << ",\n"
    << "    \"controller_temperature_c\": "; appendNumber(json, mppt.controllerTemperature); json << ",\n"
    << "    \"battery_temperature_c\": "; appendNumber(json, mppt.batteryTemperature); json << ",\n"
    << "    \"load_voltage_v\": "; appendNumber(json, mppt.loadVoltage); json << ",\n"
    << "    \"load_current_a\": "; appendNumber(json, mppt.loadCurrent); json << ",\n"
    << "    \"load_power_w\": " << mppt.loadPower << ",\n"
    << "    \"solar_voltage_v\": "; appendNumber(json, mppt.solarVoltage); json << ",\n"
    << "    \"solar_current_a\": "; appendNumber(json, mppt.solarCurrent); json << ",\n"
    << "    \"charging_power_w\": " << mppt.chargingPower << ",\n"
    << "    \"daily_minimum_battery_voltage_v\": "; appendNumber(json, mppt.dailyMinimumBatteryVoltage); json << ",\n"
    << "    \"daily_maximum_battery_voltage_v\": "; appendNumber(json, mppt.dailyMaximumBatteryVoltage); json << ",\n"
    << "    \"daily_maximum_charging_current_a\": "; appendNumber(json, mppt.dailyMaximumChargingCurrent); json << ",\n"
    << "    \"daily_maximum_discharging_current_a\": "; appendNumber(json, mppt.dailyMaximumDischargingCurrent); json << ",\n"
    << "    \"daily_maximum_charging_power_w\": " << mppt.dailyMaximumChargingPower << ",\n"
    << "    \"daily_maximum_discharging_power_w\": " << mppt.dailyMaximumDischargingPower << ",\n"
    << "    \"daily_charging_amp_hours\": " << mppt.dailyChargingAmpHours << ",\n"
    << "    \"daily_discharging_amp_hours\": " << mppt.dailyDischargingAmpHours << ",\n"
    << "    \"daily_energy_generation_kwh\": "; appendNumber(json, mppt.dailyEnergyGenerationKwh); json << ",\n"
    << "    \"daily_energy_consumption_kwh\": "; appendNumber(json, mppt.dailyEnergyConsumptionKwh); json << ",\n"
    << "    \"total_operating_days\": " << mppt.totalOperatingDays << ",\n"
    << "    \"total_battery_over_discharges\": " << mppt.totalBatteryOverDischarges << ",\n"
    << "    \"total_battery_full_charges\": " << mppt.totalBatteryFullCharges << ",\n"
    << "    \"total_charging_amp_hours\": " << mppt.totalChargingAmpHours << ",\n"
    << "    \"total_discharging_amp_hours\": " << mppt.totalDischargingAmpHours << ",\n"
    << "    \"cumulative_energy_generation_kwh\": "; appendNumber(json, mppt.cumulativeEnergyGenerationKwh); json << ",\n"
    << "    \"cumulative_energy_consumption_kwh\": "; appendNumber(json, mppt.cumulativeEnergyConsumptionKwh); json << ",\n"
    << "    \"street_light_on\": " << (mppt.streetLightOn ? "true" : "false") << ",\n"
    << "    \"street_light_brightness_percent\": " << static_cast<unsigned>(mppt.streetLightBrightnessPercent) << ",\n"
    << "    \"charging_state\": " << static_cast<unsigned>(mppt.chargingState) << ",\n"
    << "    \"controller_faults\": " << mppt.controllerFaults << "\n"
    << "  },\n"
    << "  \"input_registers\": [";
    for (std::size_t index = 0; index < telemetry_.inputRegisters.size(); ++index) {
        if (index != 0) {
            json << ", ";
        }
        json << telemetry_.inputRegisters[index];
    }
    json << "]\n}\n";

    const auto temporaryPath = path.string() + ".tmp";
    {
        std::ofstream file(temporaryPath, std::ios::trunc);
        if (!file) {
            return;
        }
        file << json.str();
        if (!file) {
            return;
        }
    }
    std::filesystem::rename(temporaryPath, path, error);
    if (error) {
        std::filesystem::remove(temporaryPath, error);
    }
}

} // namespace solar
