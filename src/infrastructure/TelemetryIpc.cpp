#include "infrastructure/TelemetryIpc.hpp"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <utility>

namespace solar::infrastructure {

namespace {

void closeSocket(int& socket) noexcept {
    if (socket >= 0) {
        ::close(socket);
        socket = -1;
    }
}

bool setNonBlocking(int socket) noexcept {
    const auto flags = ::fcntl(socket, F_GETFL, 0);
    return flags >= 0 && ::fcntl(socket, F_SETFL, flags | O_NONBLOCK) == 0;
}

} // namespace

TelemetryIpcServer::TelemetryIpcServer(std::string socketPath)
    : socketPath_(std::move(socketPath)) {}

TelemetryIpcServer::~TelemetryIpcServer() {
    stop();
}

bool TelemetryIpcServer::start() {
    if (serverSocket_ >= 0) {
        return true;
    }

    const auto path = std::filesystem::path(socketPath_);
    std::error_code filesystemError;
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path(), filesystemError);
        if (filesystemError) {
            error_ = "Cannot create telemetry IPC directory: " + filesystemError.message();
            return false;
        }
    }

    ::unlink(socketPath_.c_str());
    serverSocket_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverSocket_ < 0) {
        error_ = std::string("Telemetry IPC socket failed: ") + std::strerror(errno);
        return false;
    }

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    if (socketPath_.size() >= sizeof(address.sun_path)) {
        error_ = "Telemetry IPC socket path is too long";
        closeSocket(serverSocket_);
        return false;
    }
    std::strncpy(address.sun_path, socketPath_.c_str(), sizeof(address.sun_path) - 1);

    if (::bind(
            serverSocket_,
            reinterpret_cast<const sockaddr*>(&address),
            sizeof(address)) != 0) {
        error_ = std::string("Telemetry IPC bind failed: ") + std::strerror(errno);
        closeSocket(serverSocket_);
        return false;
    }
    if (::listen(serverSocket_, 4) != 0 || !setNonBlocking(serverSocket_)) {
        error_ = std::string("Telemetry IPC listen failed: ") + std::strerror(errno);
        stop();
        return false;
    }

    error_.clear();
    return true;
}

void TelemetryIpcServer::service() {
    if (serverSocket_ < 0 || snapshot_.empty()) {
        return;
    }

    for (;;) {
        const auto client = ::accept(serverSocket_, nullptr, nullptr);
        if (client < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            }
            if (errno == EINTR) {
                continue;
            }
            error_ = std::string("Telemetry IPC accept failed: ") + std::strerror(errno);
            return;
        }

        std::size_t offset = 0;
        while (offset < snapshot_.size()) {
            const auto sent = ::send(
                client,
                snapshot_.data() + offset,
                snapshot_.size() - offset,
#ifdef MSG_NOSIGNAL
                MSG_NOSIGNAL
#else
                0
#endif
            );
            if (sent <= 0) {
                break;
            }
            offset += static_cast<std::size_t>(sent);
        }
        ::shutdown(client, SHUT_WR);
        ::close(client);
    }
}

void TelemetryIpcServer::publish(std::string snapshot) {
    snapshot_ = std::move(snapshot);
}

void TelemetryIpcServer::stop() noexcept {
    closeSocket(serverSocket_);
    if (!socketPath_.empty()) {
        ::unlink(socketPath_.c_str());
    }
}

} // namespace solar::infrastructure
