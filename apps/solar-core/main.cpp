#include "application/ApplicationController.hpp"

#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

namespace {

volatile std::sig_atomic_t stopRequested = 0;

void handleStopSignal(int /*signal*/) noexcept {
    stopRequested = 1;
}

} // namespace

int main() {
    std::signal(SIGINT, handleStopSignal);
    std::signal(SIGTERM, handleStopSignal);

    solar::ApplicationController application;
    if (!application.start()) {
        std::cerr << "solar-core: startup failed\n";
        return 1;
    }

    std::cout << "solar-core " << SOLAR_POWERHOUSE_VERSION << ": application started; hardware adapters pending\n";
    std::cout << "solar-core: automatic switching remains inhibited until commissioning\n";

    while (application.running() && stopRequested == 0) {
        application.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds{250});
    }

    application.stop();
    std::cout << "solar-core: stopped\n";
    return 0;
}
