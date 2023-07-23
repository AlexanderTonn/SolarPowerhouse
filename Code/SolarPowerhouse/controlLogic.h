#ifndef CONTROL_LOGIC_HPP
#define CONTROL_LOGIC_HPP

uint32_t uiBaudrate = 115200;

// Control Logic
float fSwitchCurrentTarget = 1.0;                                                                        // Threshold Current when is switching allowed
bool xFirstCycle = true;                                                                                 // for all function which should only execute once
float fBatteryVoltage = 0.0;                                                                             // measured voltage of battery
float fTrigBatteryFullPrevious = 0.0;                                                                    //used for edge detection
float fTrigBatteryEmptyPrevious = 0.0;                                                                   //used for edge detection
bool xTrigBatteryFull = false;                                                                           // Trigger if battery has reached a specific SoC
bool xTrigBatteryEmpty = false;                                                                          //Trigger if battery is dropped down to a specific voltage
bool xSwitchLaterOnMppt = false;                                                                         //Switch later if current certainly to high
bool xSwitchLaterOnInverter = false;                                                                     //Switch later if current certainly to high
float fBatterySoCVoltage[10] = { 25.7, 25.12, 26.42, 26.44, 26.46, 26.56, 26.66, 26.70, 26.74, 26.92 };  // State of Charges of LiFePo4 Battery in 10% steps (beginning from 10%)
uint64_t uiMyMillis = 0;


void switchRelays(bool xDateChanged, float fCurrent1, float fCurrent2);
void relayOnMppt();
void relayOnInverter();


#endif
