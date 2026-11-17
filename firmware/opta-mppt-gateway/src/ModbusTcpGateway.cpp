#include "ModbusTcpGateway.hpp"

#include "ModbusRegisterMap.hpp"

#include <Arduino.h>
#include <SPI.h>

namespace solar {
namespace opta {

ModbusTcpGateway::ModbusTcpGateway(const GatewayConfig& config)
    : config_(config),
      tcpServer_(config.modbusTcpPort) {}

bool ModbusTcpGateway::begin() {
    // Arduino Mbed Ethernet takes a mutable MAC buffer although it does not
    // modify the supplied address. Keep configuration immutable and pass a
    // local copy to the framework API.
    std::uint8_t mac[6]{};
    for (std::uint8_t index = 0; index < 6; ++index) {
        mac[index] = config_.mac[index];
    }

    Ethernet.begin(
        mac,
        config_.localIp,
        config_.dns,
        config_.gateway,
        config_.subnet);
    tcpServer_.begin();

    if (modbusTcpServer_.begin() != 1) {
        return false;
    }
    if (modbusTcpServer_.configureInputRegisters(0, register_map::InputRegisterCount) != 0) {
        return false;
    }
    return modbusTcpServer_.configureHoldingRegisters(
               register_map::HoldingRegisterStart,
               register_map::HoldingRegisterCount)
        == 0;
}

void ModbusTcpGateway::service() {
    // ArduinoModbus keeps a reference to the accepted Client. Keep the client
    // in member storage instead of passing a loop-local object.
    if (!activeIpcClient_ || !activeIpcClient_.connected()) {
        if (activeIpcClient_) {
            activeIpcClient_.stop();
        }
        ipcConnected_ = false;

        EthernetClient candidate = tcpServer_.accept();
        if (!candidate) {
            return;
        }

        if (!allowedIpcClient(candidate)) {
            Serial.print("Rejected Modbus TCP client: ");
            Serial.println(candidate.remoteIP());
            candidate.stop();
            return;
        }

        activeIpcClient_ = candidate;
        modbusTcpServer_.accept(activeIpcClient_);
        ipcConnected_ = true;
    }

    modbusTcpServer_.poll();

    if (!activeIpcClient_.connected()) {
        activeIpcClient_.stop();
        ipcConnected_ = false;
    }
}

bool ModbusTcpGateway::readIpcCommand(IpcCommand& command) {
    if (!ipcConnected_) {
        return false;
    }

    const long heartbeat = modbusTcpServer_.holdingRegisterRead(register_map::IpcHeartbeat);
    const long sequence = modbusTcpServer_.holdingRegisterRead(register_map::CommandSequence);
    const long contactors = modbusTcpServer_.holdingRegisterRead(register_map::RequestedContactors);
    if (heartbeat < 0 || sequence < 0 || contactors < 0) {
        return false;
    }

    command.heartbeat = static_cast<std::uint16_t>(heartbeat);
    command.sequence = static_cast<std::uint16_t>(sequence);
    command.requestedContactors = static_cast<std::uint16_t>(contactors);
    return true;
}

void ModbusTcpGateway::publish(
    const MpptSnapshot& mppt,
    std::uint16_t contactorFeedback,
    std::uint16_t faultStatus,
    std::uint16_t rtuErrorCount,
    bool heartbeatFresh) {
    const auto batteryVoltage = static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(mppt.batteryVoltageRaw) * 10u);
    const auto gatewayStatus = static_cast<std::uint16_t>(
        (mppt.online ? 1u : 0u)
        | (heartbeatFresh ? 1u << 1 : 0u)
        | (ipcConnected_ ? 1u << 2 : 0u));

    modbusTcpServer_.inputRegisterWrite(register_map::DigitalInputs, readDigitalInputs());
    modbusTcpServer_.inputRegisterWrite(register_map::AnalogInput1, 0);
    modbusTcpServer_.inputRegisterWrite(register_map::BatteryVoltage, batteryVoltage);
    modbusTcpServer_.inputRegisterWrite(register_map::ChargingCurrent, mppt.chargingCurrentRaw);
    modbusTcpServer_.inputRegisterWrite(register_map::PvPowerHighWord, 0);
    modbusTcpServer_.inputRegisterWrite(register_map::PvPowerLowWord, mppt.chargingPower);
    modbusTcpServer_.inputRegisterWrite(register_map::MpptStatus, mppt.statusRaw);
    modbusTcpServer_.inputRegisterWrite(register_map::ContactorFeedback, contactorFeedback);
    modbusTcpServer_.inputRegisterWrite(register_map::FaultStatus, faultStatus);
    modbusTcpServer_.inputRegisterWrite(
        register_map::MpptFaultHighWord,
        static_cast<std::uint16_t>(mppt.faultRaw >> 16));
    modbusTcpServer_.inputRegisterWrite(
        register_map::MpptFaultLowWord,
        static_cast<std::uint16_t>(mppt.faultRaw & 0xFFFFu));
    modbusTcpServer_.inputRegisterWrite(register_map::GatewayStatus, gatewayStatus);
    modbusTcpServer_.inputRegisterWrite(register_map::RtuErrorCount, rtuErrorCount);
    modbusTcpServer_.inputRegisterWrite(register_map::MpptSoc, mppt.soc);
    modbusTcpServer_.inputRegisterWrite(register_map::PvVoltage, mppt.pvVoltageRaw);
    modbusTcpServer_.inputRegisterWrite(register_map::PvCurrent, mppt.pvCurrentRaw);
    modbusTcpServer_.inputRegisterWrite(register_map::LoadVoltage, mppt.loadVoltageRaw);
    modbusTcpServer_.inputRegisterWrite(register_map::LoadCurrent, mppt.loadCurrentRaw);
    modbusTcpServer_.inputRegisterWrite(register_map::LoadPower, mppt.loadPower);
    modbusTcpServer_.inputRegisterWrite(register_map::MpptTemperatureRaw, mppt.temperatureRaw);

    for (std::size_t index = 0; index < mppt.registers.size(); ++index) {
        modbusTcpServer_.inputRegisterWrite(
            static_cast<std::uint16_t>(register_map::MpptRawRegisterBase + index),
            mppt.registers[index]);
    }
    for (std::size_t index = 0; index < mppt.systemInformation.size(); ++index) {
        modbusTcpServer_.inputRegisterWrite(
            static_cast<std::uint16_t>(
                register_map::MpptSystemInformationRawBase + index),
            mppt.systemInformation[index]);
    }
}

bool ModbusTcpGateway::allowedIpcClient(EthernetClient& client) const {
    return !config_.enforceIpcSourceIp || client.remoteIP() == config_.ipcIp;
}

std::uint16_t ModbusTcpGateway::readDigitalInputs() const {
    std::uint16_t result = 0;
    for (std::uint8_t index = 0; index < 8; ++index) {
        if (digitalRead(config_.digitalInputPins[index]) == HIGH) {
            result |= static_cast<std::uint16_t>(1u << index);
        }
    }
    return result;
}

} // namespace opta
} // namespace solar
