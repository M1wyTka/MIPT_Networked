#pragma once
#include <enet/enet.h>
#include <string>
#include <Packet/Packet.hpp>

class AgarClient
{
public:
    explicit AgarClient(int port);
    ~AgarClient();

    void Run();
private:
    void InitAgarClient();

    void InitGameConnection();

    void DisplayGame();

    void ReadGameState(Packet* packet);

    void SendStartGamePacket();
    void ReadServerPacket(ENetPacket *client_packet);


    int play_port_;
    bool is_running_;
    std::string client_uid_;

    ENetHost *client_;
    ENetAddress play_server_addr_;
    ENetPeer *play_server_;
};