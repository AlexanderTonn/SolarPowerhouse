# Arduino Opta MPPT gateway

This project turns an Arduino Opta RS485 into a narrow field gateway:

1. Read the SR-ML24/Rover-compatible MPPT controller over Modbus RTU.
2. Expose MPPT values and gateway diagnostics as Modbus TCP input registers.
3. Apply the IPC contactor command word to Opta relays `D0..D3`.
4. Open all relays if the IPC heartbeat expires.

The Opta does not implement automatic energy management, battery thresholds,
operating modes, MQTT, or switching policy. Those remain in `solar-core`.

## RS485 connection

The previous project version successfully used the controller's RS485
interface directly through `ArduinoRS485`. This firmware follows that proven
configuration:

- MPPT RS485 A/B -> Opta RS485 A/B; verify polarity and termination.
- Use the controller's documented RS485 terminals only.
- Default serial format: `9600 8N2`, MPPT slave ID `1`.
- The gateway applies a 3.5-character Modbus RTU silent interval after
  `ModbusRTUClient.begin()`; the library reinitializes the RS485 transport in
  `begin()`.

The supplied Rover protocol document does not specify every serial setting;
verify address and baud rate on the actual controller before commissioning.

## PlatformIO setup

Open `firmware/opta-mppt-gateway/` as a PlatformIO project. The project uses
the PlatformIO board ID `opta`, Arduino framework, ArduinoModbus, and
ArduinoRS485.

Build and upload from the repository root:

```sh
cd firmware/opta-mppt-gateway
pio run -e opta
pio run -e opta -t upload
pio device monitor -b 115200
```

The default upload protocol is DFU. If a different programmer is connected,
override it on the command line, for example:

```sh
pio run -e opta -t upload --upload-protocol cmsis-dap
```

The implementation is split into focused C++ classes under `src/`:

- `OptaMpptGateway`: lifecycle, scheduling, watchdog, and orchestration
- `MpptRtuClient`: MPPT Modbus-RTU transport and snapshot acquisition
- `ModbusTcpGateway`: Panel-PC Modbus-TCP server and register publication
- `RelayController`: output handling, feedback, and local interlock checks
- `GatewayConfig` and `ModbusRegisterMap`: configuration and protocol contract

There are no Arduino sketch files. PlatformIO builds `src/main.cpp` as the
Arduino framework entry point.

## Configuration

Change the static Ethernet address, IPC address, MPPT slave ID, and serial
settings in `src/GatewayConfig.cpp` before commissioning.

The Modbus TCP write path is restricted in firmware to the configured IPC IP.
This is not a replacement for a protected control network or firewall.

## Modbus TCP map

All addresses are zero-based library addresses.

### Input registers: Opta -> IPC

The IPC reads these registers with Modbus TCP function `04`.

| Address | Name | Encoding |
|---:|---|---|
| 0 | Digital inputs | Opta `A0..A7`, bitfield |
| 1 | Analog input 1 | Reserved, `0` |
| 2 | Battery voltage | V x 100 |
| 3 | Charging current | A x 100; raw MPPT `0x0102` value |
| 4 | PV power high word | W, high word |
| 5 | PV power low word | W, low word |
| 6 | MPPT status | Raw register `0x0120` |
| 7 | Contactor/relay state | Commanded relay bits, unless auxiliary feedback is enabled |
| 8 | Gateway fault status | Gateway fault bitfield |
| 9 | MPPT fault high word | Raw MPPT fault bits 31..16 |
| 10 | MPPT fault low word | Raw MPPT fault bits 15..0 |
| 11 | Gateway status | bit 0 MPPT online, bit 1 heartbeat fresh, bit 2 IPC connected |
| 12 | RTU error count | Saturating `uint16` counter |
| 13 | MPPT state of charge | Percent |
| 14 | PV voltage | V x 10 |
| 15 | PV current | A x 100 |
| 16 | Load voltage | V x 10 |
| 17 | Load current | A x 100 |
| 18 | Load power | W |
| 19 | MPPT temperature | Raw register `0x0103`; decode according to controller revision |
| 20..54 | Raw MPPT register span | `0x0100..0x0122`; write-only `0x010A` is zero |
| 55..71 | Raw MPPT system-information block | `0x000A..0x001A`, one input register per MPPT register |

MPPT values are read from the Rover registers `0x000A..0x001A` and
`0x0100..0x0122`, including system information, daily, and cumulative
read-only values, using Modbus function `03` on the RTU side. Complete raw
blocks are forwarded through the Opta input registers.
The write-only load command register `0x010A` is not read or written in this
milestone; its corresponding raw-span slot remains zero.

### Holding registers: IPC -> Opta

| Address | Name | Encoding |
|---:|---|---|
| 100 | Requested contactors | bit 0 K1, bit 1 K2, bit 2 K3, bit 3 K4 |
| 101 | IPC heartbeat | Incrementing counter |
| 102 | Command sequence | Increment after every new contactor command |

The gateway rejects K1/K2 mismatch and simultaneous K2/K3 requests. It opens
all relays on invalid commands or a stale heartbeat. This is only a local I/O
fail-safe; the engineered electrical interlock and PLC safety design remain
mandatory.

The relay command is applied only after a valid IPC heartbeat has been seen.
The heartbeat must change within 3 seconds; otherwise the gateway de-energizes
`D0..D3`. A command sequence change is applied on the next Modbus service pass.
The current relay word is published at input register `7`, while gateway and
MPPT fault bits are published at input register `8`.

## Commissioning checklist

1. Confirm the MPPT slave address, `9600 8N2` serial format, RS485 A/B polarity,
   reference conductor, and termination.
2. Set `localIp` and `ipcIp` to the protected control-network addresses.
3. Verify that every relay is de-energized after reset, invalid commands, and
   heartbeat timeout.
4. Read input registers `0..71` from the panel PC and compare raw values with
   the MPPT display before enabling switching.
5. Write only the three holding registers `100..102`; never expose TCP/502
   beyond the isolated control network.
