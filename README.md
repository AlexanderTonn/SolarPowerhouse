# Solar Powerhouse

Embedded energy-management platform for routing a shared PV source between an MPPT charge controller and a grid-connected microinverter.

Current baseline follows the supplied software architecture draft:

- C++20 core with testable domain logic.
- Non-blocking contactor switching FSM with feedback timeouts and K2/K3 interlock checks.
- Battery-voltage hysteresis with confirmation time, stale-data rejection, and minimum dwell time.
- Versioned register and MQTT topic contracts.
- K4 island-inverter DC undervoltage protection logic with hysteresis, confirmation time, feedback timeout, and tests.
- Qt Quick/QML HMI skeleton, enabled when Qt 6 is available.
- Commissioning-safe default config: `null` voltage thresholds inhibit automatic switching.
- HMI weather header with editable location and current conditions.
- Persistent SQLite telemetry history for the island live chart. Sampling and
  batched database write intervals are configurable from `Setup`; the HMI
  displays calculated batch size and RAM payload estimate.
- HMI menu separation: control-logic `Settings` and system `Setup` page.
- HMI language switcher with German and English flag icons; German is the
  default and the selected language is persisted locally.
- Responsive dashboard layout with dedicated system-status card and persistent
  red active-alarm banners for Modbus loss, backend loss, MPPT faults, and the
  configured undervoltage-protection state.
- Role-aware HMI setup access foundation; password authentication remains
  disabled until secure credential provisioning is defined.
- Arduino Opta RS485 gateway with TOYO / RENOGY Modbus-RTU mapping, complete
  MPPT system/daily/cumulative telemetry over Modbus TCP, and fail-safe relay
  command handling from the IPC.

Development runs on macOS with Qt 6. Production target is Linux/Debian. Target-host installation is intentionally a separate operational step until host address, credentials, and update policy are defined.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

## Debian package

Build the native Debian package on a Debian build host:

```sh
./deploy/build-debian-package.sh
```

The result is one file under `dist/`, for example
`solarpowerhouse_0.1.0_amd64.deb`. Copy that file to the target and install it
with APT so Debian resolves the declared Qt, SQLite, systemd, Polkit, and QML
runtime dependencies:

```sh
sudo apt install ./solarpowerhouse_0.1.0_amd64.deb
```

The package contains both programs, the default configuration, systemd units,
the Polkit rule, and maintainer scripts. Installation creates the dedicated
`solarpowerhouse` account, preserves existing files under
`/var/lib/solarpowerhouse`, and enables/starts `solar-powerhouse.target`.

The same package can be built from GitHub Actions: open the `Debian package`
workflow, select `Run workflow`, and download the uploaded
`solarpowerhouse-debian-<commit>` artifact.

Qt Installer Framework creates self-contained Linux installer binaries, not
native Debian packages. Therefore the deliverable for the one-file Debian
workflow is the native `.deb` above; a QIF installer would be a separate
`.run` artifact and cannot be installed with Debian's package manager.

## Debian source installation

From the repository root, run the idempotent Debian installer as root:

```sh
sudo ./deploy/install-debian.sh
```

The installer updates APT, installs the CMake/Qt/SQLite/systemd/Polkit
dependencies, creates the dedicated `solarpowerhouse` service account, builds
and tests both `solar-core` and `solar-hmi`, installs the programs and service
units, installs the Polkit rule, creates the persistent configuration directory,
and enables and starts `solar-powerhouse.target`.

To install without starting the services immediately:

```sh
sudo ./deploy/install-debian.sh --no-start
```

The installer preserves existing files under `/var/lib/solarpowerhouse`. The
Opta firmware is not flashed by this installer; upload the PlatformIO project
from `firmware/opta-mppt-gateway/` separately after verifying the field-network
configuration.

Qt 6 is optional for the first core-only milestone. Without Qt, `solar-core` and `solar-core-tests` still build.

Qt Creator can use the installed Qt kit directly. QML builds require CMake automoc; this is enabled in `CMakeLists.txt` for Qt 6 QML metatype generation.

`solar-core` is a long-running service. Stop it with `Ctrl+C` locally; systemd sends `SIGTERM` during service shutdown.

Weather uses Open-Meteo geocoding and current-weather endpoints. Internet access is required; the weather display is informational and never participates in switching decisions. The selected HMI location is persisted locally until the core settings/IPC path replaces this temporary HMI-side storage.

The K4 protection controller is implemented against `IHardwareInterface`. Exact Modbus register addresses, PLC bit ownership, feedback wiring, and the approved safe output behavior remain intentionally unassigned until the project-specific register map is provided.

