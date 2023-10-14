#include "Util.h"

// ############
// MY UTILITIES
// ############
// Collection of custom functions
auto myMillis(uint64_t& uiMyMillis) -> void {
  uint32_t uiMillis = 0;
  uint64_t uiMillisMax = 4294967295; // Max Value of millis
  uint64_t uiNewStartpoint = 0;     // This value will work as new start point for myMillis()
  uint16_t uiMultiplier = 0;


  uiMultiplier    = uiMyMillis / uiMillisMax;
  uiNewStartpoint = uiMillisMax * uiMultiplier; //calculating the multiplier which will calculate automatically

  uiMillis = millis();

  // if millis() < MyMillis, then add the millis onto my millis
  // max millis which could be saved with uint64t: 18446744073709551616 == 213503982334 days

  if (uiMillis < 4320000000 && uiMyMillis >= uiNewStartpoint) {
     uiMyMillis = uiNewStartpoint + uiMillis;
  }
  // my Millis counted up to 100 days
  else {
    uiMyMillis = uiMillis;
  }
}


template<typename T1, typename T2, typename T3 >
auto edgeDetection (T1 TinputSignal, T2 &TpreviousSignal, edgeType Type, T3 Ttrigger ) -> bool {
  // RISING EDGE detection
  switch(Type)
  {
    case edgeType::RISING_EDGE :
    if (TinputSignal >= Ttrigger && TpreviousSignal < Ttrigger ) {
      TpreviousSignal = TinputSignal;
      return true;
    }
    // Resetting the signal if Ttrigger was reached
    else if ((TinputSignal >= Ttrigger && TpreviousSignal >= Ttrigger) 
          || (TinputSignal < Ttrigger && TpreviousSignal >= Ttrigger)) {
      TpreviousSignal = TinputSignal;
      return false;
    }else 
      return false; 
    break;

    case edgeType::FALLING_EDGE :
    if (TinputSignal <= Ttrigger && TpreviousSignal > Ttrigger) {
      TpreviousSignal = TinputSignal;
      return true;
    }
    else if ((TinputSignal <= Ttrigger && TpreviousSignal <= Ttrigger) || (TinputSignal > Ttrigger && TpreviousSignal <= Ttrigger)) {
      TpreviousSignal = TinputSignal;
      return false;
    }
    else 
      return false;
    break;

    default:
    Serial.println("Error: No edge type defined");
    break;

  }

}

// Max Targettime: about 49 days 
auto functionTrigger(uint32_t &uiMillisOld, uint32_t uiTargetTime) -> bool {

  if (millis() - uiMillisOld >= uiTargetTime) {
    uiMillisOld += uiTargetTime;
    return true;
  } else {
    return false;
  }

}

auto secondToMs(uint16_t uiS) -> uint16_t{
  return uiS*1000;
}

auto fmap(float value, float in_min, float in_max, float out_min, float out_max) -> float
{
  float fScaled = 0.0;
  fScaled = out_min+(value-out_min)*((out_max-out_min)/(in_max-in_min));
  return fScaled;

}