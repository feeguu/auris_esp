#ifndef AURIS_PROTOCOL_H
#define AURIS_PROTOCOL_H

#include <cstdint>

enum AurisPacketType : uint32_t
{
  HANDSHAKE_TYPE = 0x01,
  LABEL_TYPE = 0x02,
  CONFIG_TYPE = 0x03,
  RESTART_TYPE = 0x04,
  CAPTIONS_TYPE = 0x05,
  GET_CONFIG = 0x06
};

typedef struct
{
  AurisPacketType type;
  uint32_t length;
} AurisPacketHeader;

typedef struct
{
  AurisPacketHeader header;
  uint8_t *payload;
} AurisPacket;

AurisPacket ProtocolHandlePacket(char *data);
void ProtocolDeallocatePacket(AurisPacket &packet);

#endif // AURIS_PROTOCOL_H
