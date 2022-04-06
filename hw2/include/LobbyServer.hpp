#pragma once
#include <Server.hpp>
#include <Packet/Packet.hpp>

#include <deque>

class LobbyServer : Server
{
public:
    LobbyServer(int port, int play_port);
    ~LobbyServer();

    void Run();
private:
    void InitLobby();
    void InitPlayServerConnection();


    void ReadClientPacket(ENetPeer *source, ENetPacket *client_packet);
    void TellClientsToPlay(Packet* pack);
    void TellClientToPlay(ENetPeer* client);
    void TellClientsNewMember(ENetPeer* new_peer);


    ENetAddress game_server_addr_;

    int play_port_;
    bool is_started_;
};