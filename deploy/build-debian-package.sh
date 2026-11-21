#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SOURCE_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
OUTPUT_DIR="$SOURCE_DIR/dist"
BUILD_DIR=""
PACKAGE_ROOT=""

usage() {
    cat <<'USAGE'
Usage: deploy/build-debian-package.sh [options]

Builds one native Debian package containing Solar Powerhouse, its service
units, default configuration, and package-maintainer setup scripts.

Options:
  --source-dir PATH   Use PATH as the repository source directory.
  --output-dir PATH   Write the .deb to PATH (default: SOURCE_DIR/dist).
  -h, --help          Show this help.

Install the resulting package on Debian with:
  sudo apt install ./solarpowerhouse_<version>_<architecture>.deb
USAGE
}

die() {
    printf 'ERROR: %s\n' "$*"
    exit 1
}

cleanup() {
    if [ -n "${BUILD_DIR:-}" ] && [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
    if [ -n "${PACKAGE_ROOT:-}" ] && [ -d "$PACKAGE_ROOT" ]; then
        rm -rf "$PACKAGE_ROOT"
    fi
}
trap cleanup EXIT

require_command() {
    command -v "$1" >/dev/null 2>&1 || die "Required command not found: $1"
}

require_debian_tools() {
    require_command cmake
    require_command ctest
    require_command dpkg
    require_command dpkg-deb
    require_command install
    require_command ninja
}

read_project_version() {
    local version
    version=$(sed -n 's/^project(SolarPowerhouse VERSION \([^)]*\).*/\1/p' "$SOURCE_DIR/CMakeLists.txt" | head -1)
    [ -n "$version" ] || die "Could not determine the project version"
    printf '%s' "$version"
}

write_maintainer_scripts() {
    install -D -m 0755 /dev/stdin "$PACKAGE_ROOT/DEBIAN/postinst" <<'POSTINST'
#!/bin/sh
set -eu

if ! getent group solarpowerhouse >/dev/null 2>&1; then
    groupadd --system solarpowerhouse
fi
if ! id solarpowerhouse >/dev/null 2>&1; then
    useradd --system --gid solarpowerhouse \
        --home-dir /var/lib/solarpowerhouse --create-home \
        --shell /usr/sbin/nologin solarpowerhouse
fi

for optional_group in video render; do
    if getent group "$optional_group" >/dev/null 2>&1; then
        usermod --append --groups "$optional_group" solarpowerhouse || true
    fi
done

install -d -o solarpowerhouse -g solarpowerhouse -m 0750 \
    /var/lib/solarpowerhouse \
    /var/lib/solarpowerhouse/hmi-config \
    /var/log/solarpowerhouse

for config_file in modbus-mapping.conf modbus-connection.conf telemetry-history.conf; do
    if [ ! -e "/var/lib/solarpowerhouse/$config_file" ]; then
        install -o solarpowerhouse -g solarpowerhouse -m 0640 \
            "/usr/share/solarpowerhouse/config/$config_file" \
            "/var/lib/solarpowerhouse/$config_file"
    fi
done

chown -R solarpowerhouse:solarpowerhouse \
    /var/lib/solarpowerhouse /var/log/solarpowerhouse

if [ -d /run/systemd/system ] && command -v systemctl >/dev/null 2>&1; then
    systemctl daemon-reload || true
    systemctl enable solar-powerhouse.target || true
    if systemctl is-active --quiet solar-powerhouse.target; then
        systemctl try-restart solar-core.service solar-hmi.service || true
    else
        systemctl start solar-powerhouse.target || true
    fi
    if systemctl is-active --quiet polkit.service; then
        systemctl try-restart polkit.service || true
    fi
fi

exit 0
POSTINST

    install -D -m 0755 /dev/stdin "$PACKAGE_ROOT/DEBIAN/prerm" <<'PRERM'
#!/bin/sh
set -eu

if [ "${1:-}" = remove ] || [ "${1:-}" = deconfigure ]; then
    if [ -d /run/systemd/system ] && command -v systemctl >/dev/null 2>&1; then
        systemctl stop solar-powerhouse.target || true
        systemctl disable solar-powerhouse.target || true
    fi
fi

exit 0
PRERM

    install -D -m 0755 /dev/stdin "$PACKAGE_ROOT/DEBIAN/postrm" <<'POSTRM'
#!/bin/sh
set -eu

if [ -d /run/systemd/system ] && command -v systemctl >/dev/null 2>&1; then
    systemctl daemon-reload || true
fi

# Persistent configuration and history are intentionally retained on removal.
exit 0
POSTRM
}

build_repository() {
    [ -f "$SOURCE_DIR/CMakeLists.txt" ] || die "CMakeLists.txt not found in $SOURCE_DIR"
    BUILD_DIR=$(mktemp -d /tmp/solarpowerhouse-build.XXXXXX)
    cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=ON \
        -DCMAKE_INSTALL_PREFIX=/usr/local
    cmake --build "$BUILD_DIR" --parallel
    ctest --test-dir "$BUILD_DIR" --output-on-failure
    [ -x "$BUILD_DIR/solar-core" ] || die "solar-core was not built"
    [ -x "$BUILD_DIR/solar-hmi" ] || die "solar-hmi was not built; check Qt6 dependencies"
}

stage_package() {
    local version architecture package_name package_path
    version=$(read_project_version)
    architecture=$(dpkg --print-architecture)
    package_name="solarpowerhouse_${version}_${architecture}"
    package_path="$OUTPUT_DIR/$package_name.deb"

    PACKAGE_ROOT=$(mktemp -d /tmp/solarpowerhouse-package.XXXXXX)
    install -d "$PACKAGE_ROOT/DEBIAN"

    DESTDIR="$PACKAGE_ROOT" cmake --install "$BUILD_DIR" --prefix /usr/local --strip

    # Keep the package layout Debian-native while leaving the standalone
    # source installer unchanged.
    install -d "$PACKAGE_ROOT/usr/bin" "$PACKAGE_ROOT/usr/lib/systemd/system" "$PACKAGE_ROOT/usr/share/solarpowerhouse"
    if [ -d "$PACKAGE_ROOT/usr/local/bin" ]; then
        cp -a "$PACKAGE_ROOT/usr/local/bin/." "$PACKAGE_ROOT/usr/bin/"
        rm -rf "$PACKAGE_ROOT/usr/local/bin"
    fi
    if [ -d "$PACKAGE_ROOT/usr/local/lib/systemd/system" ]; then
        cp -a "$PACKAGE_ROOT/usr/local/lib/systemd/system/." "$PACKAGE_ROOT/usr/lib/systemd/system/"
        rm -rf "$PACKAGE_ROOT/usr/local/lib/systemd"
    fi
    sed -i 's#/usr/local/bin#/usr/bin#g' "$PACKAGE_ROOT/usr/lib/systemd/system/solar-core.service" "$PACKAGE_ROOT/usr/lib/systemd/system/solar-hmi.service"
    if [ -d "$PACKAGE_ROOT/usr/local/share/solarpowerhouse" ]; then
        cp -a "$PACKAGE_ROOT/usr/local/share/solarpowerhouse/." "$PACKAGE_ROOT/usr/share/solarpowerhouse/"
        rm -rf "$PACKAGE_ROOT/usr/local/share/solarpowerhouse"
    fi

    install -D -m 0644 \
        "$SOURCE_DIR/deploy/50-solarpowerhouse.rules" \
        "$PACKAGE_ROOT/etc/polkit-1/rules.d/50-solarpowerhouse.rules"
    printf '%s\n' /etc/polkit-1/rules.d/50-solarpowerhouse.rules \
        > "$PACKAGE_ROOT/DEBIAN/conffiles"

    write_maintainer_scripts

    cat > "$PACKAGE_ROOT/DEBIAN/control" <<CONTROL
Package: solarpowerhouse
Version: $version
Section: misc
Priority: optional
Architecture: $architecture
Maintainer: Solar Powerhouse maintainers
Depends: systemd, polkitd, libsqlite3-0, libqt6core6, libqt6gui6, libqt6network6, libqt6qml6, libqt6quick6, libqt6quickcontrols2-6, libqt6sql6, libqt6sql6-sqlite, qt6-qpa-plugins, qml6-module-qtqml, qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-layouts, qml6-module-qtquick-window
Description: Solar Powerhouse energy management system
 Solar Powerhouse provides the solar-core control service and the Qt Quick
 touch HMI, including Modbus TCP telemetry, SQLite history, and systemd
 service integration.
CONTROL

    install -d "$OUTPUT_DIR"
    dpkg-deb --build --root-owner-group "$PACKAGE_ROOT" "$package_path"
    printf 'Created %s\n' "$package_path"
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --source-dir)
            [ "$#" -ge 2 ] || die "--source-dir requires a path"
            SOURCE_DIR=$(CDPATH= cd -- "$2" && pwd)
            shift 2
            ;;
        --output-dir)
            [ "$#" -ge 2 ] || die "--output-dir requires a path"
            OUTPUT_DIR="$2"
            shift 2
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

require_debian_tools
build_repository
stage_package

