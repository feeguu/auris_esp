#ifndef AURIS_CONFIG_H
#define AURIS_CONFIG_H

#include "AurisProtocol.h"
#include "AurisConfig.h"

typedef struct
{
  char ssid[32];
  char password[64];
  char language[3];
  uint8_t font_size;
} AurisConfig;

bool UpdateConfig(AurisPacket &packet);
bool LoadConfig(AurisConfig &aurisConfig);

#endif // AURIS_CONFIG_H
