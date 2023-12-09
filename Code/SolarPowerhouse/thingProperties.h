// Code generated by Arduino IoT Cloud, DO NOT EDIT.

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

/*
const char IP[]      = SECRET_OPTIONAL_IP;
const char DNS[]     = SECRET_OPTIONAL_DNS;
const char GATEWAY[] = SECRET_OPTIONAL_GATEWAY;
const char NETMASK[] = SECRET_OPTIONAL_NETMASK;
*/

CloudElectricCurrent fBatteryCurrent;
CloudElectricCurrent fBatTodayCurrMax;
CloudElectricCurrent fSolarCurrent;
CloudElectricPotential fBatteryVoltage;
CloudElectricPotential fBatTodayVoltMax;
CloudElectricPotential fBatTodayVoltMin;
CloudElectricPotential fSolarVoltage;
CloudTemperatureSensor uiBatteryTemperature;
CloudTemperatureSensor uiMpptChargerTemperature;
CloudPower uiBatTodayPwrGeneration;
CloudPower uiBatTodayPwrMax;
CloudPower uiCurrentSolarPower;
bool xMpptChargerLoadActive;

void initProperties(){

  ArduinoCloud.addProperty(fBatteryCurrent, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(fBatTodayCurrMax, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(fSolarCurrent, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(fBatteryVoltage, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(fBatTodayVoltMax, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(fBatTodayVoltMin, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(fSolarVoltage, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(uiBatteryTemperature, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(uiMpptChargerTemperature, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(uiBatTodayPwrGeneration, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(uiBatTodayPwrMax, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(uiCurrentSolarPower, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(xMpptChargerLoadActive, READ, ON_CHANGE, NULL);

}

// EthernetConnectionHandler ArduinoIoTPreferredConnection(IP, DNS, GATEWAY, NETMASK);