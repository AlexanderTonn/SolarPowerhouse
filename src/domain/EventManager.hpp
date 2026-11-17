#pragma once

#include <chrono>
#include <cstdint>
#include <deque>
#include <string>
#include <string_view>

namespace solar {

enum class EventSeverity {
    Info,
    Warning,
    Critical,
};

enum class EventType {
    SystemStarted,
    SettingChanged,
    OperatingModeChanged,
    ManualSwitchRequested,
    SwitchingRequested,
    AllContactorsDeenergized,
    BackendConnectionLost,
    BackendConnectionRestored,
    FaultRaised,
    FaultAcknowledged,
    IslandInverterProtectionCutoff,
    IslandInverterProtectionRestored,
};

struct SystemEvent {
    std::uint64_t id{0};
    std::chrono::system_clock::time_point timestamp{};
    EventSeverity severity{EventSeverity::Info};
    EventType type{EventType::SystemStarted};
    std::string component{};
    std::string message{};
};

class EventManager {
public:
    explicit EventManager(std::size_t maximumEntries = 500);

    void record(
        EventType type,
        EventSeverity severity,
        std::string component,
        std::string message,
        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now());

    void recordSettingChanged(
        std::string component,
        std::string setting,
        std::string previousValue,
        std::string newValue);
    void recordBackendConnection(bool connected, std::string component = "HMI IPC");

    [[nodiscard]] const std::deque<SystemEvent>& events() const noexcept { return events_; }
    [[nodiscard]] std::size_t size() const noexcept { return events_.size(); }

private:
    std::size_t maximumEntries_;
    std::uint64_t nextId_{1};
    std::deque<SystemEvent> events_{};
    bool hasBackendState_{false};
    bool backendConnected_{false};
};

[[nodiscard]] std::string_view toString(EventSeverity severity) noexcept;
[[nodiscard]] std::string_view toString(EventType type) noexcept;

} // namespace solar
