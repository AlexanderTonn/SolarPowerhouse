# IPC field-gateway register map

Zero-based Modbus TCP offsets shared by `solar-core` and the Arduino Opta
gateway. Confirm address base, word order, and physical feedback wiring before
commissioning.

## Input registers: Opta -> IPC

The IPC reads these registers with Modbus TCP function `04`.

| Offset | Name | Encoding |
|---:|---|---|
| 0 | Digital inputs | Opta `A0..A7`, `uint16` bitfield |
| 1 | Analog input 1 | Reserved, `0` |
| 2 | Battery voltage | V x 100, `uint16` |
| 3 | Charging current | A x 100; raw MPPT `0x0102` value, `uint16` |
| 4-5 | PV charging power | 32-bit W, high word then low word |
| 6 | MPPT status | Raw Rover register `0x0120` |
| 7 | Contactor/relay state | K1/K2/K3/K4 bits; physical aux feedback only when wired/configured |
| 8 | Gateway fault status | Gateway fault bitfield |
| 9-10 | MPPT fault | Raw Rover fault bits 31..0 |
| 11 | Gateway status | bit 0 MPPT online; bit 1 IPC heartbeat fresh; bit 2 IPC seen |
| 12 | RTU error count | Saturating `uint16` |
| 13 | MPPT state of charge | Percent |
| 14 | PV voltage | V x 10 |
| 15 | PV current | A x 100 |
| 16 | Load voltage | V x 10 |
| 17 | Load current | A x 100 |
| 18 | Load power | W |
| 19 | MPPT temperature | Raw register `0x0103`; decode according to controller revision |
| 20-54 | Raw MPPT register span | Registers `0x0100..0x0122`; write-only `0x010A` is zero |
| 55-71 | Raw MPPT system-information block | Registers `0x000A..0x001A`, one input offset per MPPT register |

The Opta reads MPPT registers `0x000A..0x001A` and `0x0100..0x0122` using
Modbus function `03` on the RTU side. Register `0x010A` is a write-only load
command in the controller manual; it is not read or written in this milestone.

## Holding registers: IPC -> Opta

| Offset | Name | Encoding |
|---:|---|---|
| 100 | Requested contactors | bit 0 K1, bit 1 K2, bit 2 K3, bit 3 K4 |
| 101 | IPC heartbeat counter | Incrementing `uint16` |
| 102 | Command sequence number | Increment after each new command |

The Opta is an I/O/protocol gateway. It does not implement battery-threshold
energy-management policy, operating modes, MQTT, or source selection. It only
rejects malformed/interlocked contactor words and opens all relays when the IPC
heartbeat expires. The engineered electrical interlock and any required PLC
watchdog remain mandatory.
