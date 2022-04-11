#pragma once
#include <Server.hpp>
#include <AgarGame.hpp>
#include <unordered_map>
#include <enet/enet.h>
#include <chrono>

class AgarServer : public Server
        {
public:
    explicit AgarServer(int port);
    ~AgarServer();

    void Run() override;
private:
    void InitAgarServer();

    void AddNewPlayer(ENetPeer* new_peer);
    void KillPlayer(ENetPeer* new_peer);
    void SendUIDBack(ENetPeer* back_peer, std::string uid);

    void ReadClientPacket(ENetPeer* peer, ENetPacket* packet);

    void UpdateGameState();
    void SendGameState();

    AgarGame game_;
    std::unordered_map<ENetPeer*, std::string> UID_by_peer_{};

    std::chrono::duration<double> dt;
    std::chrono::high_resolution_clock::time_point last_frame;
};