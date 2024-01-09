#include "CORE_M7.h"

// init Serial Communication for Modbus RTU and prepare shared memory
auto CORE_M7::init() -> void
{
  pinMode(K1_INVERTER_DC_SIDE, OUTPUT);
  pinMode(K2_MPPT_CHARGER, OUTPUT);
  pinMode(K3_INVERTER_AC_SIDE, OUTPUT);
}

// Set Relays on Mppt Mode
void CORE_M7::pv1OnMppt(debugMode dLevel)
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

  if (util.functionTrigger(uiMillisElapsed.at(0), settings.uiPVswitchingDelay * 1000))
  {
    digitalWrite(K3_LED, LOW);
    digitalWrite(K3_INVERTER_AC_SIDE, LOW);
    aRelayStates[2] = false;
  }

  if (util.functionTrigger(uiMillisElapsed.at(1), settings.uiPVswitchingDelay * 1000 * 2))
  {
    digitalWrite(K1_LED, LOW);
    digitalWrite(K1_INVERTER_DC_SIDE, LOW);
    aRelayStates[0] = false;
  }

  if (util.functionTrigger(uiMillisElapsed.at(2), settings.uiPVswitchingDelay * 1000 * 3))
  {
    digitalWrite(K2_LED, HIGH);
    digitalWrite(K2_MPPT_CHARGER, HIGH);
    aRelayStates[1] = true;
    byModeOld = byModeActual;
    xPv1OnMppt = false;
    xGetMillis = true;
  }
}
// Set Relays on Inverter Mode
void CORE_M7::pv1OnInverter(debugMode dLevel)
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

  if (util.functionTrigger(uiMillisElapsed.at(0), settings.uiPVswitchingDelay * 1000))
  {
     digitalWrite(K2_LED, LOW);
     digitalWrite(K2_MPPT_CHARGER, LOW);
     aRelayStates[1] = false;
  }
  if (util.functionTrigger(uiMillisElapsed.at(1), settings.uiPVswitchingDelay * 1000 * 2))
  {
    digitalWrite(K1_LED, HIGH);
    digitalWrite(K1_INVERTER_DC_SIDE, HIGH);
    aRelayStates[0] = true;
  }
  if (util.functionTrigger(uiMillisElapsed.at(2), settings.uiPVswitchingDelay * 1000 * 3))
  {
    digitalWrite(K3_LED, HIGH);
    digitalWrite(K3_INVERTER_AC_SIDE, HIGH);
    aRelayStates[2] = true;
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
  bool xTrigBatteryFull = util.edgeDetection(mpptCharger.controllerInformation.fBatVolt,
                                        fTrigBatteryFullPrevious,
                                        atUtilities::edgeType::RISING_EDGE,
                                        settings.fTargetVoltageInverter); // Battery is full after reaching 100%
  bool xTrigBatteryEmpty = util.edgeDetection(mpptCharger.controllerInformation.fBatVolt,
                                         fTrigBatteryEmptyPrevious,
                                         atUtilities::edgeType::FALLING_EDGE,
                                         settings.fTargetVoltageMppt); // Battery should charged if dropped down to 50%

  auto xCheckDayChange = util.functionTrigger(uiMillisCheckDayChange, 3600000); // 1 hour

  // CONTROL LOGIC
  // Set Relays on Mppt if day changed
  if (xCheckDayChange || xFirstCycle)
  {
    Serial.println("xCheckDayChange: " + String(xCheckDayChange));
    auto xDayChanged = ntp.rtcCheckDayChange(settings.xResetDay);
    if (xDayChanged == true && mpptCharger.controllerInformation.fSolarCurr <= settings.fSwitchCurrentTarget || xFirstCycle)
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
      if (xTrigBatteryFull || xSwitchLaterOnInverter)
      {
        if (mpptCharger.controllerInformation.fSolarCurr <= settings.fSwitchCurrentTarget)
        {
          byModeActual = INVERTER_MODE;
          xPv1OnInverter = true;
          xPv1OnMppt = false;
          xSwitchLaterOnInverter = false;
        }
        else
          xSwitchLaterOnInverter = true;
      }
      // Switching from Invertwe to mppt charger
      // if current is certainly to high then try to switch later if the current is low
      else if (xTrigBatteryEmpty == true || xSwitchLaterOnMppt == true)
      {
        if (mpptCharger.controllerInformation.fSolarCurr <= settings.fSwitchCurrentTarget)
        {
          byModeActual = BATTERY_MODE;
          xPv1OnMppt = true;
          xPv1OnInverter = false;
          xSwitchLaterOnMppt = false;
        }
        else
          xSwitchLaterOnMppt = true;
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
/**
 * @brief
 * write Modbus RTU data to clodu variables
 * @param none
 * @return none
 */
auto CORE_M7::writeCloudVariables() -> void
{
  fBatteryCurrent = mpptCharger.controllerInformation.fBatCurr;
  fBatteryVoltage = mpptCharger.controllerInformation.fBatVolt;
  fSolarCurrent = mpptCharger.controllerInformation.fSolarCurr;
  fSolarVoltage = mpptCharger.controllerInformation.fSolarVolt;
  fBatTodayVoltMin = mpptCharger.controllerInformation.fBatMinVoltToday;
  fBatTodayVoltMax = mpptCharger.controllerInformation.fBatMaxVoltToday;
  fBatTodayCurrMax = mpptCharger.controllerInformation.fMaxChargCurrToday;
  uiCurrentSolarPower = mpptCharger.controllerInformation.uiSolarPwr;
  uiBatTodayPwrMax = mpptCharger.controllerInformation.uiMaxChargPwrToday;
  uiBatTodayPwrGeneration = mpptCharger.controllerInformation.uiPwrGenerationToday;
  uiBatteryTemperature = mpptCharger.controllerInformation.uiBatSurTemp;
  uiMpptChargerTemperature = mpptCharger.controllerInformation.uiDevTemp;
  xMpptChargerLoadActive = mpptCharger.controllerInformation.xLoadOutputActive;
  uiTotalAmpHoursCharged = mpptCharger.controllerInformation.uiTotalAmpHoursCharged;
  uiTotalDaysOfOperation = mpptCharger.controllerInformation.uiTotalDaysOfOperation;
  uiTotalNoOverDischarges = mpptCharger.controllerInformation.uiTotalNoOverDischarges;
  uiChargAhToday = mpptCharger.controllerInformation.uiChargAhToday;
  uiCumulativeKwhGeneration = mpptCharger.controllerInformation.uiCumulativeKwhGeneration;
  fBatSoC = calculateSoC();
  xRelayState1 = aRelayStates[0];
  xRelayState2 = aRelayStates[1];
  xRelayState3 = aRelayStates[2];
  xRelayState4 = aRelayStates[3];
  xSettPVonInverter = settings.xPVonInverter;
  xSettPVonMppt = settings.xPVonMppt;
  xSettResetDay = settings.xResetDay;
  iSettPVswitchingDelay = settings.uiPVswitchingDelay;
  xSettIgnoreCurrent = settings.xignoreCurrent;
  fSettTargetVoltageInverter = settings.fTargetVoltageInverter;
  fSettTargetVoltageMppt = settings.fTargetVoltageMppt;
  fSettSwitchCurrentTarget = settings.fSwitchCurrentTarget;
  uiTotalNoFullCharges = mpptCharger.controllerInformation.uiTotalNoFullCharges;
  xMpptChargerLoadActive = mpptCharger.controllerInformation.xLoadOutputActive;
  sChargingState = mpptCharger.controllerInformation.sChargingState;
  xSettIgnoreCurrent = settings.xignoreCurrent; 
  xVoltageLow = warnLowBatVoltage();

}
/**
 * @brief Calculate State of Charge depending of the voltage
 * 
 * @return float 
 */
auto CORE_M7::calculateSoC() -> float
{
  return util.fLinInterpolation(mpptCharger.controllerInformation.fBatVolt, aSocVoltage24, aSoCPercentage);
}
/**
 * @brief Check if battery voltage is lower than target voltage for warning
 * 
 * @return true 
 * @return false 
 */
auto  CORE_M7::warnLowBatVoltage() -> bool
{
  if (mpptCharger.controllerInformation.fBatVolt <= settings.fTargetWarningVoltage)
    return true;
  else
    return false;
}
