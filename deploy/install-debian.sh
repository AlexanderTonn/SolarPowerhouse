#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SOURCE_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
BUILD_DIR=""
START_SERVICES=1
SKIP_DEPENDENCIES=0

usage() {
    cat <<'USAGE'
Usage: deploy/install-debian.sh [options]

Builds and installs Solar Powerhouse on Debian.

Options:
  --source-dir PATH   Use PATH as the repository source directory.
  --no-start          Install and enable services without starting them.
  --skip-dependencies Skip apt dependency installation.
  -h, --help          Show this help.
USAGE
}

die() {
    printf 'ERROR: %s\n' "$*" >&2
    exit 1
}

require_root() {
    if [ "$(id -u)" -ne 0 ]; then
        die "Run this installer as root, for example: sudo $0"
    fi
}

require_debian() {
    [ -r /etc/os-release ] || die "/etc/os-release is missing"
    # shellcheck disable=SC1091
    . /etc/os-release
    [ "${ID:-}" = "debian" ] || die "This installer supports Debian; detected ${ID:-unknown}"
}

install_available_packages() {
    local package
    local -a packages=()
    for package in "$@"; do
        if apt-cache show "$package" >/dev/null 2>&1; then
            packages+=("$package")
        else
            printf 'NOTICE: optional package not available: %s\n' "$package" >&2
        fi
    done
    if [ "${#packages[@]}" -gt 0 ]; then
        DEBIAN_FRONTEND=noninteractive apt-get install --yes "${packages[@]}"
    fi
}

install_dependencies() {
    export DEBIAN_FRONTEND=noninteractive
    apt-get update
    apt-get install --yes \
        cmake \
        ninja-build \
        g++ \
        pkg-config \
        libsqlite3-dev \
        qt6-base-dev \
        qt6-declarative-dev \
        qt6-declarative-dev-tools \
        libqt6sql6-sqlite \
        polkitd \
        systemd

    install_available_packages \
        qt6-qpa-plugins \
        libqt6quickcontrols2-6 \
        libqt6quickcontrols2impl6 \
        qml6-module-qtqml \
        qml6-module-qtquick \
        qml6-module-qtquick-controls \
        qml6-module-qtquick-layouts \
        qml6-module-qtquick-window \
        libegl1 \
        libgbm1 \
        libinput10 \
        libxkbcommon0 \
        libdrm2 \
        libgl1-mesa-dri
}

prepare_user_and_directories() {
    if ! getent group solarpowerhouse >/dev/null; then
        groupadd --system solarpowerhouse
    fi
    if ! id solarpowerhouse >/dev/null 2>&1; then
        useradd \
            --system \
            --gid solarpowerhouse \
            --home-dir /var/lib/solarpowerhouse \
            --create-home \
            --shell /usr/sbin/nologin \
            solarpowerhouse
    fi

    local optional_group
    for optional_group in video render; do
        if getent group "$optional_group" >/dev/null; then
            usermod --append --groups "$optional_group" solarpowerhouse
        fi
    done

    install -d -o solarpowerhouse -g solarpowerhouse -m 0750 \
        /var/lib/solarpowerhouse \
        /var/lib/solarpowerhouse/hmi-config \
        /var/log/solarpowerhouse
}

build_repository() {
    [ -f "$SOURCE_DIR/CMakeLists.txt" ] || die "CMakeLists.txt not found in $SOURCE_DIR"
    BUILD_DIR=$(mktemp -d /tmp/solarpowerhouse-build.XXXXXX)
    trap 'if [ -n "${BUILD_DIR:-}" ] && [ -d "$BUILD_DIR" ]; then rm -rf "$BUILD_DIR"; fi' EXIT

    cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=ON \
        -DCMAKE_INSTALL_PREFIX=/usr/local
    cmake --build "$BUILD_DIR" --parallel
    ctest --test-dir "$BUILD_DIR" --output-on-failure
    [ -x "$BUILD_DIR/solar-core" ] || die "solar-core was not built"
    [ -x "$BUILD_DIR/solar-hmi" ] || die "solar-hmi was not built; check Qt6 dependencies"
}

install_repository() {
    cmake --install "$BUILD_DIR" --prefix /usr/local

    install -D -o root -g root -m 0644 \
        "$SOURCE_DIR/deploy/50-solarpowerhouse.rules" \
        /etc/polkit-1/rules.d/50-solarpowerhouse.rules

    if [ ! -e /var/lib/solarpowerhouse/modbus-mapping.conf ]; then
        install -o solarpowerhouse -g solarpowerhouse -m 0640 \
            "$SOURCE_DIR/config/modbus-mapping.conf" \
            /var/lib/solarpowerhouse/modbus-mapping.conf
    fi
    if [ ! -e /var/lib/solarpowerhouse/modbus-connection.conf ]; then
        install -o solarpowerhouse -g solarpowerhouse -m 0640 \
            "$SOURCE_DIR/config/modbus-connection.conf" \
            /var/lib/solarpowerhouse/modbus-connection.conf
    fi
    if [ ! -e /var/lib/solarpowerhouse/telemetry-history.conf ]; then
        install -o solarpowerhouse -g solarpowerhouse -m 0640 \
            "$SOURCE_DIR/config/telemetry-history.conf" \
            /var/lib/solarpowerhouse/telemetry-history.conf
    fi
    chown -R solarpowerhouse:solarpowerhouse /var/lib/solarpowerhouse /var/log/solarpowerhouse
}

activate_services() {
    systemctl daemon-reload
    systemctl enable solar-powerhouse.target
    if [ "$START_SERVICES" -eq 1 ]; then
        systemctl start solar-powerhouse.target
    fi
    if systemctl cat polkit.service >/dev/null 2>&1; then
        systemctl try-restart polkit.service || true
    fi
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --source-dir)
            [ "$#" -ge 2 ] || die "--source-dir requires a path"
            SOURCE_DIR=$(CDPATH= cd -- "$2" && pwd)
            shift 2
            ;;
        --no-start)
            START_SERVICES=0
            shift
            ;;
        --skip-dependencies)
            SKIP_DEPENDENCIES=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            die "Unknown option: $1"
            ;;
    esac
done

require_root
require_debian
if [ "$SKIP_DEPENDENCIES" -eq 0 ]; then
    install_dependencies
fi
prepare_user_and_directories
build_repository
install_repository
activate_services

printf '%s\n' 'Solar Powerhouse installation completed.'
printf '%s\n' 'Services: solar-core.service, solar-hmi.service, solar-powerhouse.target'
printf '%s\n' 'Configuration: /var/lib/solarpowerhouse/modbus-connection.conf'
