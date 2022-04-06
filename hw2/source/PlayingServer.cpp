#include <PlayingServer.hpp>

#include <string>
#include <cstring>

PlayingServer::PlayingServer(int port) :
                            Server(port, 32, 2),
                            begin_time_(std::chrono::system_clock::now())
{
    InitPlayingServer();
}

PlayingServer::~PlayingServer()
{

}

void PlayingServer::InitPlayingServer()
{

}

void PlayingServer::SendToOtherClients(ENetPeer *source, Packet *packet)
{
    for(auto&[client_peer, _] : client_infos_)
    {
        if(client_peer == source)
            continue;

        ENetPacket *en_packet = enet_packet_create(&packet, packet->header.size, ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(client_peer, 0,  en_packet);
    }
}

void PlayingServer::CreateNewClient(ENetPeer *source)
{
    ClientInfo info = {
            .u_id = std::to_string(source->address.host) + " : " + std::to_string(source->address.port),
            .ping = std::chrono::milliseconds::zero(),
            .last_alive = std::chrono::steady_clock::now()
    };
    client_infos_.emplace(source,  info);
    std::cout << info.u_id << std::endl;
}

void PlayingServer::ReadClientPacket(ENetPeer *source, ENetPacket *client_packet)
{
    Packet* pack = reinterpret_cast<Packet*>(client_packet->data);
    switch (pack->header.type) {
        case PacketType::Connect:
            CreateNewClient(source);
            break;
        case PacketType::Alive:
            UpdateClientPing(source);
            break;
        case PacketType::Dumb:
            SendToOtherClients(source, pack);
            break;
        default:
            break;
    }
}

void PlayingServer::UpdateClientPing(ENetPeer *source)
{
    client_infos_[source].ping = std::chrono::duration_cast<std::chrono::seconds>
            (std::chrono::steady_clock::now() -  client_infos_[source].last_alive);
    client_infos_[source].last_alive = std::chrono::steady_clock::now();

    std::string ping = client_infos_[source].u_id + " " + std::to_string(client_infos_[source].ping.count()) + "s";

    PacketHeader header = {
            .type = PacketType::Dumb,
            .size = sizeof(PacketHeader) + ping.size()
    };

    Packet packet = {
            .header = header
            };

    std::memmove(packet.data, ping.data(), ping.size());

    for(auto&[client_peer, _] : client_infos_)
    {
        if(client_peer == source)
            continue;

        ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(client_peer, 0,  en_packet);
    }
}

void PlayingServer::PingAllClients()
{
    if(client_infos_.empty())
        return;

    auto count = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - system_time_).count();
    if(count < 5)
        return;

    PacketHeader header = {
            .type = PacketType::Alive,
            .size = sizeof(PacketType)

    };
    Packet packet = {
        .header = header
    };

    for(auto&[client_peer, client_info] : client_infos_)
    {
        client_info.ping = std::chrono::duration_cast<std::chrono::seconds>
                (std::chrono::steady_clock::now() -  client_info.last_alive);
        client_info.last_alive = std::chrono::steady_clock::now();

        ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(client_peer, 0,  en_packet);
    }
}

void PlayingServer::SendSystemTime()
{
    if(client_infos_.empty())
        return;

    auto count = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - system_time_).count();
    if(count < 5)
        return;
    system_time_ = std::chrono::system_clock::now();

    std::string time = std::to_string( std::chrono::duration_cast<std::chrono::seconds>
                                        (std::chrono::system_clock::now() - begin_time_).count());

    PacketHeader header = {
            .type = PacketType::Dumb,
            .size = sizeof(PacketHeader) + time.size()
    };

    Packet packet = {
            .header = header
    };

    std::memmove(packet.data, time.data(), time.size());
    for(auto&[client_peer, client_info] : client_infos_)
    {
        ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(client_peer, 0,  en_packet);
    }
}

void PlayingServer::Run()
{
    while (is_running_)
    {
        PingAllClients();
        SendSystemTime();

        ENetEvent event;
        while (enet_host_service(server, &event, 10) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    CreateNewClient(event.peer);
                    enet_packet_destroy(event.packet);
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