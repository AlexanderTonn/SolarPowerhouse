#include "CORE_M7.h"

// init Serial Communication for Modbus RTU and prepare shared memory
auto CORE_M7::init() -> void
{
  pinMode(K1_INVERTER_DC_SIDE, OUTPUT);
  pinMode(K2_MPPT_CHARGER, OUTPUT);
  pinMode(K3_INVERTER_AC_SIDE, OUTPUT);

  mpptCharger.init(9600);
  ipc.memoryInit();
}

// Read Inputs from MPPT Charger through Modbus RTU
auto CORE_M7::readInputs() -> void
{
  static uint32_t uiMillisElapsed = millis();

  // get each second data from mppt charger
  if (functionTrigger(uiMillisElapsed, 1000))
  {
    process.fBatteryVoltage = mpptCharger.getBatVolt(uiToyoDeviceId);
    process.fSolarCurrent = mpptCharger.getSolarCurr(uiToyoDeviceId);
  }
}

// Set Relays on Mppt Mode
void CORE_M7::pv1OnMppt(debugLevel dLevel)
{
  static std::array<uint32_t, 3> uiMillisElapsed = {0};
  static bool xGetMillis = true;
  if (xGetMillis)
  {
    uiMillisElapsed.at(0) = millis();
    uiMillisElapsed.at(1) = millis();
    uiMillisElapsed.at(2) = millis();
    xGetMillis = false;
  }

  if (functionTrigger(uiMillisElapsed.at(0), settings.uiPVswitchingDelay * 1000))
  {
    digitalWrite(LED_D2, LOW);
    digitalWrite(K3_INVERTER_AC_SIDE, LOW);
  }

  if (functionTrigger(uiMillisElapsed.at(1), settings.uiPVswitchingDelay * 1000 * 2))
  {
    digitalWrite(LED_D0, LOW);
    digitalWrite(K1_INVERTER_DC_SIDE, LOW);
  }

  if (functionTrigger(uiMillisElapsed.at(2), settings.uiPVswitchingDelay * 1000 * 3))
  {
    digitalWrite(LED_D1, HIGH);
    digitalWrite(K3_INVERTER_AC_SIDE, HIGH);
    byModeOld = byModeActual;
    xPv1OnMppt = false;
    xGetMillis = true;
  }
}
// Set Relays on Inverter Mode
void CORE_M7::pv1OnInverter(debugLevel dLevel)
{
  static std::array<uint32_t, 3> uiMillisElapsed = {0};
  static bool xGetMillis = true;
  if (xGetMillis)
  {
    uiMillisElapsed.at(0) = millis();
    uiMillisElapsed.at(1) = millis();
    uiMillisElapsed.at(2) = millis();
    xGetMillis = false;
  }

  if (functionTrigger(uiMillisElapsed.at(0), settings.uiPVswitchingDelay * 1000))
    digitalWrite(K2_MPPT_CHARGER, LOW);

  if (functionTrigger(uiMillisElapsed.at(1), settings.uiPVswitchingDelay * 1000 * 2))
    digitalWrite(K1_INVERTER_DC_SIDE, HIGH);

  if (functionTrigger(uiMillisElapsed.at(2), settings.uiPVswitchingDelay * 1000 * 3))
  {
    digitalWrite(K3_INVERTER_AC_SIDE, HIGH);
    byModeOld = byModeActual;
    xPv1OnInverter = false;
    xGetMillis = true;
  }
}

// Option for setting relays in manual state
auto CORE_M7::SettingsPvSwitching(
    bool xInverterManual,
    bool xMpptManual) -> operationMode
{
  if (xInverterManual)
  {
    return INVERTER_MODE;
  }
  else if (xMpptManual)
  {
    return BATTERY_MODE;
  }
  else
  {
    return NORMAL;
  }
}

