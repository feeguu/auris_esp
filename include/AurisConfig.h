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

bool UpdateConfig(AurisPacket &packet);
bool LoadConfig(AurisConfig &aurisConfig);

#endif // AURIS_CONFIG_H
