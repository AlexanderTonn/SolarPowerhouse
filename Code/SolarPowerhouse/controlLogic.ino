#include "controlLogic.h"

// ##############
// Switch Relay Function
// ##############
// - if date changed set the relayys onto mppt charger and turn of Inverter

// Set Relays on Mppt Mode
void pv1OnMppt()
{
    static std::array<uint32_t, 3> uiMillisElapsed = {0};
    static bool xGetMillis = true;
    if(xGetMillis)
    {
      uiMillisElapsed.at(0) = millis();
      uiMillisElapsed.at(1) = millis();
      uiMillisElapsed.at(2) = millis();
      xGetMillis = false;
    }

    if(functionTrigger(uiMillisElapsed.at(0), uiSett_PVswitchingDelay*1000))
    {
      digitalWrite(K2_INVERTER_AC, LOW);
    }


    if(functionTrigger(uiMillisElapsed.at(1), uiSett_PVswitchingDelay*1000*2))
    {
      digitalWrite(K1_INVERTER_DC, LOW);
    }

    if(functionTrigger(uiMillisElapsed.at(2), uiSett_PVswitchingDelay*1000*3))
    {
      digitalWrite(K3_MPPT_CHARGER, HIGH);
      byModeOld = byModeActual;
      xPv1OnMppt = false;
      xGetMillis = true;
    }
}
void pv1OnInverter()
{
    static std::array<uint32_t, 3> uiMillisElapsed = {0};
    static bool xGetMillis = true;
    if(xGetMillis)
    {
      uiMillisElapsed.at(0) = millis();
      uiMillisElapsed.at(1) = millis();
      uiMillisElapsed.at(2) = millis();
      xGetMillis = false;
    }
    
    if(functionTrigger(uiMillisElapsed.at(0), uiSett_PVswitchingDelay*1000))
      digitalWrite(K3_MPPT_CHARGER, LOW);
    
    if(functionTrigger(uiMillisElapsed.at(1), uiSett_PVswitchingDelay*1000*2))
      digitalWrite(K1_INVERTER_DC, HIGH);
    
    if(functionTrigger(uiMillisElapsed.at(2), uiSett_PVswitchingDelay*1000*3))
    {
      digitalWrite(K2_INVERTER_AC, HIGH);
      byModeOld = byModeActual;
      xPv1OnInverter = false;
      xGetMillis = true;
    }
}

// Switching from mppt to inverter
// consider the option of page 2
auto SettingsPvSwitching(bool xInverterManual,
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

auto controlLogic() -> void
{
  bool xTrigBatteryFull = edgeDetection(fBatteryVoltage, 
                                  fTrigBatteryFullPrevious, 
                                  edgeType::RISING_EDGE, 
                                  fSett_TargetVoltageInverter);    // Battery is full after reaching 100%
  bool xTrigBatteryEmpty = edgeDetection(fBatteryVoltage, 
                                  fTrigBatteryEmptyPrevious, 
                                  edgeType::FALLING_EDGE, 
                                  fSett_TargetVoltageMppt); // Battery should charged if dropped down to 50%

  auto xCheckDayChange = functionTrigger(uiMillisCheckDayChange, 3600000); // 1 hour

  // CONTROL LOGIC
  // Set Relays on Mppt if day changed

  if (xCheckDayChange || xFirstCycle)
  {
    Serial.println("xCheckDayChange: " + String(xCheckDayChange));
    // Set Relays on Mppt if day changed
    auto xDayChanged = rtcCheckDayChange(xSett_ResetDay);
    if (xDayChanged == true && fCurrMppt <= fSett_SwitchCurrentTarget && fCurrInverter <= fSett_SwitchCurrentTarget || xFirstCycle)
    {
      byModeActual = BATTERY_MODE;
      xPv1OnMppt = true;
      xFirstCycle = false;
    }
  }

  // check operation mode, whehter manual on inverter or mppt or automatic
  auto auOperationMode = SettingsPvSwitching(xSett_PVonInverter, xSett_PVonMppt);

  switch (auOperationMode)
  {
  case NORMAL:
   
  if (auOperationMode == NORMAL)
  {
    // Switching from mppt to inverter
    if (xTrigBatteryFull || xSwitchLaterOnInverter)
    {
      if (fCurrMppt <= fSett_SwitchCurrentTarget)
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
      if (fCurrInverter <= fSett_SwitchCurrentTarget)
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
