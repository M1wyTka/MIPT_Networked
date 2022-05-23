#pragma once
#include <Server.hpp>
#include <AgarGame.hpp>

#include <enet/enet.h>

#include <unordered_map>
#include <chrono>

class AgarServer : public Server
        {
public:
    explicit AgarServer(int port);
    ~AgarServer();

    void Run() override;
private:
    struct ClientInfo
    {
        std::string u_id;
        std::chrono::duration<float> ping;
        std::chrono::steady_clock::time_point last_alive;
    };

    using PeerClientMap =  std::unordered_map<ENetPeer*, ClientInfo>;
    using PeerStringMap =  std::unordered_map<ENetPeer*, std::string>;

    void InitAgarServer();

    void CreateNewClient(ENetPeer* new_peer);
    void AddNewPlayer(ENetPeer* new_peer);
    void KillPlayer(ENetPeer* new_peer);
    void SendUIDBack(ENetPeer* back_peer, std::string uid);

    void ProcessENet();
    void ReadClientPacket(ENetPeer* peer, ENetPacket* packet);
    void PingAll();
    void UpdateClientPing(ENetPeer* peer);

    void UpdateGameState();
    void SendGameState();

    static constexpr int PingPeriod = 0.01;

    PeerClientMap client_infos_;
    std::chrono::time_point<std::chrono::system_clock> system_time_;

    AgarGame game_;
    PeerStringMap UID_by_peer_{};

    std::chrono::duration<double> dt;
    std::chrono::high_resolution_clock::time_point last_frame;
};