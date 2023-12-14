

#ifndef UTIL_H
#define UTIL_H

// Edge detection
class atUtilities 
{
    private:

    public: 
    enum class edgeType{
    RISING_EDGE,
    FALLING_EDGE
    };

    auto myMillis(uint64_t& uiMyMillis) -> void;

    template<typename T1, typename T2, typename T3 >
    auto edgeDetection (T1 TinputSignal, T2 &TpreviousSignal, edgeType Type, T3 Ttrigger ) -> bool;

    auto functionTrigger(uint32_t &uiMillisOld, uint32_t uiTargetTime) -> bool;
    auto secondToMs(uint16_t uiS) -> uint16_t;
    auto fmap(float value, float in_min, float in_max, float out_min, float out_max) -> float;



};

atUtilities util;

#endif

