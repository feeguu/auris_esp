#ifndef AURIS_EEPROM_H
#define AURIS_EEPROM_H

#include <EEPROM.h>
#include "AurisConfig.h"

#define EEPROM_SENTINEL_VALUE 0xF0DA
#define EEPROM_START_ADDRESS 2

#define EEPROM_SIZE sizeof(AurisConfig) + EEPROM_START_ADDRESS

void EEPROMInit();

bool EEPROMReadConfig(AurisConfig &config);
bool EEPROMWriteConfig(const AurisConfig &config);

#endif // AURIS_EEPROM_H
