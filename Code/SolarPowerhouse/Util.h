#ifndef UTIL_H
#define UTIL_H

// Edge detection
#define RISING_EDGE 0
#define FALLING_EDGE 1

auto myMillis(uint64_t& uiMyMillis) -> void;

template<typename T1, typename T2, typename T3 >
auto edgeDetection (T1 TinputSignal, T2 &TpreviousSignal, byte byEdgeType, T3 Ttrigger ) -> bool;

auto functionTrigger(uint64_t &uiMillisOld, uint64_t uiTargetTime) -> bool;

#endif

