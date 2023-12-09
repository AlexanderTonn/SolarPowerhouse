#include "sharedMemory.h"

/**
 * @brief 
 * Intialising the shared memory
 * on Core M7:
 * - get the addresses of the <new> allocated arrays
 * - write the addresses to the shared memory
 * on Core M4:
 * - read the addresses from the shared memory
 * - assign the addresses to the arrays
 * @param none
 * @return none
 * 
 */
auto sharedMemory::memoryInit() -> void 
{
    #if defined(CORE_CM7)

    floatArrayAddress   = reinterpret_cast<uint32_t>(sharedFloatArray); 
    intArrayAddress     = reinterpret_cast<uint32_t>(sharedIntArray);
    uiArrayAddress      = reinterpret_cast<uint32_t>(sharedUnsignedIntArray); 
    boolArrayAddress    = reinterpret_cast<uint32_t>(sharedBoolArray);
    byteArrayAddress    = reinterpret_cast<uint32_t>(sharedByteArray); 
    
    *PtrAddrToFloatArray          = floatArrayAddress;
    *PtrAddrToIntArray            = intArrayAddress;
    *PtrAddrToUnsignedIntArray    = uiArrayAddress; 
    *PtrAddrToBoolArray           = boolArrayAddress; 
    *PtrAddrToByteArray           = byteArrayAddress;

    #elif defined(CORE_CM4)

    // Get the addresses of the arrays
   floatArrayAddress  = *PtrAddrToFloatArray;
   intArrayAddress    = *PtrAddrToIntArray;
   uiArrayAddress     = *PtrAddrToUnsignedIntArray;
   boolArrayAddress   = *PtrAddrToBoolArray;
   byteArrayAddress   = *PtrAddrToByteArray;

   *sharedFloatArray        = floatArrayAddress;       
   *sharedIntArray          = intArrayAddress;
   *sharedUnsignedIntArray  = uiArrayAddress;
   *sharedBoolArray         = boolArrayAddress;
   *sharedByteArray         = byteArrayAddress;

    #endif 
}
/**
 * @brief 
 * writing an unsigned int to the shared memory
 * uiValue is the input value
 * uiTargetIndex is the array index of the target variable
 * @param uiValue 
 * @param targetId 
 * @return none
 */
auto sharedMemory::writeSharedUnsignedInt(uint16_t uiValue, uiId targetId) -> void
{
    // wait for semaphore
    if(HAL_HSEM_FastTake(SEMAPHORE_ID) == HAL_OK)
    {
        sharedUnsignedIntArray[static_cast<unsigned int>(targetId)] = uiValue;

        // Release Semaphore
        HAL_HSEM_Release(SEMAPHORE_ID, 0);
    }
}
/**
 * @brief 
 * writing an int to the shared memory
 * uiValue is the input value
 * uiTargetIndex is the array index of the target variable
 * @param iValue 
 * @param targetId 
 */
auto sharedMemory::writeSharedInt(int16_t iValue, intId targetId) -> void
{
    // wait for semaphore
    if(HAL_HSEM_FastTake(SEMAPHORE_ID) == HAL_OK)
    {
        sharedIntArray[static_cast<unsigned int>(targetId)] = iValue;

        // Release Semaphore
        HAL_HSEM_Release(SEMAPHORE_ID, 0);
    }
}

/**
 * @brief 
 * writing an float to the shared memory
 * uiValue is the input value
 * uiTargetIndex is the array index of the target variable
 * @param fValue 
 * @param targetId 
 */
auto sharedMemory::writeSharedFloat(float fValue, floatId targetId) -> void
{
    // wait for semaphore
    if(HAL_HSEM_FastTake(SEMAPHORE_ID) == HAL_OK)
    {
        sharedFloatArray[static_cast<unsigned int>(targetId)] = fValue;

        // Release Semaphore
        HAL_HSEM_Release(SEMAPHORE_ID, 0);
    }
}
/**
 * @brief 
 * writing an bool to the shared memory
 * uiValue is the input value
 * uiTargetIndex is the array index of the target variable
 * @param xValue 
 * @param targetId 
 */