The current field adapter reads the Opta Modbus TCP input-register map from
`0..71` and publishes all documented TOYO / RENOGY read-only values to
`solar-core`. The Arduino gateway
accepts contactor commands in holding registers `100..102`: requested relay
bits, IPC heartbeat, and command sequence. The gateway rejects invalid source
combinations and opens every relay when the heartbeat expires. The MPPT
gateway's write-only load command is not used as an application control value.
The HMI receives the current state over a local Unix socket at
`/run/solarpowerhouse/telemetry.sock`; no JSON state file is written. The
in-memory IPC message contains only the latest state: connection/error status,
timestamps, mapping information, the decoded MPPT values, all 72 raw input
registers, and history bookkeeping such as the pending-sample count and latest
sample. It does not contain the complete history; that remains in SQLite.

The HMI can persist the Modbus TCP gateway IP, subnet, gateway and Unit ID in
`/var/lib/solarpowerhouse/modbus-connection.conf`. The gateway IP and Unit ID
are used by `solar-core` after its restart. Subnet and gateway are retained as
commissioning parameters; applying them to the Linux network interface remains
an operating-system/network-manager task and is not performed by the HMI.

The core stores the complete island chart history in SQLite at
`/var/lib/solarpowerhouse/telemetry.sqlite` on Debian or
`/tmp/solarpowerhouse-telemetry.sqlite` during macOS development. Default
sampling is every 10 seconds. Samples remain in RAM until the default 5-minute
batch interval, then are committed in one transaction. Setup writes the two
values to `telemetry-history.conf`; the running core reloads them without a
process restart. The HMI opens the database read-only and loads the complete
stored history for chart zooming and time-axis panning. RAM estimate covers the
sample payload (timestamp plus four chart values), not allocator/SQLite page
overhead.
The Debian image must provide the Qt SQLite driver package `libqt6sql6-sqlite`
for the HMI's read-only history query.

The standalone Opta project is in
`firmware/opta-mppt-gateway/`. It is a PlatformIO project and is compiled by
the `opta-firmware.yml` GitHub Actions workflow. The gateway is deliberately not a
second energy-management controller: the IPC remains the owner of operating
mode and switching policy. Modbus TCP is plaintext, so the configured IPC
source-IP restriction must be combined with a dedicated control network or
firewall before commissioning.

Mapping selection is deliberately catalog-based. Step 1 contains only
`TOYO_RENOGY`; adding another mapping means adding its connection/register
definition without changing the HMI flow. A confirmed mapping reload writes the
selected key, sends the Opta safe all-zero lifecycle command, closes Modbus,
and restarts `solar-core.service`.

`Setup` contains Modbus mapping, backend service control, weather service
configuration, software version, and access status. `Settings` contains only
operating mode, switching thresholds, timing, and island-inverter protection.
The HMI `UserManager` already exposes roles and permissions for
`setup.view`, `backend.control`, and `modbus.mapping.change`; password storage
and hashing must be provisioned before enabling password enforcement.

The HMI language is changed with the DE/EN flag ComboBox in the top header. The
language manager stores the selection in the local Qt settings store. All
visible HMI labels, navigation, status text, event filters, telemetry groups,
numeric-pad actions, weather conditions, and generated HMI messages use the
selected language. Core/backend protocol identifiers remain stable in English
for logging and integration contracts.

For local visualization, select the `solar-hmi` run target in Qt Creator. `solar-core` is the backend service and intentionally has no window. Both processes are separate by design:

```sh
cmake --build build/Qt_6_12_0_for_macOS_Debug --target solar-hmi
./build/Qt_6_12_0_for_macOS_Debug/solar-hmi
```

On Debian, install the Linux package, then enable the combined systemd target. It starts both processes at graphical boot:

```sh
sudo systemctl daemon-reload
sudo systemctl enable --now solar-powerhouse.target
systemctl status solar-powerhouse.target solar-core.service solar-hmi.service
```

The HMI controls only the fixed `solar-core.service` unit through `/usr/bin/systemctl`; it never evaluates a shell command or accepts a unit name from the network. HMI stays alive while core is manually stopped, so it can restart the service and show diagnostics. Install the packaged Polkit rule to allow the dedicated `solarpowerhouse` user only `start`, `stop`, and `restart` for that one unit:

```sh
sudo install -m 0644 /usr/share/solarpowerhouse/polkit-rules/50-solarpowerhouse.rules /etc/polkit-1/rules.d/50-solarpowerhouse.rules
sudo systemctl restart polkit
```

Without that rule, HMI backend-control buttons remain unavailable; systemd boot supervision still works.

Core and HMI remain separate processes. The target provides one operational start/stop switch; `solar-core` remains independent when the HMI crashes. `QT_QPA_PLATFORM=eglfs` assumes direct fullscreen access to the touch display. Use the target's platform plugin and grant the service user `video`/`render` access when the iPC uses a different display stack.

## Repository scope

Legacy Arduino Opta/Arduino Cloud application code and generated KiCad backup/cache files were removed. Current KiCad source designs and technical reference PDFs remain for electrical review.

## Safety boundary

Architecture and software tests do not approve energizing hardware. Verify contactor ratings, DC switching duty, manufacturer sequencing, feedback wiring, PLC watchdog behavior, grid protection, and the engineered safe state before commissioning.
