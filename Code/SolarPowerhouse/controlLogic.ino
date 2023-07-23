#include "pinDefinitions.h"
#include "HMI.h"
#include "controlLogic.h"

// ##############
// Switch Relay Function
// ##############
// - if date changed set the relayys onto mppt charger and turn of Inverter

void switchRelays(bool xDateChanged, float fCurrent1, float fCurrent2) {
  if (xDateChanged == true) {
    relayOnMppt();
  }
}

// Set Relays on Mppt Mode
void relayOnMppt() {
  digitalWrite(K2_INVERTER_AC, LOW);
  delay(2000);
  digitalWrite(K1_INVERTER_DC, LOW);
  delay(2000);
  digitalWrite(K3_MPPT_CHARGER, HIGH);
  byModeActual = BATTERY_MODE;
}
void relayOnInverter() {
  digitalWrite(K3_MPPT_CHARGER, LOW);
  delay(2000);
  digitalWrite(K1_INVERTER_DC, HIGH);
  delay(2000);
  digitalWrite(K2_INVERTER_AC, HIGH);
  byModeActual = INVERTER_MODE;
}