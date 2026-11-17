#include "infrastructure/OptaModbusTcp.hpp"

#include <array>
#include <cerrno>
#include <cstring>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <utility>

namespace solar::infrastructure {

namespace {

constexpr std::uint16_t mpptRawInputBase = 20;
constexpr std::uint16_t systemInformationRawInputBase = 55;
constexpr std::size_t maxModbusTcpPdu = 253;
constexpr int socketTimeoutMilliseconds = 500;

std::uint16_t readU16(const std::uint8_t* bytes) noexcept {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[0]) << 8) | bytes[1]);
}

void writeU16(std::uint8_t* bytes, std::uint16_t value) noexcept {
    bytes[0] = static_cast<std::uint8_t>(value >> 8);
    bytes[1] = static_cast<std::uint8_t>(value & 0xFFu);
}

std::uint32_t combineU32(std::uint16_t high, std::uint16_t low) noexcept {
    return (static_cast<std::uint32_t>(high) << 16) | low;
}

double voltage(std::uint16_t value) noexcept {
    return static_cast<double>(value) * 0.1;
}

double current(std::uint16_t value) noexcept {
    return static_cast<double>(value) * 0.01;
}

double signedTemperature(std::uint8_t value) noexcept {
    const auto magnitude = static_cast<double>(value & 0x7Fu);
    return (value & 0x80u) != 0 ? -magnitude : magnitude;
}

std::string decodeText(const std::array<std::uint16_t, SystemInformationRegisterCount>& registers,
                       std::size_t start,
                       std::size_t count) {
    std::string result;
    result.reserve(count * 2);
    for (std::size_t index = 0; index < count; ++index) {
        const auto value = registers[start + index];
        const auto high = static_cast<char>(value >> 8);
        const auto low = static_cast<char>(value & 0xFFu);
        if (high != '\0' && high != ' ') {
            result.push_back(high);
        }
        if (low != '\0' && low != ' ') {
            result.push_back(low);
        }
    }
    return result;
}

} // namespace

const ModbusMappingDefinition& toyoRenogyMapping() noexcept {
    static constexpr ModbusMappingDefinition definition{
        ModbusMappingId::ToyoRenogy,
        "TOYO_RENOGY",
        "TOYO / RENOGY",
        "192.168.10.20",
        502,
        1,
        0,
        static_cast<std::uint16_t>(OptaInputRegisterCount),
        100,
        101,
        102,
    };
    return definition;
}

const ModbusMappingDefinition& mappingDefinition(ModbusMappingId id) noexcept {
    switch (id) {
    case ModbusMappingId::ToyoRenogy:
        return toyoRenogyMapping();
    }
    return toyoRenogyMapping();
}

std::optional<ModbusMappingId> mappingFromKey(std::string_view key) noexcept {
    if (key == toyoRenogyMapping().key || key == "TOYO / RENOGY") {
        return ModbusMappingId::ToyoRenogy;
    }
    return std::nullopt;
}

std::string_view mappingKey(ModbusMappingId id) noexcept {
    return mappingDefinition(id).key;
}

std::string_view mappingLabel(ModbusMappingId id) noexcept {
    return mappingDefinition(id).label;
}

MpptReadOnlyData decodeToyoRenogy(
    const std::array<std::uint16_t, MpptRegisterCount>& registers) noexcept {
    MpptReadOnlyData result{};
    result.stateOfChargePercent = registers[0];
    result.batteryVoltage = voltage(registers[1]);
    result.chargingCurrent = current(registers[2]);
    result.controllerTemperature = signedTemperature(static_cast<std::uint8_t>(registers[3] >> 8));
    result.batteryTemperature = signedTemperature(static_cast<std::uint8_t>(registers[3] & 0xFFu));
    result.loadVoltage = voltage(registers[4]);
    result.loadCurrent = current(registers[5]);
    result.loadPower = registers[6];
    result.solarVoltage = voltage(registers[7]);
    result.solarCurrent = current(registers[8]);
    result.chargingPower = registers[9];

    // Register 0x010A is a write-only load command and is intentionally not
    // exposed as a read-only measurement.
    result.dailyMinimumBatteryVoltage = voltage(registers[11]);
    result.dailyMaximumBatteryVoltage = voltage(registers[12]);
    result.dailyMaximumChargingCurrent = current(registers[13]);
    result.dailyMaximumDischargingCurrent = current(registers[14]);
    result.dailyMaximumChargingPower = registers[15];
    result.dailyMaximumDischargingPower = registers[16];
    result.dailyChargingAmpHours = registers[17];
    result.dailyDischargingAmpHours = registers[18];
    result.dailyEnergyGenerationKwh = static_cast<double>(registers[19]) / 10000.0;
    result.dailyEnergyConsumptionKwh = static_cast<double>(registers[20]) / 10000.0;
    result.totalOperatingDays = registers[21];
    result.totalBatteryOverDischarges = registers[22];
    result.totalBatteryFullCharges = registers[23];
    result.totalChargingAmpHours = combineU32(registers[24], registers[25]);
    result.totalDischargingAmpHours = combineU32(registers[26], registers[27]);
    result.cumulativeEnergyGenerationKwh = static_cast<double>(combineU32(registers[28], registers[29])) / 10000.0;
    result.cumulativeEnergyConsumptionKwh = static_cast<double>(combineU32(registers[30], registers[31])) / 10000.0;

    const auto status = registers[32];
    const auto statusHigh = static_cast<std::uint8_t>(status >> 8);
    result.streetLightOn = (statusHigh & 0x80u) != 0;
    result.streetLightBrightnessPercent = static_cast<std::uint8_t>(statusHigh & 0x7Fu);
    result.chargingState = static_cast<std::uint8_t>(status & 0xFFu);
    result.controllerFaults = combineU32(registers[33], registers[34]);
    return result;
}

