#include "GatewayConfig.hpp"

namespace solar {
namespace opta {

GatewayConfig::GatewayConfig()
    : mpptSlaveId(1),
      mpptBaudRate(9600),
      mpptPollIntervalMs(1000),
      mpptResponseTimeoutMs(300),
      localIp(192, 168, 10, 20),
      dns(192, 168, 10, 1),
      gateway(192, 168, 10, 1),
      subnet(255, 255, 255, 0),
      enforceIpcSourceIp(true),
      ipcIp(192, 168, 10, 10),
      modbusTcpPort(502),
      ipcHeartbeatTimeoutMs(3000),
      mpptSystemInformationAddress(0x000A),
      mpptSystemInformationCount(17),
      mpptSystemDataAddress(0x0100),
      mpptRegisterCount(35),
      useAuxiliaryContactorFeedback(false),
      relayPins{D0, D1, D2, D3},
      contactorFeedbackPins{A0, A1, A2, A3},
      digitalInputPins{A0, A1, A2, A3, A4, A5, A6, A7} {
    const byte configuredMac[6] = {0x02, 0x53, 0x50, 0x48, 0x01, 0x01};
    for (std::uint8_t index = 0; index < 6; ++index) {
        mac[index] = configuredMac[index];
    }
}

} // namespace opta
} // namespace solar
