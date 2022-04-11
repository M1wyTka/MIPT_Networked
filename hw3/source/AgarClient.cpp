#include <AgarClient.hpp>
#include <iostream>
#include <Packet.hpp>
#include <AgarGame.hpp>


AgarClient::AgarClient(int port) :
                    play_port_(port),
                    is_running_(true)
{
    InitAgarClient();
}

void AgarClient::InitAgarClient()
{
    InitGameConnection();
    InitAllegro();
}

AgarClient::~AgarClient()
{

}

void AgarClient::SendInput()
{
    PacketHeader header = {
            .type = PacketType::Input,
            .size = sizeof(PacketHeader) + sizeof(Vec2)
    };
    Packet packet = {
            .header = header
    };

    std::memmove(packet.data, &input, sizeof(Vec2));

    ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(play_server_, 0,  en_packet);
}

void AgarClient::DisplayGame()
{
    bool done = false;
    bool redraw = true;
    bool isInp = false;
    ALLEGRO_EVENT event;

    while(!al_is_event_queue_empty(queue)) {
        al_wait_for_event_timed(queue, &event, 0);
        switch (event.type) {
            case ALLEGRO_EVENT_TIMER:
                // game logic goes here.
                redraw = true;
                break;

            case ALLEGRO_EVENT_KEY_DOWN:
                switch (event.keyboard.keycode) {
                    case ALLEGRO_KEY_W:
                        input.y = -10;
                        isInp = true;
                        break;
                    case ALLEGRO_KEY_S:
                        input.y = 10;
                        isInp = true;
                        break;
                    case ALLEGRO_KEY_D:
                        input.x = 10;
                        isInp = true;
                        break;
                    case ALLEGRO_KEY_A:
                        input.x = -10;
                        isInp = true;
                        break;
                }
            break;
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                done = true;
                is_running_ = false;
        }

        if (done)
            is_running_ = false;
    }
    if(redraw)
    {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        if(!cur_frame_info.empty()){
        for(int i = 0; i < cur_frame_info.size(); i++)
        {
            int x = cur_frame_info[i].entity.pos.x;
            int y = cur_frame_info[i].entity.pos.y;
            int size = cur_frame_info[i].entity.size;
            if(cur_frame_info[i].uid == client_uid_){
                al_draw_circle(x, y, size, al_map_rgb(255, 255, 255), 1);
                std::cout << x << " " << y << std::endl;
            }
            else
                al_draw_circle(x, y, size, al_map_rgb(0, 255, 0), 1);
        }
        }
        //al_draw_text(font, al_map_rgb(255, 255, 255), 0, 0, 0, "Hello world!");
        al_flip_display();

        redraw = false;
    }
    if(isInp)
    {
        isInp = false;
        SendInput();
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

    timer = al_create_timer(1.0 / 30.0);
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
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

}

void AgarClient::ReadGameState(Packet *packet)
{
    int pair_cnt = (packet->header.size - sizeof(PacketHeader)) / (client_uid_.size() + sizeof(GameEntity));

    cur_frame_info.resize(pair_cnt);
    size_t offset = sizeof(AgarGame::EntityPair);
    std::memmove(cur_frame_info.data(), packet->data, packet->header.size - sizeof(PacketHeader));

    std::cout << pair_cnt << std::endl;
    //for(int i = 0; i < pair_cnt; i++)
    //{
    //    AgarGame::EntityPair* pair = reinterpret_cast<AgarGame::EntityPair*>(packet->data+offset*i);
    //    if(pair->entity.is_player)
    //        std::cout << "Player! " << std::endl;
    //    else
    //        std::cout << "AI! " << std::endl;
    //    std::cout <<"Pos:"<< pair->entity.pos.x << " " << pair->entity.pos.y << std::endl
    //    << "Target: "<< pair->entity.target.x << " " << pair->entity.target.y << std::endl
    //    << "Speed: "<< pair->entity.vel.x << " " << pair->entity.vel.y << std::endl;
    //}
}

void AgarClient::ReadServerPacket(ENetPacket *client_packet)
{
 Packet* packet = reinterpret_cast<Packet*>(client_packet->data);
 switch(packet->header.type)
 {
     case PacketType::GameState:
         ReadGameState(packet);
         break;
     case PacketType::GameStart:
         client_uid_ = std::string(reinterpret_cast<char*>(packet->data), packet->header.size - sizeof(PacketHeader));
         break;
 }

}

void AgarClient::InitGameConnection()
{
    enet_initialize();
    client_ = enet_host_create(nullptr, 2, 4, 0, 0);

    enet_address_set_host(&play_server_addr_, "localhost");
    play_server_addr_.port = play_port_;

    play_server_ = enet_host_connect(client_, &play_server_addr_, 2, 0);

    if (!play_server_)
    {
        std::cout <<"Cannot connect to lobby" << std::endl;
    }
}

void AgarClient::SendStartGamePacket()
{
    PacketHeader header = {
            .type = PacketType::GameStart,
            .size = sizeof(PacketHeader)
    };
    Packet packet = {
            .header = header
    };

    ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(play_server_, 0,  en_packet);
}


void AgarClient::ProcessNetwork()
{
    ENetEvent event;
    std::string command;

    while (enet_host_service(client_, &event, 10) > 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
                SendStartGamePacket();
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                ReadServerPacket(event.packet);

                enet_packet_destroy(event.packet);
                break;
            default:
                break;
        };
    }
}

void AgarClient::Run()
{
    al_start_timer(timer);
    while(is_running_)
    {
        ProcessNetwork();
        DisplayGame();
        cur_frame_info.clear();
    }

    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
}