MpptReadOnlyData decodeToyoRenogy(
    const std::array<std::uint16_t, SystemInformationRegisterCount>& systemInformation,
    const std::array<std::uint16_t, MpptRegisterCount>& registers) {
    auto result = decodeToyoRenogy(registers);
    result.maximumSystemVoltage = static_cast<std::uint8_t>(systemInformation[0] >> 8);
    result.ratedChargingCurrent = static_cast<std::uint8_t>(systemInformation[0] & 0xFFu);
    result.ratedDischargingCurrent = static_cast<std::uint8_t>(systemInformation[1] >> 8);
    result.productType = static_cast<std::uint8_t>(systemInformation[1] & 0xFFu);
    result.productModel = decodeText(systemInformation, 2, 8);
    result.softwareVersion = decodeText(systemInformation, 10, 2);
    result.hardwareVersion = decodeText(systemInformation, 12, 2);
    result.serialNumber = decodeText(systemInformation, 14, 2);
    result.deviceAddress = static_cast<std::uint8_t>(systemInformation[16] & 0xFFu);
    return result;
}

OptaModbusTcpClient::OptaModbusTcpClient(ModbusMappingId mapping)
    : mapping_(mappingDefinition(mapping)) {}

OptaModbusTcpClient::~OptaModbusTcpClient() {
    stop();
}

bool OptaModbusTcpClient::connectSocket() {
    if (socket_ >= 0) {
        return true;
    }

    socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        setError(std::string("Modbus TCP socket failed: ") + std::strerror(errno));
        return false;
    }

    const timeval timeout{
        socketTimeoutMilliseconds / 1000,
        (socketTimeoutMilliseconds % 1000) * 1000,
    };
    setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(mapping_.port);

    const std::string host(mapping_.host);
    if (::inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1) {
        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        addrinfo* resolved = nullptr;
        if (::getaddrinfo(host.c_str(), nullptr, &hints, &resolved) != 0 || resolved == nullptr) {
            setError("Cannot resolve Modbus TCP host: " + host);
            closeSocket();
            return false;
        }
        address.sin_addr = reinterpret_cast<sockaddr_in*>(resolved->ai_addr)->sin_addr;
        ::freeaddrinfo(resolved);
    }

    if (::connect(socket_, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0) {
        setError(std::string("Modbus TCP connect failed: ") + std::strerror(errno));
        closeSocket();
        return false;
    }
    return true;
}

void OptaModbusTcpClient::closeSocket() noexcept {
    if (socket_ >= 0) {
        ::close(socket_);
        socket_ = -1;
    }
}

void OptaModbusTcpClient::stop() noexcept {
    closeSocket();
}

bool OptaModbusTcpClient::readExact(std::uint8_t* buffer, std::size_t size) {
    std::size_t offset = 0;
    while (offset < size) {
        const auto received = ::recv(socket_, buffer + offset, size - offset, 0);
        if (received <= 0) {
            setError(std::string("Modbus TCP receive failed: ") + std::strerror(errno));
            closeSocket();
            return false;
        }
        offset += static_cast<std::size_t>(received);
    }
    return true;
}

bool OptaModbusTcpClient::writeAll(const std::uint8_t* buffer, std::size_t size) {
    std::size_t offset = 0;
    while (offset < size) {
        const auto sent = ::send(socket_, buffer + offset, size - offset, 0);
        if (sent <= 0) {
            setError(std::string("Modbus TCP send failed: ") + std::strerror(errno));
            closeSocket();
            return false;
        }
        offset += static_cast<std::size_t>(sent);
    }
    return true;
}

