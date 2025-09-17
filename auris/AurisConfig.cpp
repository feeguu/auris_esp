#include "AurisConfig.h"
#include "AurisEEPROM.h"
#include <Arduino.h>

bool UpdateConfig(AurisPacket &packet)
{
  if (packet.header.type != CONFIG_TYPE || packet.payload == nullptr || packet.header.length == 0)
  {
    // Invalid packet
    return false;
  }
  if (packet.header.length != sizeof(AurisConfig))
  {
    // Invalid length
    return false;
  }

  AurisConfig newConfig;
  memcpy(&newConfig, packet.payload, sizeof(AurisConfig));
  if (!EEPROMWriteConfig(newConfig))
  {
    // Failed to write config
    return false;
  }

  // Reboot esp
  ESP.restart();
  return true;
}

bool LoadConfig(AurisConfig &config)
{
  return EEPROMReadConfig(config);
}
