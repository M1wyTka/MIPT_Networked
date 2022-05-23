#include <LobbyServer.hpp>
#include <cstring>
#include <iostream>
#include <enet/enet.h>
#include <string>
#include <format>

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
    for(int i = 0; i < rooms_.size(); i++)
    {
        enet_address_set_host(&rooms_[i].game_addr_, "localhost");
        rooms_[i].game_addr_.port = play_port_+i;
    }
}

void LobbyServer::TellClientsToPlay()
{
    std::cout << "Sending to play" << std::endl;
    std::string server_address = std::to_string(game_server_addr_.port);

    Packet packet(PacketType::ServerChange);
    PackData(packet, server_address.size(), server_address.data());

    for (auto& client : client_peers_)
    {
        ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(client, 0,  en_packet);
    }
}

std::string LobbyServer::StateToString() {
    std::string message{};
    for(int i = 0; i < rooms_.size(); i++)
    {
        message += std::format("Room {} | {} | {}/{} players\n",
                               i, rooms_[i].Is_running ? "Running" : " Waiting ", rooms_[i].players.size(), max_size);
        for(int j = 0; j < rooms_[i].players.size(); j++){
            message += std::format("Player: {}\n", reinterpret_cast<int>(rooms_[i].players[j]));
        }

    }
    return message;
}

void LobbyServer::SendState() {

    std::string lobby_state = StateToString();
    for(const auto& client : client_peers_)
    {
        std::string temp;
        if(room_by_player[client] >= 0)
            temp = std::format("You are in room {}\n{}", room_by_player[client], lobby_state);
        else
            temp = std::format("You are in the lobby\n{}",  lobby_state);

        Packet pack(PacketType::Lobby);
        PackData(pack, temp.size(), temp.data());

        ENetPacket *en_packet = enet_packet_create(&pack, pack.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(client, 0,  en_packet);
    }
}

void LobbyServer::SendGameStartInRoom(int room_idx)
{
    if(room_idx == -1)
        return;

    auto& room = rooms_[room_idx];
    if(room.Is_running)
        return;

    std::cout << "Sending to play" << std::endl;
    std::string server_address = std::to_string(room.game_addr_.port);

    Packet packet(PacketType::ServerChange);
    PackData(packet, server_address.size(), server_address.data());

    auto& players = room.players;
    room.Is_running = true;

    for (auto& client : players)
    {
        ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(client, 0,  en_packet);
    }
}

void LobbyServer::UpdateLobbyState(ENetPeer *source, Packet *packet)
{
    int dst_room = *reinterpret_cast<int*>(packet->data);
    int prev_room_idx = room_by_player[source];
    if(prev_room_idx != -1) {
        auto& src_room = rooms_[prev_room_idx];
        src_room.players.erase(
                std::remove(src_room.players.begin(), src_room.players.end(), source),
                src_room.players.end()
        );
    }

    if(dst_room != -1 && rooms_[dst_room].players.size() == max_size)
        dst_room = -1;

    room_by_player[source] = dst_room;
    if(dst_room != -1) {
        auto& room = rooms_[dst_room];

        room.players.push_back(source);
        if(room.Is_running) {
            TellClientToPlay(source);
        }
        else if(room.players.size() == max_size)
            SendGameStartInRoom(dst_room);
    }

    SendState();
}

void LobbyServer::ReadClientPacket(ENetPeer *source, ENetPacket *client_packet)
{
    Packet* pack = reinterpret_cast<Packet*>(client_packet->data);
    switch (pack->header.type) {
        case PacketType::GameStart:
            SendGameStartInRoom(room_by_player[source]);
            break;
        case PacketType::Lobby:
            UpdateLobbyState(source, pack);
            break;
        default:
            break;
    }
}

void LobbyServer::TellClientToPlay(ENetPeer *client)
{
    if(room_by_player[client] == -1)
        return;

    auto& room = rooms_[room_by_player[client]];

    if(!room.Is_running)
        return;

    std::cout << "Sending to play" << std::endl;
    std::string server_address = std::to_string(room.game_addr_.port);

    Packet packet(PacketType::ServerChange);
    PackData(packet, server_address.size(), server_address.data());

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
                    room_by_player[event.peer] = -1;
                    //rooms_[0].players.push_back(event.peer);
                    SendState();
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
