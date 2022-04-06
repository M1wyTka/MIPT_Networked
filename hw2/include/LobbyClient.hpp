#pragma once
#include <Packet/Packet.hpp>

#include <enet/enet.h>
#include <unordered_map>
#include <future>

class LobbyClient
{
public:
    LobbyClient(int lobby_port);
    ~LobbyClient();

    void Run();
private:

    void InitClient();
    void InitLobbyConnection();
    void InitPlayServerConnection(Packet* packet);

    void ReadServerPacket(ENetPacket *client_packet);
    void PrintChatMessages(Packet* packet);
    void PingBack();

    void SendStartGamePacket();

    bool is_running_;
    bool is_playing_;
    int lobby_port_;
    int play_port_;

    ENetHost *client_;

    ENetAddress play_server_addr_;
    ENetPeer *play_server_;

    ENetAddress lobby_server_addr_;
    ENetPeer *lobby_server_;
};