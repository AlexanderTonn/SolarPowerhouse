#pragma once

#include "GatewayConfig.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace solar {
namespace opta {

constexpr std::size_t MpptRegisterCount = 35;
constexpr std::size_t MpptSystemInformationRegisterCount = 17;

static_assert(MpptRegisterCount == 0x0122u - 0x0100u + 1u,
              "MPPT data snapshot must cover 0x0100..0x0122");
static_assert(MpptSystemInformationRegisterCount == 0x001Au - 0x000Au + 1u,
              "MPPT system snapshot must cover 0x000A..0x001A");

struct MpptSnapshot final {
    std::uint16_t soc{0};
    std::uint16_t batteryVoltageRaw{0};
    std::uint16_t chargingCurrentRaw{0};
    std::uint16_t temperatureRaw{0};
    std::uint16_t loadVoltageRaw{0};
    std::uint16_t loadCurrentRaw{0};
    std::uint16_t loadPower{0};
    std::uint16_t pvVoltageRaw{0};
    std::uint16_t pvCurrentRaw{0};
    std::uint16_t chargingPower{0};
    std::uint16_t statusRaw{0};
    std::uint32_t faultRaw{0};
    std::array<std::uint16_t, MpptRegisterCount> registers{};
    std::array<std::uint16_t, MpptSystemInformationRegisterCount> systemInformation{};
    bool online{false};
};

class MpptRtuClient final {
public:
    explicit MpptRtuClient(const GatewayConfig& config);

    bool begin();
    bool poll();

    [[nodiscard]] bool started() const noexcept { return started_; }
    [[nodiscard]] const MpptSnapshot& snapshot() const noexcept { return snapshot_; }
    [[nodiscard]] std::uint16_t errorCount() const noexcept { return errorCount_; }

private:
    bool readRegisters(std::uint16_t address, std::uint16_t count, std::uint16_t* values);
    void recordError() noexcept;
    [[nodiscard]] static unsigned long frameDelayMicroseconds(unsigned long baudRate) noexcept;

    const GatewayConfig& config_;
    MpptSnapshot snapshot_{};
    std::uint16_t errorCount_{0};
    bool started_{false};
};

} // namespace opta
} // namespace solar
