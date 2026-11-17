#include "domain/EventManager.hpp"

#include <utility>

namespace solar {

EventManager::EventManager(std::size_t maximumEntries)
    : maximumEntries_(maximumEntries) {}

void EventManager::record(
    EventType type,
    EventSeverity severity,
    std::string component,
    std::string message,
    std::chrono::system_clock::time_point timestamp) {
    if (maximumEntries_ == 0) {
        return;
    }
    events_.push_back({nextId_++, timestamp, severity, type, std::move(component), std::move(message)});
    while (events_.size() > maximumEntries_) {
        events_.pop_front();
    }
}

void EventManager::recordSettingChanged(
    std::string component,
    std::string setting,
    std::string previousValue,
    std::string newValue) {
    record(
        EventType::SettingChanged,
        EventSeverity::Info,
        std::move(component),
        setting + " changed from " + previousValue + " to " + newValue);
}

void EventManager::recordBackendConnection(bool connected, std::string component) {
    if (hasBackendState_ && backendConnected_ == connected) {
        return;
    }
    hasBackendState_ = true;
    backendConnected_ = connected;
    record(
        connected ? EventType::BackendConnectionRestored : EventType::BackendConnectionLost,
        connected ? EventSeverity::Info : EventSeverity::Critical,
        std::move(component),
        connected ? "Backend connection restored" : "Backend connection lost");
}

std::string_view toString(EventSeverity severity) noexcept {
    switch (severity) {
    case EventSeverity::Info: return "INFO";
    case EventSeverity::Warning: return "WARNING";
    case EventSeverity::Critical: return "CRITICAL";
    }
    return "UNKNOWN";
}

std::string_view toString(EventType type) noexcept {
    switch (type) {
    case EventType::SystemStarted: return "SYSTEM_STARTED";
    case EventType::SettingChanged: return "SETTING_CHANGED";
    case EventType::OperatingModeChanged: return "OPERATING_MODE_CHANGED";
    case EventType::ManualSwitchRequested: return "MANUAL_SWITCH_REQUESTED";
    case EventType::SwitchingRequested: return "SWITCHING_REQUESTED";
    case EventType::AllContactorsDeenergized: return "ALL_CONTACTORS_DEENERGIZED";
    case EventType::BackendConnectionLost: return "BACKEND_CONNECTION_LOST";
    case EventType::BackendConnectionRestored: return "BACKEND_CONNECTION_RESTORED";
    case EventType::FaultRaised: return "FAULT_RAISED";
    case EventType::FaultAcknowledged: return "FAULT_ACKNOWLEDGED";
    case EventType::IslandInverterProtectionCutoff: return "ISLAND_INVERTER_PROTECTION_CUTOFF";
    case EventType::IslandInverterProtectionRestored: return "ISLAND_INVERTER_PROTECTION_RESTORED";
    }
    return "UNKNOWN";
}

} // namespace solar
