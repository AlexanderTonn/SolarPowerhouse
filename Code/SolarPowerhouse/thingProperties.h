// Code generated by Arduino IoT Cloud, DO NOT EDIT.

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

/*
const char IP[]      = SECRET_OPTIONAL_IP;
const char DNS[]     = SECRET_OPTIONAL_DNS;
const char GATEWAY[] = SECRET_OPTIONAL_GATEWAY;
const char NETMASK[] = SECRET_OPTIONAL_NETMASK;
*/

void onFSettSwitchCurrentTargetChange();
void onFSettTargetVoltageInverterChange();
void onFSettTargetVoltageMpptChange();
void onISettPVswitchingDelayChange();
void onXSettIgnoreCurrentChange();
void onXSettPVonInverterChange();
void onXSettPVonMpptChange();
void onXSettResetDayChange();

String sChargingState;
String sDebug0;
String sDebug1;
String sDebug10;
String sDebug2;
String sDebug3;
String sDebug4;
String sDebug5;
String sDebug6;
String sDebug7;
String sDebug8;
String sDebug9;
CloudElectricCurrent fBatteryCurrent;
CloudElectricCurrent fBatTodayCurrMax;
CloudElectricCurrent fSolarCurrent;
CloudElectricCurrent uiChargAhToday;
CloudElectricCurrent uiTotalAmpHoursCharged;
CloudElectricPotential fBatteryVoltage;
CloudElectricPotential fBatTodayVoltMax;
CloudElectricPotential fBatTodayVoltMin;
CloudElectricPotential fSolarVoltage;
float fBatSoC;
float fSettSwitchCurrentTarget;
float fSettTargetVoltageInverter;
float fSettTargetVoltageMppt;
CloudTemperatureSensor uiBatteryTemperature;
CloudTemperatureSensor uiMpptChargerTemperature;
int iSettPVswitchingDelay;
int uiTotalDaysOfOperation;
int uiTotalNoFullCharges;
int uiTotalNoOverDischarges;
CloudPower uiBatTodayPwrGeneration;
CloudPower uiBatTodayPwrMax;
CloudPower uiCumulativeKwhGeneration;
CloudPower uiCurrentSolarPower;
bool xMpptChargerLoadActive;
bool xRelayState1;
bool xRelayState2;
bool xRelayState3;
bool xRelayState4;
bool xSettIgnoreCurrent;
bool xSettPVonInverter;
bool xSettPVonMppt;
bool xSettResetDay;
bool xVoltageLow;

void initProperties(){

  ArduinoCloud.addProperty(sChargingState, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(sDebug0, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(sDebug1, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(sDebug10, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(sDebug2, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(sDebug3, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(sDebug4, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(sDebug5, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(sDebug6, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(sDebug7, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(sDebug8, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(sDebug9, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(fBatteryCurrent, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(fBatTodayCurrMax, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(fSolarCurrent, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(uiChargAhToday, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(uiTotalAmpHoursCharged, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(fBatteryVoltage, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(fBatTodayVoltMax, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(fBatTodayVoltMin, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(fSolarVoltage, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(fBatSoC, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(fSettSwitchCurrentTarget, READWRITE, ON_CHANGE, onFSettSwitchCurrentTargetChange);
  ArduinoCloud.addProperty(fSettTargetVoltageInverter, READWRITE, ON_CHANGE, onFSettTargetVoltageInverterChange);
  ArduinoCloud.addProperty(fSettTargetVoltageMppt, READWRITE, ON_CHANGE, onFSettTargetVoltageMpptChange);
  ArduinoCloud.addProperty(uiBatteryTemperature, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(uiMpptChargerTemperature, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(iSettPVswitchingDelay, READWRITE, ON_CHANGE, onISettPVswitchingDelayChange);
  ArduinoCloud.addProperty(uiTotalDaysOfOperation, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(uiTotalNoFullCharges, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(uiTotalNoOverDischarges, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(uiBatTodayPwrGeneration, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(uiBatTodayPwrMax, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(uiCumulativeKwhGeneration, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(uiCurrentSolarPower, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(xMpptChargerLoadActive, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(xRelayState1, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(xRelayState2, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(xRelayState3, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(xRelayState4, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(xSettIgnoreCurrent, READWRITE, ON_CHANGE, onXSettIgnoreCurrentChange);
  ArduinoCloud.addProperty(xSettPVonInverter, READWRITE, ON_CHANGE, onXSettPVonInverterChange);
  ArduinoCloud.addProperty(xSettPVonMppt, READWRITE, ON_CHANGE, onXSettPVonMpptChange);
  ArduinoCloud.addProperty(xSettResetDay, READWRITE, ON_CHANGE, onXSettResetDayChange);
  ArduinoCloud.addProperty(xVoltageLow, READ, ON_CHANGE, NULL);
}

// EthernetConnectionHandler ArduinoIoTPreferredConnection(IP, DNS, GATEWAY, NETMASK);