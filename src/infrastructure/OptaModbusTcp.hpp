#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace solar::infrastructure {

enum class ModbusMappingId {
    ToyoRenogy,
};

struct ModbusMappingDefinition {
    ModbusMappingId id;
    std::string_view key;
    std::string_view label;
    std::string_view host;
    std::uint16_t port;
    std::uint8_t unitId;
    std::uint16_t inputRegisterStart;
    std::uint16_t inputRegisterCount;
    std::uint16_t requestedContactorsRegister;
    std::uint16_t heartbeatRegister;
    std::uint16_t commandSequenceRegister;
};

[[nodiscard]] const ModbusMappingDefinition& toyoRenogyMapping() noexcept;
[[nodiscard]] const ModbusMappingDefinition& mappingDefinition(ModbusMappingId id) noexcept;
[[nodiscard]] std::optional<ModbusMappingId> mappingFromKey(std::string_view key) noexcept;
[[nodiscard]] std::string_view mappingKey(ModbusMappingId id) noexcept;
[[nodiscard]] std::string_view mappingLabel(ModbusMappingId id) noexcept;

inline constexpr std::size_t OptaInputRegisterCount = 72;
inline constexpr std::size_t MpptRegisterCount = 35;
inline constexpr std::size_t SystemInformationRegisterCount = 17;

struct MpptReadOnlyData {
    std::uint16_t maximumSystemVoltage{0};
    std::uint16_t ratedChargingCurrent{0};
    std::uint16_t ratedDischargingCurrent{0};
    std::uint8_t productType{0};
    std::string productModel{};
    std::string softwareVersion{};
    std::string hardwareVersion{};
    std::string serialNumber{};
    std::uint8_t deviceAddress{0};
    std::uint16_t stateOfChargePercent{0};
    double batteryVoltage{0.0};
    double chargingCurrent{0.0};
    double controllerTemperature{0.0};
    double batteryTemperature{0.0};
    double loadVoltage{0.0};
    double loadCurrent{0.0};
    std::uint16_t loadPower{0};
    double solarVoltage{0.0};
    double solarCurrent{0.0};
    std::uint16_t chargingPower{0};
    double dailyMinimumBatteryVoltage{0.0};
    double dailyMaximumBatteryVoltage{0.0};
    double dailyMaximumChargingCurrent{0.0};
    double dailyMaximumDischargingCurrent{0.0};
    std::uint16_t dailyMaximumChargingPower{0};
    std::uint16_t dailyMaximumDischargingPower{0};
    std::uint16_t dailyChargingAmpHours{0};
    std::uint16_t dailyDischargingAmpHours{0};
    double dailyEnergyGenerationKwh{0.0};
    double dailyEnergyConsumptionKwh{0.0};
    std::uint16_t totalOperatingDays{0};
    std::uint16_t totalBatteryOverDischarges{0};
    std::uint16_t totalBatteryFullCharges{0};
    std::uint32_t totalChargingAmpHours{0};
    std::uint32_t totalDischargingAmpHours{0};
    double cumulativeEnergyGenerationKwh{0.0};
    double cumulativeEnergyConsumptionKwh{0.0};
    bool streetLightOn{false};
    std::uint8_t streetLightBrightnessPercent{0};
    std::uint8_t chargingState{0};
    std::uint32_t controllerFaults{0};
};

struct OptaTelemetry {
    bool connected{false};
    std::uint16_t digitalInputs{0};
    std::uint16_t contactorState{0};
    std::uint16_t gatewayFaults{0};
    std::uint16_t gatewayStatus{0};
    std::uint16_t rtuErrorCount{0};
    std::array<std::uint16_t, OptaInputRegisterCount> inputRegisters{};
    MpptReadOnlyData mppt{};
    std::string error{};
};

[[nodiscard]] MpptReadOnlyData decodeToyoRenogy(
    const std::array<std::uint16_t, MpptRegisterCount>& registers) noexcept;
[[nodiscard]] MpptReadOnlyData decodeToyoRenogy(
    const std::array<std::uint16_t, SystemInformationRegisterCount>& systemInformation,
    const std::array<std::uint16_t, MpptRegisterCount>& registers);

class OptaModbusTcpClient final {
public:
    explicit OptaModbusTcpClient(ModbusMappingId mapping);
    ~OptaModbusTcpClient();

    OptaModbusTcpClient(const OptaModbusTcpClient&) = delete;
    OptaModbusTcpClient& operator=(const OptaModbusTcpClient&) = delete;

    [[nodiscard]] bool poll(OptaTelemetry& telemetry);
    [[nodiscard]] bool safeShutdown();
    void stop() noexcept;

    [[nodiscard]] const ModbusMappingDefinition& mapping() const noexcept { return mapping_; }
    [[nodiscard]] std::string_view lastError() const noexcept { return lastError_; }

private:
    [[nodiscard]] bool connectSocket();
    void closeSocket() noexcept;
    [[nodiscard]] bool readInputRegisters(
        std::uint16_t start,
        std::uint16_t count,
        std::array<std::uint16_t, OptaInputRegisterCount>& values);
    [[nodiscard]] bool writeHoldingRegisters(
        std::uint16_t start,
        const std::array<std::uint16_t, 3>& values);
    [[nodiscard]] bool readExact(std::uint8_t* buffer, std::size_t size);
    [[nodiscard]] bool writeAll(const std::uint8_t* buffer, std::size_t size);
    void setError(std::string error);

    const ModbusMappingDefinition& mapping_;
    int socket_{-1};
    std::uint16_t transactionId_{0};
    std::uint16_t heartbeat_{0};
    std::uint16_t commandSequence_{0};
    std::string lastError_{};
};

} // namespace solar::infrastructure
