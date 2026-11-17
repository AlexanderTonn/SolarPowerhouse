#include "MpptRtuClient.hpp"

#include <ArduinoModbus.h>
#include <ArduinoRS485.h>

namespace solar {
namespace opta {

MpptRtuClient::MpptRtuClient(const GatewayConfig& config)
    : config_(config) {}

bool MpptRtuClient::begin() {
    started_ = ModbusRTUClient.begin(config_.mpptBaudRate, SERIAL_8N2);
    if (!started_) {
        return false;
    }

    const auto frameDelay = frameDelayMicroseconds(config_.mpptBaudRate);
    RS485.setDelays(frameDelay, frameDelay);
    ModbusRTUClient.setTimeout(config_.mpptResponseTimeoutMs);
    return true;
}

bool MpptRtuClient::poll() {
    if (!started_ && !begin()) {
        snapshot_.online = false;
        return false;
    }

    std::array<std::uint16_t, MpptSystemInformationRegisterCount> systemInformation{};
    std::array<std::uint16_t, MpptRegisterCount> data{};

    if (!readRegisters(config_.mpptSystemInformationAddress,
                       config_.mpptSystemInformationCount,
                       systemInformation.data())
        || !readRegisters(config_.mpptSystemDataAddress, 10, data.data())
        || !readRegisters(0x010B, 24, data.data() + 11)) {
        snapshot_.online = false;
        return false;
    }

    snapshot_.registers = data;
    snapshot_.systemInformation = systemInformation;
    snapshot_.soc = data[0];
    snapshot_.batteryVoltageRaw = data[1];
    snapshot_.chargingCurrentRaw = data[2];
    snapshot_.temperatureRaw = data[3];
    snapshot_.loadVoltageRaw = data[4];
    snapshot_.loadCurrentRaw = data[5];
    snapshot_.loadPower = data[6];
    snapshot_.pvVoltageRaw = data[7];
    snapshot_.pvCurrentRaw = data[8];
    snapshot_.chargingPower = data[9];
    snapshot_.statusRaw = data[32];
    snapshot_.faultRaw = (static_cast<std::uint32_t>(data[33]) << 16) | data[34];
    snapshot_.online = true;
    return true;
}

bool MpptRtuClient::readRegisters(
    std::uint16_t address,
    std::uint16_t count,
    std::uint16_t* values) {
    if (!ModbusRTUClient.requestFrom(
            config_.mpptSlaveId,
            HOLDING_REGISTERS,
            address,
            count)) {
        recordError();
        return false;
    }

    for (std::uint16_t index = 0; index < count; ++index) {
        const long value = ModbusRTUClient.read();
        if (value < 0 || value > 0xFFFF) {
            recordError();
            return false;
        }
        values[index] = static_cast<std::uint16_t>(value);
    }
    return true;
}

void MpptRtuClient::recordError() noexcept {
    if (errorCount_ != 0xFFFFu) {
        ++errorCount_;
    }
}

unsigned long MpptRtuClient::frameDelayMicroseconds(unsigned long baudRate) noexcept {
    // 3.5 characters using 10-bit character timing, rounded up.
    return (35000000UL + baudRate - 1UL) / baudRate;
}

} // namespace opta
} // namespace solar
