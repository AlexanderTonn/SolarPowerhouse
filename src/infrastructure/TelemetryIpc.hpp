#pragma once

#include <string>
#include <string_view>

namespace solar::infrastructure {

class TelemetryIpcServer final {
public:
    explicit TelemetryIpcServer(std::string socketPath);
    ~TelemetryIpcServer();

    TelemetryIpcServer(const TelemetryIpcServer&) = delete;
    TelemetryIpcServer& operator=(const TelemetryIpcServer&) = delete;

    [[nodiscard]] bool start();
    void service();
    void publish(std::string snapshot);
    void stop() noexcept;

    [[nodiscard]] const std::string& error() const noexcept { return error_; }

private:
    std::string socketPath_;
    std::string snapshot_{};
    std::string error_{};
    int serverSocket_{-1};
};

} // namespace solar::infrastructure
