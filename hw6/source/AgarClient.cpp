#include <AgarClient.hpp>
#include <iostream>
#include <Packet.hpp>
#include <AgarGame.hpp>
#include "imgui.h"
#include "backends/imgui_impl_allegro5.h"

AgarClient::AgarClient(int port) :
                    lobby_port_(port),
                    is_running_(true),
                    last_frame(std::chrono::high_resolution_clock::now())
{
    InitAgarClient();
    InitAllegro();
}

void AgarClient::InitAgarClient()
{
    InitLobbyConnection();
}

AgarClient::~AgarClient()
{

}

void AgarClient::SendInput()
{
    Packet packet(PacketType::Input);
    PackData(packet, sizeof(Vec2), &input);

    ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(play_server_, 0,  en_packet);
}

void AgarClient::DisplayGame()
{
    bool done = false;
    bool redraw = false;
    ALLEGRO_EVENT event;

    while(!al_is_event_queue_empty(queue)) {
        ImGui_ImplAllegro5_ProcessEvent(&event);
        al_wait_for_event_timed(queue, &event, 0);
        switch (event.type) {
            case ALLEGRO_EVENT_TIMER:
                // game logic goes here.
                redraw = true;
                break;

            case ALLEGRO_EVENT_KEY_DOWN:
                ReadInputGame(event);
                break;
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                done = true;
        }
        if (done)
            is_running_ = false;
    }
    if(redraw)
    {
        RedrawGame();
    }
    if(any_input_)
    {
        any_input_ = false;
        SendInput();
        view_.SetPlayerInput(client_uid_, input);
        input = {.x = 0, .y = 0};
    }
}

void AgarClient::RedrawGame()
{
    if(client_uid_.empty())
        return;
    auto game_state = view_.GetGameView();

    ImGui_ImplAllegro5_NewFrame();
    ImGui::Render();

    al_clear_to_color(al_map_rgb(0, 0, 0));
    if(!game_state->empty()){
        for(int i = 0; i < game_state->size(); i++)
        {
            int x = game_state->at(i).entity.pos.x;
            int y = game_state->at(i).entity.pos.y;
            int size = game_state->at(i).entity.size;

            if(std::string(game_state->at(i).uid, client_uid_.length()) == client_uid_){
                al_draw_circle(x, y, size, al_map_rgb(255, 255, 255), 1);
            }
            else
                al_draw_circle(x, y, size, al_map_rgb(0, 255, 0), 1);

            //std::cout << "Pos " << x << " " << y << "\n"
            //          << "Target " << game_state->at(i).entity.target.x << " " << game_state->at(i).entity.target.y << "\n"
            //          << "Vel " << game_state->at(i).entity.vel.x << " " << game_state->at(i).entity.vel.y << "\n";
        }
    }
    al_flip_display();
}

void AgarClient::ReadInputGame(ALLEGRO_EVENT &event)
{
    static constexpr int speed = 50;
    switch (event.keyboard.keycode) {
        case ALLEGRO_KEY_W:
            input.y = -speed;
            any_input_ = true;
            break;
        case ALLEGRO_KEY_S:
            input.y = speed;
            any_input_ = true;
            break;
        case ALLEGRO_KEY_D:
            input.x = speed;
            any_input_ = true;
            break;
        case ALLEGRO_KEY_A:
            input.x = -speed;
            any_input_ = true;
            break;
    }
}

void AgarClient::InitAllegro()
{
    if(!al_init())
    {
        printf("couldn't initialize allegro\n");
        return;
    }

    if(!al_install_keyboard())
    {
        printf("couldn't initialize keyboard\n");
        return;
    }

    if(!al_install_mouse())
    {
        printf("couldn't initialize mouse\n");
        return;
    }

    timer = al_create_timer(1.0 / 60.0);
    if(!timer)
    {
        printf("couldn't initialize timer\n");
        return;
    }

    queue = al_create_event_queue();
    if(!queue)
    {
        printf("couldn't initialize queue\n");
        return;
    }

    disp = al_create_display(640, 480);
    if(!disp)
    {
        printf("couldn't initialize display\n");
        return;
    }

    font = al_create_builtin_font();
    if(!font)
    {
        printf("couldn't initialize font\n");
        return;
    }
    al_init_primitives_addon();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));


// Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplAllegro5_Init(disp);
}

void AgarClient::ReadGameState(Packet *packet)
{
    if(client_uid_.empty())
        return;

    int pair_cnt = (packet->header.size - sizeof(PacketHeader)) / (client_uid_.size() + sizeof(GameEntity));

    std::vector<AgarGame::EntityPair> cur_frame_info;
    cur_frame_info.resize(pair_cnt);
    size_t offset = sizeof(AgarGame::EntityPair);
    std::memmove(cur_frame_info.data(), packet->data, packet->header.size - sizeof(PacketHeader));

    view_.ForceGameView(std::move(cur_frame_info));
    view_.UpdateGameView(ping_);

    std::cout << pair_cnt << std::endl;
}

void AgarClient::PingBack()
{
    Packet packet(PacketType::Alive);

    ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(play_server_, 0,  en_packet);
}

void AgarClient::InitPlayServerConnection(Packet* packet)
{
    std::string play_port_str = std::string(reinterpret_cast<char*>(packet->data),
                                            packet->header.size - sizeof(PacketHeader));

    play_port_ = std::stoi(play_port_str);

    play_server_addr_.port = play_port_;

    enet_address_set_host(&play_server_addr_, "localhost");
    play_server_ = enet_host_connect(client_, &play_server_addr_, 2, 0);

    if (!play_server_)
    {
        std::cout <<"Cannot connect to play\n";
    }
}

