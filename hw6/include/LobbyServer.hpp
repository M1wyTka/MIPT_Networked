#pragma once
#include <Server.hpp>
#include <Packet.hpp>

#include <vector>
#include <unordered_map>

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
    void TellClientsToPlay();
    void TellClientToPlay(ENetPeer* client);
    void TellClientsNewMember(ENetPeer* new_peer);

    void UpdateLobbyState(ENetPeer *source, Packet* pack);

    void SendGameStartInRoom(int room_idx);

private:
    std::string StateToString();

    void SendState();

    static constexpr int max_size{2};
    // Room - idx + player to move to server
    struct Room
    {
        bool Is_running {false};
        std::vector<ENetPeer*> players{};
        ENetAddress game_addr_{};
    };

    std::vector<Room> rooms_{3};
    std::unordered_map<ENetPeer*, int> room_by_player;

    ENetAddress game_server_addr_;

    int play_port_;
    bool is_started_;
};