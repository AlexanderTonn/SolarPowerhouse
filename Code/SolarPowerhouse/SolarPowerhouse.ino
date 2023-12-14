#include "CORE_M7.h"

CORE_M7 mainCore; // create object of control class

void setup()
{
  delay(5000); // More delay for debugging
  mainCore.mpptCharger.init(9600);
  delay(2000);
  mainCore.init(); 
  // Defined in thingProperties.h
  initProperties();
  // Connect to Arduino IoT Cloud
  //ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  delay(2000);
  mainCore.ntp.init();
  

  Serial.println("Setup finished");
}

void loop()
{
  static  uint32_t milRtuOld = 0, milCloudOld = 0;
  if (util.functionTrigger(milRtuOld, 1000))
  {
    mainCore.mpptCharger.update(5000);
  }
  if (util.functionTrigger(milCloudOld, 5000))
  {
    mainCore.writeCloudVariables();
    ArduinoCloud.update();
  }
  mainCore.controlLogic();
  mainCore.writeOutputs(); 
   

}




