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

AurisConfig GetDefaultConfig()
{
  AurisConfig config;
  memset(&config, 0, sizeof(AurisConfig));
  strncpy(config.ssid, DEFAULT_SSID, MAX_SSID_LENGTH);
  strncpy(config.password, DEFAULT_PASSWORD, MAX_PASSWORD_LENGTH);
  strncpy(config.language, DEFAULT_LANGUAGE, MAX_LANGUAGE_LENGTH);
  config.font_size = DEFAULT_FONT_SIZE;
  strncpy(config.version, DEFAULT_VERSION, MAX_VERSION_LENGTH);
  strncpy(config.firmware_url, DEFAULT_FIRMWARE_URL, MAX_URL_LENGTH);
  config.captions_enabled = DEFAULT_CAPTIONS_ENABLED;
  return config;
}