#include <AurisEEPROM.h>

void EEPROMInit()
{
  EEPROM.begin(EEPROM_SIZE);

  // Check sentinel value
  uint16_t sentinel = 0;
  EEPROM.get(0, sentinel);
  if (sentinel != EEPROM_SENTINEL_VALUE)
  {
    // Initialize EEPROM with default values
    sentinel = EEPROM_SENTINEL_VALUE;

    EEPROM.put(0, sentinel);

    AurisConfig defaultConfig = {};
    strncpy(defaultConfig.ssid, "AURIS", sizeof(defaultConfig.ssid) - 1);
    strncpy(defaultConfig.password, "AURIS123", sizeof(defaultConfig.password) - 1);
    strncpy(defaultConfig.language, "pt", sizeof(defaultConfig.language) - 1);

    EEPROM.put(EEPROM_START_ADDRESS, defaultConfig);
  }

  EEPROM.commit();
}

bool EEPROMReadConfig(AurisConfig &config)
{
  uint16_t sentinel = 0;
  EEPROM.get(0, sentinel);
  if (sentinel != EEPROM_SENTINEL_VALUE)
  {
    // Invalid sentinel, no valid config
    return false;
  }

  EEPROM.get(EEPROM_START_ADDRESS, config);
  return true;
}

bool EEPROMWriteConfig(const AurisConfig &config)
{
  uint16_t sentinel = EEPROM_SENTINEL_VALUE;
  EEPROM.put(0, sentinel);
  EEPROM.put(EEPROM_START_ADDRESS, config);
  return EEPROM.commit();
}
