/**
 * @file Util.ino
 * @author https://github.com/AlexanderTonn
 * @brief Collection of self developed functions for simplifying the code
 * @version 0.1
 * @date 2023-12-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#include "Util.h"

/**
 * @brief enhanced millis() function for counting up to 584942417 years
 * ! could be necessary if you log events with millis and you dont want a reset after 49 days
 * ! This function is RAM intense! This function is for special use cases and is not replacing the original millis() function
 * @param uiMyMillis uint64_t for saving the millis, must be a static or global variable
 */
auto atUtilities::myMillis(uint64_t& uiMyMillis) -> void {
  uint32_t uiMillis = 0;
  uint64_t uiMillisMax = 4294967295; // Max Value of millis
  uint64_t uiNewStartpoint = 0;     // This value will work as new start point for myMillis()
  uint16_t uiMultiplier = 0;


  uiMultiplier    = uiMyMillis / uiMillisMax;
  uiNewStartpoint = uiMillisMax * uiMultiplier; //calculating the multiplier which will calculate automatically

  uiMillis = millis();

  // if millis() < MyMillis, then add the millis onto my millis
  // max millis which could be saved with uint64t: 18446744073709551616 == 213503982334 days == 584942417 years
  if (uiMillis < 4320000000 && uiMyMillis >= uiNewStartpoint) {
     uiMyMillis = uiNewStartpoint + uiMillis;
  }
  else {
    uiMyMillis = uiMillis;
  }
}

/**
 * @brief 
 * 
 * @tparam T1 
 * @tparam T2 
 * @tparam T3 
 * @param TinputSignal 
 * @param TpreviousSignal 
 * @param Type 
 * @param Ttrigger 
 * @return true 
 * @return false 
 */
template<typename T1, typename T2, typename T3 >
auto atUtilities::edgeDetection (T1 TinputSignal, T2 &TpreviousSignal, edgeType Type, T3 Ttrigger ) -> bool {
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


/** 
 * @brief Triggering functions with a certain time interval
 * ! Max Targettime: about 49 days 
 * @param uiMillisOld 
 * @param uiTargetTime 
 * @return true if target time is reached
 * @return false if target time is not reached
 */
auto atUtilities::functionTrigger(uint32_t &uiMillisOld, uint32_t uiTargetTime) -> bool {

  uint16_t uiMillisTemp = millis() - uiMillisOld;

  if (uiMillisTemp >= uiTargetTime) {
    uiMillisOld += uiTargetTime;
    return true;
  } else {
    return false;
  }
}

auto atUtilities::secondToMs(uint16_t uiS) -> uint16_t{
  return uiS*1000;
}

auto atUtilities::fmap(float value, float in_min, float in_max, float out_min, float out_max) -> float
{
  float fScaled = 0.0;
  fScaled = out_min+(value-out_min)*((out_max-out_min)/(in_max-in_min));
  return fScaled;

}
