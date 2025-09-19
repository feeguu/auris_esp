#include "AurisProtocol.h"
#include <cstring>

AurisPacket ProtocolHandlePacket(char *data)
{
  AurisPacket packet;
  packet.payload = nullptr;

  if (data == nullptr)
  {
    packet.header.type = static_cast<AurisPacketType>(0);
    packet.header.length = 0;
    return packet;
  }

  // Extract header
  AurisPacketHeader *header = reinterpret_cast<AurisPacketHeader *>(data);
  packet.header.type = header->type;
  packet.header.length = header->length;

  // Validate length
  if (packet.header.length > 0 && packet.header.length <= 1024) // Arbitrary max length
  {
    packet.payload = new uint8_t[packet.header.length + 1];
    packet.payload[packet.header.length] = '\0'; // Null-terminate for safety
    memcpy(packet.payload, data + sizeof(AurisPacketHeader), packet.header.length);
  }
  else
  {
    packet.payload = nullptr; // Invalid length
  }

  return packet;
}

void ProtocolDeallocatePacket(AurisPacket &packet)
{
  if (packet.payload != nullptr)
  {
    delete[] packet.payload;
    packet.payload = nullptr;
  }
}
