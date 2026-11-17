#pragma once

#include "domain/EventManager.hpp"
#include "infrastructure/OptaModbusTcp.hpp"
#include "shared/SolarTypes.hpp"

#include <chrono>
#include <string>

namespace solar {

class ApplicationController {
public:
    ApplicationController();

    bool start();
    void tick();
    void stop() noexcept;

    [[nodiscard]] bool running() const noexcept { return running_; }
    [[nodiscard]] const SolarSettings& settings() const noexcept { return settings_; }
    [[nodiscard]] infrastructure::ModbusMappingId mapping() const noexcept { return mapping_; }
    [[nodiscard]] const infrastructure::OptaTelemetry& telemetry() const noexcept { return telemetry_; }
    [[nodiscard]] EventManager& eventManager() noexcept { return eventManager_; }
    [[nodiscard]] const EventManager& eventManager() const noexcept { return eventManager_; }

private:
    void publishTelemetryState() const;

    SolarSettings settings_{};
    EventManager eventManager_{};
    infrastructure::ModbusMappingId mapping_;
    infrastructure::OptaModbusTcpClient modbusClient_;
    infrastructure::OptaTelemetry telemetry_{};
    std::chrono::steady_clock::time_point lastPoll_{};
    bool running_{false};
};

} // namespace solar