bool OptaModbusTcpClient::readInputRegisters(
    std::uint16_t start,
    std::uint16_t count,
    std::array<std::uint16_t, OptaInputRegisterCount>& values) {
    if (!connectSocket()) {
        return false;
    }

    std::array<std::uint8_t, 12> request{};
    const auto transaction = ++transactionId_;
    writeU16(request.data(), transaction);
    writeU16(request.data() + 2, 0);
    writeU16(request.data() + 4, 6);
    request[6] = mapping_.unitId;
    request[7] = 0x04; // Read input registers.
    writeU16(request.data() + 8, start);
    writeU16(request.data() + 10, count);

    if (!writeAll(request.data(), request.size())) {
        return false;
    }

    std::array<std::uint8_t, 7> header{};
    if (!readExact(header.data(), header.size())) {
        return false;
    }
    const auto responseTransaction = readU16(header.data());
    const auto protocol = readU16(header.data() + 2);
    const auto length = readU16(header.data() + 4);
    if (responseTransaction != transaction || protocol != 0 || length < 2 || length > maxModbusTcpPdu) {
        setError("Invalid Modbus TCP response header");
        closeSocket();
        return false;
    }

    std::array<std::uint8_t, maxModbusTcpPdu> pdu{};
    const auto pduSize = static_cast<std::size_t>(length - 1);
    if (!readExact(pdu.data(), pduSize)) {
        return false;
    }
    if (header[6] != mapping_.unitId || pdu[0] != 0x04 || pdu[1] != count * 2) {
        setError("Invalid Modbus TCP input-register response");
        closeSocket();
        return false;
    }

    for (std::uint16_t index = 0; index < count; ++index) {
        values[index] = readU16(pdu.data() + 2 + index * 2);
    }
    return true;
}

bool OptaModbusTcpClient::writeHoldingRegisters(
    std::uint16_t start,
    const std::array<std::uint16_t, 3>& values) {
    if (!connectSocket()) {
        return false;
    }

    std::array<std::uint8_t, 19> request{};
    const auto transaction = ++transactionId_;
    writeU16(request.data(), transaction);
    writeU16(request.data() + 2, 0);
    writeU16(request.data() + 4, 13);
    request[6] = mapping_.unitId;
    request[7] = 0x10; // Write multiple holding registers.
    writeU16(request.data() + 8, start);
    writeU16(request.data() + 10, static_cast<std::uint16_t>(values.size()));
    request[12] = static_cast<std::uint8_t>(values.size() * 2);
    for (std::size_t index = 0; index < values.size(); ++index) {
        writeU16(request.data() + 13 + index * 2, values[index]);
    }

    if (!writeAll(request.data(), request.size())) {
        return false;
    }

    std::array<std::uint8_t, 7> header{};
    if (!readExact(header.data(), header.size())) {
        return false;
    }
    const auto responseTransaction = readU16(header.data());
    const auto protocol = readU16(header.data() + 2);
    const auto length = readU16(header.data() + 4);
    if (responseTransaction != transaction || protocol != 0 || length < 2 || length > maxModbusTcpPdu) {
        setError("Invalid Modbus TCP write response header");
        closeSocket();
        return false;
    }

    std::array<std::uint8_t, maxModbusTcpPdu> pdu{};
    const auto pduSize = static_cast<std::size_t>(length - 1);
    if (!readExact(pdu.data(), pduSize)) {
        return false;
    }
    if (header[6] != mapping_.unitId || pdu[0] != 0x10
        || readU16(pdu.data() + 1) != start
        || readU16(pdu.data() + 3) != values.size()) {
        setError("Invalid Modbus TCP holding-register response");
        closeSocket();
        return false;
    }
    return true;
}

bool OptaModbusTcpClient::poll(OptaTelemetry& telemetry) {
    std::array<std::uint16_t, OptaInputRegisterCount> values{};
    if (!readInputRegisters(
            mapping_.inputRegisterStart,
            mapping_.inputRegisterCount,
            values)) {
        telemetry = {};
        telemetry.error = lastError_;
        return false;
    }

    telemetry = {};
    telemetry.connected = true;
    telemetry.inputRegisters = values;
    telemetry.digitalInputs = values[0];
    telemetry.contactorState = values[7];
    telemetry.gatewayFaults = values[8];
    telemetry.gatewayStatus = values[11];
    telemetry.rtuErrorCount = values[12];

    std::array<std::uint16_t, SystemInformationRegisterCount> systemInformation{};
    for (std::size_t index = 0; index < systemInformation.size(); ++index) {
        systemInformation[index] = values[systemInformationRawInputBase + index];
    }

    std::array<std::uint16_t, MpptRegisterCount> mpptRegisters{};
    for (std::size_t index = 0; index < mpptRegisters.size(); ++index) {
        mpptRegisters[index] = values[mpptRawInputBase + index];
    }
    telemetry.mppt = decodeToyoRenogy(systemInformation, mpptRegisters);
    telemetry.error.clear();
    return true;
}

bool OptaModbusTcpClient::safeShutdown() {
    ++heartbeat_;
    ++commandSequence_;
    const std::array<std::uint16_t, 3> safeValues{
        0,
        heartbeat_,
        commandSequence_,
    };
    const auto result = writeHoldingRegisters(mapping_.requestedContactorsRegister, safeValues);
    closeSocket();
    return result;
}

void OptaModbusTcpClient::setError(std::string error) {
    lastError_ = std::move(error);
}

} // namespace solar::infrastructure
