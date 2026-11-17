#pragma once

#include "GatewayConfig.hpp"

#include <cstdint>

namespace solar {
namespace opta {

class RelayController final {
public:
    explicit RelayController(const GatewayConfig& config);

    void begin();
    void allOff();
    bool applyCommand(std::uint16_t command);

    [[nodiscard]] std::uint16_t appliedBits() const noexcept { return appliedBits_; }
    [[nodiscard]] std::uint16_t feedbackBits() const;
    [[nodiscard]] static bool isValidCommand(std::uint16_t command) noexcept;

private:
    const GatewayConfig& config_;
    std::uint16_t appliedBits_{0};
};

} // namespace opta
} // namespace solar
