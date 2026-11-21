cmake_minimum_required(VERSION 3.20)

project(SolarPowerhouse VERSION 0.1.0 LANGUAGES CXX)

include(CTest)
find_package(SQLite3 REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(solar_core
    src/application/ApplicationController.cpp
    src/domain/BatteryMonitor.cpp
    src/domain/EnergyManager.cpp
    src/domain/EventManager.cpp
    src/domain/InterlockManager.cpp
    src/domain/IslandInverterProtectionController.cpp
    src/domain/SwitchingController.cpp
    src/infrastructure/OptaModbusTcp.cpp
    src/infrastructure/TelemetryHistoryStore.cpp
    src/infrastructure/TelemetryIpc.cpp
)

target_include_directories(solar_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE
        ${SQLite3_INCLUDE_DIRS}
)

target_compile_features(solar_core PUBLIC cxx_std_20)
target_link_libraries(solar_core PRIVATE ${SQLite3_LIBRARIES})

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(solar_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

add_executable(solar-core apps/solar-core/main.cpp)
target_link_libraries(solar-core PRIVATE solar_core)
target_compile_definitions(solar-core PRIVATE SOLAR_POWERHOUSE_VERSION="${PROJECT_VERSION}")

if (BUILD_TESTING)
    add_executable(solar-core-tests
        tests/main.cpp
        tests/BatteryMonitorTest.cpp
        tests/EventManagerTest.cpp
        tests/IslandInverterProtectionTest.cpp
        tests/SwitchingControllerTest.cpp
        tests/MqttContractTest.cpp
        tests/MockHardwareInterface.cpp
        tests/OptaModbusTcpTest.cpp
        tests/TelemetryHistoryStoreTest.cpp
    )
    target_link_libraries(solar-core-tests PRIVATE solar_core)
    target_include_directories(solar-core-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    add_test(NAME solar-core-tests COMMAND solar-core-tests)
endif()

find_package(Qt6 QUIET COMPONENTS Core Gui Qml Quick QuickControls2 Network Sql)
if (Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)
    if (COMMAND qt_policy)
        qt_policy(SET QTP0001 NEW)
        qt_policy(SET QTP0004 NEW)
    endif()

    qt_add_executable(solar-hmi
        apps/solar-hmi/main.cpp
        src/hmi/BackendSupervisor.hpp
        src/hmi/BackendSupervisor.cpp
        src/hmi/LanguageManager.hpp
        src/hmi/LanguageManager.cpp
        src/hmi/ModbusTelemetry.hpp
        src/hmi/ModbusTelemetry.cpp
        src/hmi/UserManager.hpp
        src/hmi/UserManager.cpp
        src/hmi/WeatherService.hpp
        src/hmi/WeatherService.cpp
    )
    target_include_directories(solar-hmi PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_compile_definitions(solar-hmi PRIVATE SOLAR_POWERHOUSE_VERSION="${PROJECT_VERSION}")
    set_target_properties(solar-hmi PROPERTIES AUTOMOC_PATH_PREFIX ON)
    qt_add_qml_module(solar-hmi
        URI SolarPowerhouse
        VERSION 1.0
        QML_FILES
            hmi/Main.qml
            hmi/Localization.qml
            hmi/components/AppHeader.qml
            hmi/components/ActiveAlarmBanner.qml
            hmi/components/LanguageSelector.qml
            hmi/components/MetricCard.qml
            hmi/components/NavigationDrawer.qml
            hmi/components/TouchInputField.qml
            hmi/components/TouchKey.qml
            hmi/components/TouchNumericPad.qml
            hmi/components/OperatingModeSelector.qml
            hmi/components/EventRow.qml
            hmi/components/SolarTile.qml
            hmi/components/StatusIndicator.qml
            hmi/components/SettingRow.qml
            hmi/components/ValueDisplay.qml
            hmi/components/CurveToggle.qml
            hmi/components/TelemetryChart.qml
            hmi/components/TelemetryValueRow.qml
            hmi/pages/Dashboard.qml
            hmi/pages/IslandPage.qml
            hmi/pages/GridPage.qml
            hmi/pages/SettingsPage.qml
            hmi/pages/SetupPage.qml
            hmi/pages/MessageListPage.qml
        RESOURCES
            hmi/assets/flag-de.svg
            hmi/assets/flag-gb.svg
    )
    target_link_libraries(solar-hmi PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::Network Qt6::Sql)
else()
    message(STATUS "Qt6 not found: solar-hmi target disabled; solar-core and tests remain buildable")
endif()

install(TARGETS solar-core RUNTIME DESTINATION bin)
if (TARGET solar-hmi)
    install(TARGETS solar-hmi RUNTIME DESTINATION bin)
endif()
install(DIRECTORY config DESTINATION share/solarpowerhouse)
install(DIRECTORY plc DESTINATION share/solarpowerhouse)
install(FILES deploy/solar-core.service deploy/solar-hmi.service deploy/solar-powerhouse.target
        DESTINATION lib/systemd/system)
install(FILES deploy/50-solarpowerhouse.rules
        DESTINATION share/solarpowerhouse/polkit-rules)
