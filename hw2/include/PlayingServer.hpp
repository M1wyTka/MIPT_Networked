#pragma once
#include <Server.hpp>
#include <Packet/Packet.hpp>

#include <enet/enet.h>
#include <unordered_map>
#include <chrono>

class PlayingServer : public Server
{
public:
    PlayingServer(int port);
    ~PlayingServer();

    void Run();
private:

    struct ClientInfo
            {
                std::string u_id;
                std::chrono::duration<float> ping;
                std::chrono::steady_clock::time_point last_alive;
            };
    void InitPlayingServer();


    void SendSystemTime();
    void PingAllClients();
    void SendToOtherClients(ENetPeer* source, Packet* packet);
    void UpdateClientPing(ENetPeer* source);
    void CreateNewClient(ENetPeer* source);

    void ReadClientPacket(ENetPeer* source, ENetPacket* client_packet);
    std::unordered_map<ENetPeer*, ClientInfo> client_infos_;

    std::chrono::time_point<std::chrono::system_clock> begin_time_;
    std::chrono::time_point<std::chrono::system_clock> system_time_;
};