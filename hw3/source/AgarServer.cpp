#include <AgarServer.hpp>
#include <Packet.hpp>
#include <cstring>

AgarServer::AgarServer(int port) :
                        Server(port, 32, 2),
                        game_(200, 200),
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
        default:
            break;
    }


}

void AgarServer::UpdateGameState()
{
    static float i = 0;
    last_frame = std::chrono::high_resolution_clock::now();

    //if(dt.count() < 1)
    //{
    //    dt += std::chrono::high_resolution_clock::now() - last_frame;
    //    return;
    //}
    game_.Step(dt.count());

    dt = std::chrono::high_resolution_clock::now() - last_frame;
}

void AgarServer::KillPlayer(ENetPeer *new_peer)
{

}

void AgarServer::Run()
{
    std::cout << "1" << std::endl;
    while (is_running_)
    {
        ENetEvent event;
        while (enet_host_service(server, &event, 10) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "3" << std::endl;
                    client_peers_.push_back(event.peer);
                    AddNewPlayer(event.peer);
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    std::cout << "4" << std::endl;
                    ReadClientPacket(event.peer, event.packet);
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    KillPlayer(event.peer);
                    break;

                default:
                    std::cout << "5" << std::endl;
                    break;
            };
        }
        UpdateGameState();
        SendGameState();
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