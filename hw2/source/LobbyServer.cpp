#include <LobbyServer.hpp>
#include <cstring>
#include <iostream>
#include <enet/enet.h>

LobbyServer::LobbyServer(int port, int play_port) :
                            Server(port, 32, 2),
                            play_port_(play_port),
                            is_started_(false)
{
    InitLobby();
}

LobbyServer::~LobbyServer()
{

}

void LobbyServer::InitLobby()
{
    InitPlayServerConnection();
}

void LobbyServer::InitPlayServerConnection()
{
    enet_address_set_host(&game_server_addr_, "localhost");
    game_server_addr_.port = play_port_;
}

void LobbyServer::TellClientsToPlay(Packet* packet)
{
    std::string server_address = std::to_string(game_server_addr_.port);
    std::memmove(packet->data, server_address.data(), server_address.size());
    packet->header.size += server_address.size();

    for (auto& client : client_peers_)
    {
        ENetPacket *en_packet = enet_packet_create(packet, packet->header.size, ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(client, 0,  en_packet);
    }
}

void LobbyServer::TellClientsNewMember(ENetPeer* new_peer)
{
    std::string notice = "New client_ joined lobby : " + std::to_string(new_peer->address.port);

    PacketHeader header = {
            .type = PacketType::Dumb,
            .size = sizeof(PacketHeader) + notice.size(),
    };

    Packet packet = {
            .header = header
    };
    std::memmove(packet.data, notice.data(), notice.size());

    for (auto& client : client_peers_)
    {
        if(client == new_peer)
            continue;
        ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(client, 0,  en_packet);
    }

    if(std::find(client_peers_.begin(), client_peers_.end(), new_peer) != client_peers_.end())
        client_peers_.push_back(new_peer);
}

void LobbyServer::ReadClientPacket(ENetPeer *source, ENetPacket *client_packet)
{
    Packet* pack = reinterpret_cast<Packet*>(client_packet->data);
    switch (pack->header.type) {
        case PacketType::GameStart:
            if(!is_started_){
                is_started_ = true;
                TellClientsToPlay(pack);
            }
            break;
        default:
            break;
    }
}

void LobbyServer::TellClientToPlay(ENetPeer *client)
{
    std::cout << "Sending to play" << std::endl;
    std::string server_address = std::to_string(game_server_addr_.port);

    PacketHeader header = {
            .type = PacketType::GameStart,
            .size = sizeof (PacketHeader) + server_address.size()
    };
    Packet packet = {
            .header = header
    };
    std::memmove(packet.data, server_address.data(), server_address.size());

    ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(client, 0,  en_packet);
}

void LobbyServer::Run()
{
    while (is_running_)
    {
        ENetEvent event;
        while (enet_host_service(server, &event, 10) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    client_peers_.push_back(event.peer);
                    if(!is_started_){
                        TellClientsNewMember(event.peer);
                    }
                    else
                        TellClientToPlay(event.peer);
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    ReadClientPacket(event.peer, event.packet);
                    enet_packet_destroy(event.packet);
                    break;
                default:
                    break;
            };
        }
    }
}
