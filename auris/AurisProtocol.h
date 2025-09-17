#ifndef AURIS_PROTOCOL_H
#define AURIS_PROTOCOL_H

#include <cstdint>

enum AurisPacketType : uint32_t
{
  HANDSHAKE_TYPE = 0x01,
  LABEL_TYPE = 0x02,
  CONFIG_TYPE = 0x03,
  RESTART_TYPE = 0x04
};

typedef struct AurisPacketHeader
{
  AurisPacketType type;
  uint32_t length;
};

typedef struct AurisPacket
{
  AurisPacketHeader header;
  uint8_t *payload;
};

AurisPacket ProtocolHandlePacket(char *data);

#endif // AURIS_PROTOCOL_H