auto sharedMemory::writeSharedBool(bool xValue, boolId targetId) -> void
{
    // wait for semaphore
    if(HAL_HSEM_FastTake(SEMAPHORE_ID) == HAL_OK)
    {
        sharedBoolArray[static_cast<unsigned int>(targetId)] = xValue;

        // Release Semaphore
        HAL_HSEM_Release(SEMAPHORE_ID, 0);
    }
}
auto sharedMemory::writeSharedByte(uint8_t byValue, byteId targetId) -> void
{
    // wait for semaphore
    if(HAL_HSEM_FastTake(SEMAPHORE_ID) == HAL_OK)
    {
        sharedByteArray[static_cast<unsigned int>(targetId)] = byValue;

        // Release Semaphore
        HAL_HSEM_Release(SEMAPHORE_ID, 0);
    }
}

/**
 * @brief 
 * Reading an float from the shared memory
 * uiTargetIndex is the array index of the target variable
 * @param targetId 
 * @return float 
 */
auto sharedMemory::readSharedFloat(floatId targetId) -> float
{
    if(HAL_HSEM_FastTake(SEMAPHORE_ID) == HAL_OK)
    {
        auto fValue = sharedFloatArray[static_cast<unsigned int>(targetId)];

        // Release Semaphore
        HAL_HSEM_Release(SEMAPHORE_ID, 0);

        return fValue;
    }
}
/**
 * @brief 
 * Reading an int from the shared memory
 * uiTargetIndex is the array index of the target variable
 * @param targetId 
 * @return int 
 */
auto sharedMemory::readSharedInt(intId targetId) -> int16_t
{
    if(HAL_HSEM_FastTake(SEMAPHORE_ID) == HAL_OK)
    {
        auto iValue = sharedIntArray[static_cast<unsigned int>(targetId)];

        // Release Semaphore
        HAL_HSEM_Release(SEMAPHORE_ID, 0);

        return iValue;
    }
}
/**
 * @brief 
 * Reading an unsigned int from the shared memory
 * uiTargetIndex is the array index of the target variable
 * @param targetId 
 * @return uint16_t 
 */
auto sharedMemory::readSharedUnsignedInt(uiId targetId) -> uint16_t
{
    if(HAL_HSEM_FastTake(SEMAPHORE_ID) == HAL_OK)
    {
        auto uiValue = sharedUnsignedIntArray[static_cast<unsigned int>(targetId)];

        // Release Semaphore
        HAL_HSEM_Release(SEMAPHORE_ID, 0);

        return uiValue;
    }
}

/**
 * @brief 
 * Reading an bool from the shared memory
 * uiTargetIndex is the array index of the target variable
 * @param targetId 
 * @return bool 
 */
auto sharedMemory::readSharedBool(boolId targetId) -> bool
{
    if(HAL_HSEM_FastTake(SEMAPHORE_ID) == HAL_OK)
    {
        auto xValue = sharedBoolArray[static_cast<unsigned int>(targetId)];

        // Release Semaphore
        HAL_HSEM_Release(SEMAPHORE_ID, 0);

        return xValue;
    }
}

/**
 * @brief 
 * Reading an byte from the shared memory
 * uiTargetIndex is the array index of the target variable
 * @param targetId 
 * @return uint8_t 
 */
auto sharedMemory::readSharedByte(byteId targetId) -> uint8_t
{
    if(HAL_HSEM_FastTake(SEMAPHORE_ID) == HAL_OK)
    {
        auto byValue = sharedByteArray[static_cast<unsigned int>(targetId)];

        // Release Semaphore
        HAL_HSEM_Release(SEMAPHORE_ID, 0);

        return byValue;
    }
}