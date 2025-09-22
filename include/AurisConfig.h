#ifndef AURIS_CONFIG_H
#define AURIS_CONFIG_H

#include "AurisProtocol.h"
#include "AurisConfig.h"

#include <string>

#define MAX_SSID_LENGTH 32
#define MAX_PASSWORD_LENGTH 64
#define MAX_LANGUAGE_LENGTH 8
#define MAX_VERSION_LENGTH 16
#define MAX_URL_LENGTH 128

#define DEFAULT_SSID "AURIS"
#define DEFAULT_PASSWORD "AURIS123"
#define DEFAULT_LANGUAGE "pt"
#define DEFAULT_FONT_SIZE 2
#define DEFAULT_CAPTIONS_ENABLED true
#define DEFAULT_VERSION "1.0.0"
#define DEFAULT_FIRMWARE_URL "https://github.com/feeguu/auris_esp/releases/latest/download/firmware.bin"


typedef struct __attribute__((packed))
{
  char ssid[MAX_SSID_LENGTH];
  char password[MAX_PASSWORD_LENGTH];
  char language[MAX_LANGUAGE_LENGTH];
  uint8_t font_size;
  char version[MAX_VERSION_LENGTH];
  char firmware_url[MAX_URL_LENGTH];
  bool captions_enabled;
} AurisConfig;

AurisConfig GetDefaultConfig();


bool UpdateConfig(AurisPacket &packet);
bool LoadConfig(AurisConfig &aurisConfig);

#endif // AURIS_CONFIG_H
