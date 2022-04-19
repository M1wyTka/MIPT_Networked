#include <AgarServer.hpp>
#include <Packet.hpp>
#include <cstring>

AgarServer::AgarServer(int port) :
                        Server(port, 32, 2),
                        game_(640, 480),
                        dt(std::chrono::nanoseconds::zero()),
                        last_frame(std::chrono::high_resolution_clock::now())
{
    InitAgarServer();
}

AgarServer::~AgarServer()
{

}

void AgarServer::InitAgarServer()
{

}

void AgarServer::CreateNewClient(ENetPeer *new_peer)
{
    ClientInfo info = {
            .u_id = std::to_string(new_peer->address.host) + " : " + std::to_string(new_peer->address.port),
            .ping = std::chrono::milliseconds::zero(),
            .last_alive = std::chrono::steady_clock::now()
    };
    client_infos_.emplace(new_peer,  info);
    std::cout << info.u_id << std::endl;

    AddNewPlayer(new_peer);
}

void AgarServer::AddNewPlayer(ENetPeer* new_player)
{
    std::string uid = game_.CreatePlayer();
    UID_by_peer_.emplace(new_player, uid);

    SendUIDBack(new_player, uid);
}

void AgarServer::SendUIDBack(ENetPeer* back_peer, std::string uid)
{
    PacketHeader header = {
            .type = PacketType::GameStart,
            .size = sizeof(PacketHeader) + uid.size()
    };

    Packet packet = {
            .header = header
    };

    std::memmove(packet.data, uid.data(), uid.size());

    ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(back_peer, 0,  en_packet);
}

void AgarServer::SendGameState()
{
    auto state = game_.GetGameState(); // Non-trivially copyable
    std::cout << state.size() << std::endl;
    if(state.empty())
        return;

    int size = sizeof(PacketHeader) + sizeof(state[0])*state.size();
    std::vector<std::byte> castille{};
    castille.resize(size);

    PacketHeader header = {
            .type = PacketType::GameState,
            .size = sizeof(PacketHeader) + sizeof(state[0])*state.size()
    };

    Packet packet = {
            .header = header
    };
    std::memmove(castille.data(), &packet, sizeof(PacketHeader));
    std::memmove(castille.data()+sizeof(PacketHeader), state.data(), sizeof(state[0])*state.size());


    for(auto& peer : client_peers_)
    {
        ENetPacket *en_packet = enet_packet_create(castille.data(), packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(peer, 0,  en_packet);
    }
}

void AgarServer::ReadClientPacket(ENetPeer* peer, ENetPacket* packet)
{
    Packet* pack = reinterpret_cast<Packet*>(packet->data);
    switch(pack->header.type)
    {
        case PacketType::Input:
            game_.SetPlayerInputs(UID_by_peer_[peer], *reinterpret_cast<Vec2 *>(pack->data));
            break;
        case PacketType::Alive:
            UpdateClientPing(peer);
            break;
        default:
            break;
    }
}

void AgarServer::UpdateClientPing(ENetPeer *source)
{
    client_infos_[source].ping = std::chrono::duration_cast<std::chrono::nanoseconds>
            (std::chrono::steady_clock::now() -  client_infos_[source].last_alive);

    PacketHeader header = {
            .type = PacketType::Ping,
            .size = sizeof(PacketHeader) + sizeof(float)
    };

    Packet packet = {
            .header = header
    };

    float ping = client_infos_[source].ping.count();
    std::memmove(packet.data, &ping, sizeof(float));

    ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(source, 0,  en_packet);
}

void AgarServer::UpdateGameState()
{
    static float i = 0;
    last_frame = std::chrono::high_resolution_clock::now();
    game_.Step(dt.count());

    dt = std::chrono::high_resolution_clock::now() - last_frame;
}

void AgarServer::PingAll()
{
    if(client_peers_.empty())
        return;

    PacketHeader header = {
            .type = PacketType::Alive,
            .size = sizeof(PacketType)

    };
    Packet packet = {
            .header = header
    };

    for(auto& [client_peer, info] : client_infos_)
    {
        info.ping = std::chrono::duration_cast<std::chrono::nanoseconds>
                (std::chrono::steady_clock::now() -  info.last_alive);
        info.last_alive = std::chrono::steady_clock::now();

        ENetPacket *en_packet = enet_packet_create(&packet, packet.header.size, ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(client_peer, 0,  en_packet);
    }
}

void AgarServer::KillPlayer(ENetPeer *new_peer)
{
    game_.KillPlayer(UID_by_peer_[new_peer]);
}

void AgarServer::Run()
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
                    CreateNewClient(event.peer);
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    ReadClientPacket(event.peer, event.packet);
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    KillPlayer(event.peer);
                    break;

                default:
                    break;
            };
        }

        UpdateGameState();
        auto count = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - system_time_).count();
        if(count < 2)
            continue;
        PingAll();
        SendGameState();
        system_time_ = std::chrono::system_clock::now();
    }
    /*
    while(true)
    {
        ReadPackets();
        UpdateGameState();
        SendGameStateBack();
    }
    */
}