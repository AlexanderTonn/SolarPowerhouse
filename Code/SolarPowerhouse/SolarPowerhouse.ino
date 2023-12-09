// NTPClient - Version: Latest 




// Core M7 is running following tasks:
// - controlLogic
// - Modbus RTU Communication to MPPT Charger

#if defined(CORE_CM7)

#include "CORE_M7.h"

CORE_M7 mainCore; // create object of control class

void setup()
{
  mainCore.init(); 

  delay(1000); // 100ms was not enough for init


  // TODO! Change it for using with Arduin Opta 
  // rtcInit(); // 

}

void loop()
{

  mainCore.readInputs();
  mainCore.readModbusRtu();
  mainCore.controlLogic();
  mainCore.writeSharedMemory();
  // TODO!: First tests without relay switching
  // control.writeOutputs();

}

// CORE M4 is running following tasks:
// - Arduin IoT Cloud
// - Future: Modbus RTU to HMI 
#elif defined(CORE_CM4)

#include "CORE_M4.h"

CORE_M4 secondCore; // create object of cloud class


void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500); 

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  
  delay(1000);
  
  secondCore.init();
}

void loop() {
  ArduinoCloud.update();
  secondCore.getVarsFromSharedMemory();
  // Your code here 
  
  
}

#endif


