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

    void InitGameConnection();

    void DisplayGame();
    void ProcessNetwork();

    void ReadGameState(Packet* packet);
    void ReadPing(Packet* packet);

    void SendStartGamePacket();
    void ReadInput(ALLEGRO_EVENT& event);
    void SendInput();

    void ReadServerPacket(ENetPacket *client_packet);
    void PingBack();

    void RedrawGame();

    Vec2 input {};
    bool any_input_{false};

    double ping_ {0};

    int play_port_;
    bool is_running_;
    std::string client_uid_;

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
};