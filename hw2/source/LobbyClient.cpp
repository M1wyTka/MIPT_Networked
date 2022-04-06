#include <LobbyClient.hpp>
#include <iostream>
#include <cstring>

LobbyClient::LobbyClient(int lobby_port) :
                            lobby_port_(lobby_port),
                            play_server_(nullptr),
                            is_running_(true),
                            is_playing_(false)
{
    InitClient();
}

LobbyClient::~LobbyClient()
{

}

void LobbyClient::InitClient()
{
    InitLobbyConnection();
}

void LobbyClient::InitLobbyConnection()
{
    client_ = enet_host_create(nullptr, 2, 4, 0, 0);

    enet_address_set_host(&lobby_server_addr_, "localhost");
    lobby_server_addr_.port = lobby_port_;

    lobby_server_ = enet_host_connect(client_, &lobby_server_addr_, 2, 0);

    if (!lobby_server_)
    {
        std::cout <<"Cannot connect to lobby" << std::endl;
    }
}

void LobbyClient::InitPlayServerConnection(Packet* packet)
{
    std::string play_port_str = std::string(reinterpret_cast<char*>(packet->data),
                                                                        packet->header.size - sizeof(PacketHeader));

    play_port_ = std::stoi(play_port_str);

    play_server_addr_.port = play_port_;

    enet_address_set_host(&play_server_addr_, "localhost");
    play_server_ = enet_host_connect(client_, &play_server_addr_, 2, 0);
}

void LobbyClient::PingBack()
{
    PacketHeader header = {
            .type = PacketType::Alive,
            .size = sizeof(PacketType)

    };
    Packet packet = {
            .header = header
    };

    ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(play_server_, 0,  en_packet);
}

void LobbyClient::PrintChatMessages(Packet *packet)
{
    std::cout << std::string(reinterpret_cast<char*>(packet->data), packet->header.size - sizeof(PacketHeader)) << std::endl;
}

void LobbyClient::ReadServerPacket(ENetPacket *client_packet)
{
    Packet* pack = reinterpret_cast<Packet*>(client_packet->data);
    switch (pack->header.type) {
        case PacketType::GameStart:
            if(!is_playing_)
                InitPlayServerConnection(pack);
            is_playing_ = true;
            break;
        case PacketType::Alive:
            PingBack();
            break;
        case PacketType::Dumb:
            PrintChatMessages(pack);
            break;
        default:
            break;
    }
}

void LobbyClient::SendStartGamePacket()
{
    PacketHeader header = {
            .type = PacketType::GameStart,
            .size = sizeof(PacketHeader)
    };
    Packet packet = {
            .header = header
    };

    ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(lobby_server_, 0,  en_packet);
}

static std::string get_input()
{
    std::string input;
    std::cin >> input;
    return input;
}

void LobbyClient::Run()
{
    //std::future<std::string> future = std::async(get_input);
    while (is_running_)
    {
        ENetEvent event;
        std::string command;

        //bool is_input_ready = future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;

        while (enet_host_service(client_, &event, 10) > 0)//  || is_input_ready)
        {
            //if (is_input_ready && future.get() == "start") {
            //    is_input_ready = false;
//
            //    //future = std::async(get_input);
            //    SendStartGamePacket();
            //}
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