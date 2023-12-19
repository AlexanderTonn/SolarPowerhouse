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
  
  
  if (util.functionTrigger(mainCore.milRtuOld, 1000))
  {
    mainCore.mpptCharger.update(5000);
    mainCore.writeCloudVariables();
  }
  if (util.functionTrigger(mainCore.milTimeUpdateOld, mainCore.uiNtpTimeInterval))
    mainCore.ntp.updateTime(rtc::debugInformations::ACTIVE);
  
  ArduinoCloud.update();
  mainCore.controlLogic();
  mainCore.writeOutputs(); 
}




