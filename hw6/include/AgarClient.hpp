#pragma once
#include <enet/enet.h>
#include <string>
#include <vector>
#include <chrono>

#include <Packet.hpp>

#include <AgarGame.hpp>
#include <AgarGameView.hpp>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

class AgarClient
{
public:
    explicit AgarClient(int port);
    ~AgarClient();

    void Run();
private:
    void InitAgarClient();
    void InitAllegro();

    void InitLobbyConnection();

    void LobbyCycle();
    void GameCycle();

    void DisplayGame();

    void ProcessLobbyNetwork();
    void ProcessGameNetwork();

    void ReadGameState(Packet* packet);
    void ReadPing(Packet* packet);

    void SendStartGamePacket();
    void ReadInputGame(ALLEGRO_EVENT& event);
    void ReadInputLobby(ALLEGRO_EVENT& event);
    void SendInput();

    void InitPlayServerConnection(Packet* packet);
    void ReadGameServerPacket(ENetPacket *client_packet);
    void ReadLobbyServerPacket(ENetPacket *client_packet);

    void ReadLobbyState(Packet* packet);

    void PingBack();

    void RedrawGame();

    void LobbyStart();

    void SendRoomChoice(int i);

    void RequestUid();

private:
    std::vector<std::string> room_headers_{};

    Vec2 input {};
    bool any_input_{false};

    double ping_ {0};

    int lobby_port_;
    int play_port_;

    bool is_running_;
    std::string client_uid_;

    bool is_lobby_{true};

    bool is_interpolating {true};
    std::chrono::duration<double> dt;
    std::chrono::high_resolution_clock::time_point last_frame;

    AgarGameView view_;

    ALLEGRO_TIMER* timer;
    ALLEGRO_EVENT_QUEUE* queue;
    ALLEGRO_DISPLAY* disp;
    ALLEGRO_FONT* font;

    ENetHost *client_;

    ENetAddress play_server_addr_;
    ENetPeer *play_server_;

    ENetAddress lobby_server_addr_;
    ENetPeer *lobby_server_;
};