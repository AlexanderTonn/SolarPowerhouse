#include "CORE_M7.h"


void onXSettPVonInverterChange()  {
  mainCore.settings.xPVonInverter = xSettPVonInverter;
}

void onXSettPVonMpptChange()  {
  mainCore.settings.xPVonMppt = xSettPVonMppt;
}

void onXSettResetDayChange()  {
  mainCore.settings.xResetDay = xSettResetDay;
}

void onISettPVswitchingDelayChange()  {
  mainCore.settings.uiPVswitchingDelay = static_cast<uint16_t>(iSettPVswitchingDelay);
}

void onXSettIgnoreCurrentChange()  {
  mainCore.settings.xignoreCurrent = xSettIgnoreCurrent;
}

void onFSettTargetVoltageInverterChange()  {
    mainCore.settings.fTargetVoltageInverter = fSettTargetVoltageInverter;
}

void onFSettTargetVoltageMpptChange()  {
  mainCore.settings.fTargetVoltageMppt = fSettTargetVoltageMppt;
}

void onFSettSwitchCurrentTargetChange()  {
  mainCore.settings.fSwitchCurrentTarget = fSettSwitchCurrentTarget;
}

void onFTariffChange()  {
  mainCore.settings.fTariff = fTariff;
}

void onSOptaLogChange()  {
  // Add your code here to act upon SOptaLog change
}