void AgarClient::ReadLobbyState(Packet *packet)
{
    std::string message(reinterpret_cast<char*>(packet->data), packet->header.size - sizeof(PacketHeader));
    std::cout << message;

    room_headers_.clear();
    std::string token;
    std::stringstream ss(message);

    while (getline(ss, token, '\n')){
        room_headers_.push_back(token);
    }
}

void AgarClient::ReadLobbyServerPacket(ENetPacket *client_packet)
{
    static bool is_playing_ = false;
    Packet* packet = reinterpret_cast<Packet*>(client_packet->data);
    switch(packet->header.type)
    {
        case PacketType::ServerChange:
            if(!is_playing_)
                InitPlayServerConnection(packet);
            is_playing_ = true;
            is_lobby_ = false;
            break;
        case PacketType::Lobby:
            ReadLobbyState(packet);
            break;
        default:
            break;
    }
}

void AgarClient::ReadGameServerPacket(ENetPacket *client_packet)
{
 Packet* packet = reinterpret_cast<Packet*>(client_packet->data);
 switch(packet->header.type)
 {
     case PacketType::GameState:
         ReadGameState(packet);
         break;
     case PacketType::Alive:
         PingBack();
         break;
     case PacketType::Ping:
         ReadPing(packet);
         break;

     case PacketType::UIDReq:
     case PacketType::GameStart:
         client_uid_ = std::string(reinterpret_cast<char*>(packet->data), packet->header.size - sizeof(PacketHeader));
         break;
 }

}

void AgarClient::ReadPing(Packet *packet)
{
    ping_ = 1e-9*(*(reinterpret_cast<float*>(packet->data)));
}

void AgarClient::InitLobbyConnection()
{
    enet_initialize();
    client_ = enet_host_create(nullptr, 2, 4, 0, 0);

    enet_address_set_host(&lobby_server_addr_, "localhost");
    lobby_server_addr_.port = lobby_port_;

    lobby_server_ = enet_host_connect(client_, &lobby_server_addr_, 2, 0);

    if (!lobby_server_)
    {
        std::cout <<"Cannot connect to lobby\n";
    }
}

void AgarClient::SendStartGamePacket()
{
    Packet packet(PacketType::GameStart);

    ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(play_server_, 0,  en_packet);
}


void AgarClient::ProcessGameNetwork()
{
    ENetEvent event;
    while (enet_host_service(client_, &event, 10) > 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                std::cout << "Connection with " << event.peer->address.host << ":" << event.peer->address.port << " established\n";
                //SendStartGamePacket();
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                ReadGameServerPacket(event.packet);
                enet_packet_destroy(event.packet);
                break;
            default:
                break;
        };
    }
}

void AgarClient::LobbyStart()
{
    Packet packet(PacketType::GameStart);

    ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(lobby_server_, 0,  en_packet);
}

void AgarClient::ProcessLobbyNetwork()
{
    ENetEvent event;
    while (enet_host_service(client_, &event, 10) > 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                std::cout << "Connection with " << event.peer->address.host << ":" << event.peer->address.port << " established\n";
                //LobbyStart();
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                ReadLobbyServerPacket(event.packet);
                enet_packet_destroy(event.packet);
                break;
            default:
                break;
        };
    }
}

void AgarClient::ReadInputLobby(ALLEGRO_EVENT &event)
{

}

void AgarClient::SendRoomChoice(int i)
{
    Packet packet(PacketType::Lobby);
    PackData(packet, sizeof(i), &i);

    ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(lobby_server_, 0,  en_packet);
}

void AgarClient::LobbyCycle()
{
    while(is_lobby_)
    {
        ALLEGRO_EVENT event;
        while(!al_is_event_queue_empty(queue)) {
            ImGui_ImplAllegro5_ProcessEvent(&event);
            al_wait_for_event_timed(queue, &event, 0);
            switch (event.type) {
                case ALLEGRO_EVENT_KEY_DOWN:
                    ReadInputLobby(event);
                    break;
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    is_lobby_ = false;
                    is_running_ = false;
                    break;
            }
        }
        ProcessLobbyNetwork();

        ImGui_ImplAllegro5_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("TEST");                          // Create a window called "Hello, world!" and append into it.
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        if(!room_headers_.empty()){
            int room_idx = 0;
            for(int i = 0; i < room_headers_.size(); i++)
            {
                if (room_headers_[i].find("Room") != std::string::npos)
                {
                    int turn = room_idx++;
                    if (ImGui::Button(room_headers_[i].c_str()))
                    {
                        SendRoomChoice(turn);
                    }
                    continue;
                }
                ImGui::Text(room_headers_[i].c_str());
            }

            if(ImGui::Button("Start")) {
                LobbyStart();
            }

            if(ImGui::Button("Cancel")) {
                SendRoomChoice(-1);
            }
        }
        ImGui::End();

        ImGui::Render();
        al_clear_to_color(al_map_rgba_f(0, 0, 0, 1));
        ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
        al_flip_display();
    }
};

void AgarClient::RequestUid()
{
    Packet packet(PacketType::UIDReq);

    ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(play_server_, 0,  en_packet);
}

void AgarClient::GameCycle()
{
    al_start_timer(timer);


    while(is_running_)
    {
        if(client_uid_.empty())
            RequestUid();

        if(is_interpolating){
            dt = std::chrono::high_resolution_clock::now() - last_frame;
            last_frame = std::chrono::high_resolution_clock::now();
            view_.UpdateGameView(dt.count()*0.16);
        }

        ProcessGameNetwork();
        DisplayGame();
    }
}

void AgarClient::Run()
{
    LobbyCycle();
    GameCycle();

    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
}
