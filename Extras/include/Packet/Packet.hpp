#pragma once
#include <cinttypes>
#include <cstddef>
#include <utility>

enum class PacketType : uint32_t
        { None, Dumb, Connect, Alive, Ping, GameStart, GameState, ServerChange};


constexpr size_t PACKET_SIZE = 1500;

struct PacketHeader
        {
            PacketType type;
            size_t size;
        };

struct Packet {
    PacketHeader header;
    std::byte data[PACKET_SIZE];
};

struct UnboundPacket {
    PacketHeader header;
    std::byte data[];
};