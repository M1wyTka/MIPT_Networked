#include <AgarClient.hpp>
#include <iostream>
#include <Packet/Packet.hpp>
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
}

AgarClient::~AgarClient()
{

}

void AgarClient::DisplayGame()
{

}

void AgarClient::ReadGameState(Packet *packet)
{
    int pair_cnt = (packet->header.size - sizeof(PacketHeader)) / (client_uid_.size() + sizeof(GameEntity));
    size_t offset = sizeof(AgarGame::EntityPair);
    for(int i = 0; i < pair_cnt; i++)
    {
        AgarGame::EntityPair* pair = reinterpret_cast<AgarGame::EntityPair*>(packet->data+offset*i);
        if(pair->entity.is_player)
            std::cout << "Player! " << std::endl;
        else
            std::cout << "AI! " << std::endl;
        std::cout <<"Pos:"<< pair->entity.pos.x << " " << pair->entity.pos.y << std::endl
        << "Target: "<< pair->entity.target.x << " " << pair->entity.target.y << std::endl
        << "Speed: "<< pair->entity.vel.x << " " << pair->entity.vel.y << std::endl;
    }
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

void AgarClient::Run()
{
    while (is_running_)
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
}