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
auto edgeDetection (T1 TinputSignal, T2 &TpreviousSignal, byte byEdgeType, T3 Ttrigger ) -> bool {
  // RISING EDGE detection
  if (byEdgeType == RISING_EDGE) {
    if (TinputSignal >= Ttrigger && TpreviousSignal < Ttrigger ) {
      TpreviousSignal = TinputSignal;
      return true;
    }
    // Resetting the signal if Ttrigger was reached
    else if ((TinputSignal >= Ttrigger && TpreviousSignal >= Ttrigger) || (TinputSignal < Ttrigger && TpreviousSignal >= Ttrigger)) {
      TpreviousSignal = TinputSignal;
      return false;
    }
  }
  // FALLING EDGE detection
  if (byEdgeType == FALLING_EDGE) {
    //rising edge
    if (TinputSignal <= Ttrigger && TpreviousSignal > Ttrigger) {
      TpreviousSignal = TinputSignal;
      return true;
    }
    else if ((TinputSignal <= Ttrigger && TpreviousSignal <= Ttrigger) || (TinputSignal > Ttrigger && TpreviousSignal <= Ttrigger)) {
      TpreviousSignal = TinputSignal;
      return false;
    }
  }
}

auto functionTrigger(uint64_t &uiMillisOld, uint64_t uiTargetTime) -> bool {
  uint64_t uiMillisActual = 0;
  myMillis(uiMillisActual);

  if (uiMillisActual - uiMillisOld >= uiTargetTime) {
    myMillis(uiMillisOld);
    return true;
  } else {
    return false;
  }

}