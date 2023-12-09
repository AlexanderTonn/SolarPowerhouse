#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include "stm32h7xx_hal.h"

#define SEMAPHORE_ID 0



class sharedMemory
{
    public:
    
    // STEP 1: 
    // List here your variables which you want to share between the cores
    // while calling of allocSharedMemory() -> void in the setup() function the memory is allocated depending on the lists below

    // List of float variables
    enum class floatId
    {
        BATTERY_VOLTAGE,
        BATTERY_CURRENT,
        SOLAR_VOLTAGE,
        SOLAR_CURRENT,
        BAT_TODAY_VOLT_MIN, // min voltage of the day
        BAT_TODAY_VOLT_MAX, // max voltage of the day
        BAT_TODAY_CURR_MAX, // max current of the day
        SIZE

    };
    // List of int variables
    enum class intId
    {
        SIZE
    };
    // List of unsigned int variables
    enum class uiId
    {
        SOLAR_POWER,
        BAT_TODAY_PWR_MAX, // max charge power of the day
        BAT_TODAY_PWR_GENERATION, // max generation power of the day
        SIZE
    };
    // List of byte variables
    enum class byteId
    {
        BATTERY_TEMPERATURE,
        MPPT_CHARGER_TEMPERATURE,
        SIZE
    };
    // List of bool variables
    enum class boolId
    {
        MPPT_CHARGER_LOAD_ACTIVE, // mppt charger load 0 = off, 1 = on
        SIZE
    };

    private: 

    uint32_t floatArrayAddress;  
    uint32_t intArrayAddress;    
    uint32_t uiArrayAddress;    
    uint32_t boolArrayAddress;   
    uint32_t byteArrayAddress; 

    // Store the pointer address in SRAM 2
    // Use the last bytes of the SRAM 2 for preventing overwriting of the pointer address by programm code
    // reinterpret_cast is used to convert the addresses to volatile pointers of the appropriate types
    volatile uint32_t *PtrAddrToFloatArray = reinterpret_cast<volatile uint32_t*>(0x3001FFFB); 
    volatile uint32_t *PtrAddrToIntArray = reinterpret_cast<volatile uint32_t*>(0x3001FFF7);
    volatile uint32_t *PtrAddrToUnsignedIntArray = reinterpret_cast<volatile uint32_t*>(0x3001FFF3);
    volatile uint32_t *PtrAddrToBoolArray = reinterpret_cast<volatile uint32_t*>(0x3001FFEF);
    volatile uint32_t *PtrAddrToByteArray = reinterpret_cast<volatile uint32_t*>(0x3001FFEB);  

    // Make Memory Accessable for M7 and M4
    #if defined(CORE_CM7)
    // Create Pointer and allocate storage for shared memory
    volatile float* sharedFloatArray = new float[static_cast<int>(floatId::SIZE)]; 
    volatile int16_t* sharedIntArray = new int16_t[static_cast<int>(intId::SIZE)];
    volatile uint16_t* sharedUnsignedIntArray = new uint16_t[static_cast<int>(uiId::SIZE)];
    volatile bool* sharedBoolArray = new bool[static_cast<int>(boolId::SIZE)];
    volatile uint8_t* sharedByteArray = new uint8_t[static_cast<int>(byteId::SIZE)];
    
    #elif defined(CORE_CM4)

    volatile float* sharedFloatArray;          // Pointer to shared memory
    volatile int16_t* sharedIntArray;          // Pointer to shared memory
    volatile uint16_t* sharedUnsignedIntArray; // Pointer to shared memory
    volatile bool* sharedBoolArray;                // Pointer to shared memory
    volatile uint8_t* sharedByteArray;          // Pointer to shared memory
  
    #else 
    #error "This Board don't have a Core M7 or M4"
    #endif

    public:

    auto memoryInit() -> void;

    auto writeSharedUnsignedInt(uint16_t uiValue, uiId targetId) -> void;
    auto writeSharedInt(int16_t iValue, intId targetId) -> void;
    auto writeSharedFloat(float fValue, floatId targetId) -> void;
    auto writeSharedBool(bool bValue, boolId targetId) -> void;
    auto writeSharedByte(uint8_t bValue, byteId targetId) -> void;

    auto readSharedUnsignedInt(uiId targetId) -> uint16_t;
    auto readSharedInt(intId targetId) -> int16_t;
    auto readSharedFloat(floatId targetId) -> float;
    auto readSharedBool(boolId targetId) -> bool;
    auto readSharedByte(byteId targetId) -> uint8_t;

    #if defined (CORE_CM7)
    ~sharedMemory()
    {
        delete[] sharedFloatArray;
        delete[] sharedIntArray;
        delete[] sharedUnsignedIntArray;
        delete[] sharedBoolArray;
        delete[] sharedByteArray;
    }
    #endif


};

#endif // SHARED_MEMORY_H
