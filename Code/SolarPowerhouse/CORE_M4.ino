#include "CORE_M4.h"


auto CORE_M4::getVarsFromSharedMemory() -> void
{
    static uint32_t uiMillis = millis();

    if(functionTrigger(uiMillis, 1000))
    {
      fBatteryVoltage = ipc.readSharedFloat(sharedMemory::floatId::BATTERY_VOLTAGE);
      fBatteryCurrent = ipc.readSharedFloat(sharedMemory::floatId::BATTERY_CURRENT);
      fSolarVoltage = ipc.readSharedFloat(sharedMemory::floatId::SOLAR_VOLTAGE);
      fSolarCurrent = ipc.readSharedFloat(sharedMemory::floatId::SOLAR_CURRENT);
      fBatTodayVoltMin = ipc.readSharedFloat(sharedMemory::floatId::BAT_TODAY_VOLT_MIN);
      fBatTodayVoltMax = ipc.readSharedFloat(sharedMemory::floatId::BAT_TODAY_VOLT_MAX);
      fBatTodayCurrMax = ipc.readSharedFloat(sharedMemory::floatId::BAT_TODAY_CURR_MAX);

      uiCurrentSolarPower = ipc.readSharedUnsignedInt(sharedMemory::uiId::SOLAR_POWER);
      uiBatTodayPwrMax = ipc.readSharedUnsignedInt(sharedMemory::uiId::BAT_TODAY_PWR_MAX);
      uiBatTodayPwrGeneration = ipc.readSharedUnsignedInt(sharedMemory::uiId::BAT_TODAY_PWR_GENERATION);

      uiBatteryTemperature = ipc.readSharedByte(sharedMemory::byteId::BATTERY_TEMPERATURE);
      uiMpptChargerTemperature = ipc.readSharedByte(sharedMemory::byteId::MPPT_CHARGER_TEMPERATURE);

      xMpptChargerLoadActive = ipc.readSharedBool(sharedMemory::boolId::MPPT_CHARGER_LOAD_ACTIVE);
    }
}