#pragma once
#include <enet/enet.h>
#include <string>
#include <vector>
#include <Packet.hpp>

#include <AgarGame.hpp>

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

    void SendStartGamePacket();
    void SendInput();

    void ReadServerPacket(ENetPacket *client_packet);

    std::vector<AgarGame::EntityPair> cur_frame_info;

    Vec2 input {};

    int play_port_;
    bool is_running_;
    std::string client_uid_;

    ALLEGRO_TIMER* timer;
    ALLEGRO_EVENT_QUEUE* queue;
    ALLEGRO_DISPLAY* disp;
    ALLEGRO_FONT* font;

    ENetHost *client_;
    ENetAddress play_server_addr_;
    ENetPeer *play_server_;
};