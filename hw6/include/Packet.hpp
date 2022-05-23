#pragma once
#include <cinttypes>
#include <cstddef>
#include <utility>
#include <cstring>

enum class PacketType : uint32_t
{ None, Dumb, Connect, Alive, Ping, GameStart, GameState, ServerChange, Input, Lobby, UIDReq};


constexpr size_t PACKET_SIZE = 3000;

struct PacketHeader
{
    PacketType type;
    size_t size;
};

struct Packet {
    explicit Packet(PacketType type) : header({type, sizeof(PacketHeader)}) {};
    PacketHeader header {PacketType::None, sizeof(PacketHeader)};
    std::byte data[PACKET_SIZE];
};

struct UnboundPacket {
    PacketHeader header {PacketType::None, sizeof(PacketHeader)};
    std::byte data[];
};

static void PackData(Packet& packet, size_t size, void* data)
{
    auto offset = packet.header.size - sizeof(PacketHeader);
    if(packet.header.size + size > sizeof(Packet))
        throw 1;
    std::memmove(packet.data+offset, data, size);
    packet.header.size += size;
};