// ##############
// Main Function for controling the relays depending of voltage and current in the system
// ##############
auto CORE_M7::controlLogic() -> void
{
  bool xTrigBatteryFull = edgeDetection(process.fBatteryVoltage,
                                        process.fTrigBatteryFullPrevious,
                                        edgeType::RISING_EDGE,
                                        settings.fTargetVoltageInverter); // Battery is full after reaching 100%
  bool xTrigBatteryEmpty = edgeDetection(process.fBatteryVoltage,
                                         process.fTrigBatteryEmptyPrevious,
                                         edgeType::FALLING_EDGE,
                                         settings.fTargetVoltageMppt); // Battery should charged if dropped down to 50%

  auto xCheckDayChange = functionTrigger(uiMillisCheckDayChange, 3600000); // 1 hour

  // CONTROL LOGIC
  // Set Relays on Mppt if day changed

  if (xCheckDayChange || xFirstCycle)
  {
    Serial.println("xCheckDayChange: " + String(xCheckDayChange));
    // Set Relays on Mppt if day changed
    auto xDayChanged = ntp.rtcCheckDayChange(settings.xResetDay);
    if (xDayChanged == true && process.fSolarCurrent <= settings.fSwitchCurrentTarget || xFirstCycle)
    {
      byModeActual = BATTERY_MODE;
      xPv1OnMppt = true;
      xFirstCycle = false;
    }
  }

  // check operation mode, whehter manual on inverter or mppt or automatic
  auto auOperationMode = SettingsPvSwitching(settings.xPVonInverter, settings.xPVonMppt);

  switch (auOperationMode)
  {
  case NORMAL:

    if (auOperationMode == NORMAL)
    {
      // Switching from mppt to inverter
      if (xTrigBatteryFull || process.xSwitchLaterOnInverter)
      {
        if (process.fSolarCurrent <= settings.fSwitchCurrentTarget)
        {
          byModeActual = INVERTER_MODE;
          xPv1OnInverter = true;
          xPv1OnMppt = false;
          process.xSwitchLaterOnInverter = false;
        }
        else
          process.xSwitchLaterOnInverter = true;
      }
      // Switching from Invertwe to mppt charger
      // if current is certainly to high then try to switch later if the current is low
      else if (xTrigBatteryEmpty == true || process.xSwitchLaterOnMppt == true)
      {
        if (process.fSolarCurrent <= settings.fSwitchCurrentTarget)
        {
          byModeActual = BATTERY_MODE;
          xPv1OnMppt = true;
          xPv1OnInverter = false;
          process.xSwitchLaterOnMppt = false;
        }
        else
          process.xSwitchLaterOnMppt = true;
      }
    }
    break;
  case BATTERY_MODE:
    byModeActual = BATTERY_MODE;
    xPv1OnMppt = true;
    xPv1OnInverter = false;
    break;

  case INVERTER_MODE:
    byModeActual = INVERTER_MODE;
    xPv1OnInverter = true;
    xPv1OnMppt = false;
    break;

  default:
    Serial.println("Error in controlLogic");
    break;
  }
}

// set Outputs of Arduino Opta
auto CORE_M7::writeOutputs() -> void
{
  // Set Relays
  if (xPv1OnInverter)
    pv1OnInverter();
  if (xPv1OnMppt)
    pv1OnMppt();
}
// Read Inputs from MPPT Charger through Modbus RTU
auto CORE_M7::readModbusRtu() -> void
{
  static uint32_t uiMillisElapsed = millis();

  // get each second data from mppt charger
  if (functionTrigger(uiMillisElapsed, 1000))
  {
    process.fBatteryVoltage = mpptCharger.getBatVolt(uiToyoDeviceId);
    process.fSolarCurrent = mpptCharger.getSolarCurr(uiToyoDeviceId);
    informative.fSolarVoltage = mpptCharger.getSolarVolt(uiToyoDeviceId);
    informative.fBatTodayVoltMin = mpptCharger.getBatMinVoltToday(uiToyoDeviceId);
    informative.fBatTodayVoltMax = mpptCharger.getBatMaxVoltToday(uiToyoDeviceId);
    informative.fBatTodayCurrMax = mpptCharger.getMaxChargCurrToday(uiToyoDeviceId);
    informative.uiCurrentSolarPower = mpptCharger.getSolarPwr(uiToyoDeviceId);
    informative.uiBatTodayPwrMax = mpptCharger.getMaxChargPwrToday(uiToyoDeviceId);
    informative.uiBatTodayPwrGeneration = mpptCharger.getPwrGenerationToday(uiToyoDeviceId);
    informative.uiBatteryTemperature = mpptCharger.getBatSurTemp(uiToyoDeviceId);
    informative.uiMpptChargerTemperature = mpptCharger.getDevTemp(uiToyoDeviceId);
  }
}
// Write variables to shared memory for CORE M4
auto CORE_M7::writeSharedMemory() -> void
{
  static uint32_t uiMillisElapsed = millis();

  if (functionTrigger(uiMillisElapsed, 1000))
  {
    ipc.writeSharedFloat(process.fBatteryVoltage, sharedMemory::floatId::BATTERY_VOLTAGE);
    ipc.writeSharedFloat(process.fSolarCurrent, sharedMemory::floatId::SOLAR_CURRENT);
    ipc.writeSharedFloat(informative.fSolarVoltage, sharedMemory::floatId::SOLAR_VOLTAGE);
    ipc.writeSharedFloat(informative.fBatTodayVoltMin, sharedMemory::floatId::BAT_TODAY_VOLT_MIN);
    ipc.writeSharedFloat(informative.fBatTodayVoltMax, sharedMemory::floatId::BAT_TODAY_VOLT_MAX);
    ipc.writeSharedFloat(informative.fBatTodayCurrMax, sharedMemory::floatId::BAT_TODAY_CURR_MAX);
    ipc.writeSharedUnsignedInt(informative.uiCurrentSolarPower, sharedMemory::uiId::SOLAR_POWER);
    ipc.writeSharedUnsignedInt(informative.uiBatTodayPwrMax, sharedMemory::uiId::BAT_TODAY_PWR_MAX);
    ipc.writeSharedUnsignedInt(informative.uiBatTodayPwrGeneration, sharedMemory::uiId::BAT_TODAY_PWR_GENERATION);
  }
}
