#include <assert.h>
#include <iostream>
#include <sys/types.h>
#include <EchoServer.hpp>
#include <cstring>


EchoServer::EchoServer(int port)  :
                    port_(port),
                    new_messages(false),
                    is_running_(true)
{
    InitServer();

    if(!is_running_)
    {
        std::cout << "Initialization failed" << std::endl;
    }
    else
        std::cout << "Server initialization successful" << std::endl;
}

EchoServer::~EchoServer()
{

}

void EchoServer::InitServer()
{
    std::cout << "Initializing server..." << std::endl;
    InitSocket();
    ScheduleInitialEvents();
}

void EchoServer::InitSocket()
{
    listener_ = muhsockets::tools::create_dgram_socket(nullptr, std::to_string(port_).c_str());
    is_running_ = listener_.valid();
    assert(("Socket not valid", is_running_));

    kill_timer_ = make_timer(5);
    is_running_ = kill_timer_.valid();
    assert(("Timer not valid", is_running_));

    epoll_ = File{epoll_create1(0)};
    is_running_ = epoll_.valid();
    assert(("Epoll is not valid", is_running_));
}

void EchoServer::ScheduleInitialEvents()
{
    is_running_ = muhsockets::epoll_ctl(epoll_, kill_timer_, EPOLL_CTL_ADD, EPOLLIN) != -1;
    assert(("Kill timer schedule failed", is_running_));

    is_running_ = muhsockets::epoll_ctl(epoll_, listener_, EPOLL_CTL_ADD, EPOLLIN) != -1;
    assert(("Socket listening schedule failed", is_running_));
}

void EchoServer::KillAllSilentClients()
{
    uint64_t count;
    is_running_ = read(kill_timer_.FD(), &count, sizeof(count)) != -1;
    assert(("Read failed", is_running_));

    auto time =  std::chrono::steady_clock::now();
    for (auto it = clients_.begin(); it != clients_.end();)
    {
        if (std::chrono::duration_cast<std::chrono::seconds>(time - it->second.last_alive).count() > 5)
        {
            std::cout << it->first << " timed out!" << std::endl;
            it = clients_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void EchoServer::SendAllNewMessages()
{
    new_messages = false;
    for (auto&[_, client] : clients_)
    {
        while (!client.outgoing.empty())
        {
            if(!SendOutgoing(client))
                break;
        }
        if (!is_running_)
            break;
    }

    if (is_running_)
    {
        is_running_ = muhsockets::epoll_ctl(epoll_, listener_, EPOLL_CTL_MOD, EPOLLIN) != -1;
        assert(("Packet receive schedule failed", is_running_));
    }
}

bool EchoServer::SendOutgoing(Client& client)
{
    auto& buf = client.outgoing.front();
    Packet* packet = reinterpret_cast<Packet*>(buf.data());
    uint32_t sent = sendto(listener_.FD(), buf.data(), packet->header.size, 0,
                           reinterpret_cast<struct sockaddr*>(&client.addr), client.len);
    if (sent != packet->header.size)
    {
        is_running_ = false;
        return false;
    }
    client.outgoing.pop_front();
    return true;
}

void EchoServer::ReadReceivedPackets()
{
    // For some reason any manipulation of this part breaks sockets
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    std::array<char, PACKET_SIZE> buf;
    ssize_t recvd = recvfrom(listener_.FD(), buf.data(), buf.size(), 0,
                             reinterpret_cast<struct sockaddr*>(&addr), &len);
    is_running_ = recvd > 0;
    assert(("Packet receive failed", is_running_));

    auto* packet = reinterpret_cast<Packet*>(buf.data());
    std::string identifier = GetClientPacketId(&addr);

    if (packet->header.type == PacketType::Connect)
    {
        RegisterNewClient(&addr, identifier);
        return;
    }

    if (!clients_.contains(identifier))
    {
        std::cout << identifier << " -- unknown client_!";
        return;
    }

    if (packet->header.type == PacketType::Alive)
    {
        clients_.at(identifier).last_alive = std::chrono::steady_clock::now();
    }
    else if (packet->header.type == PacketType::Dumb)
    {
        UpdateOutgoingMessages(packet, identifier);

        if(!new_messages)
        {
            new_messages = true;
            is_running_ = muhsockets::epoll_ctl(
                    epoll_, listener_, EPOLL_CTL_MOD,EPOLLIN | EPOLLOUT) != -1;
            assert(("New packets read schedule failed", is_running_));
        }
    }
}

void EchoServer::UpdateOutgoingMessages(Packet* packet, std::string identifier)
{
    size_t msg_len = packet->header.size - sizeof(PacketHeader);
    std::string msg(reinterpret_cast<char*>(packet->data),msg_len);

    static const std::string delim = " : ";
    std::string prefix = identifier + delim;

    msg = prefix + msg;
    packet->header.size += prefix.size();
    msg_len += prefix.size();

    unsigned char* msg_start = reinterpret_cast<unsigned char*>(packet->data);
    std::memmove(msg_start, msg.data(),  msg_len);

    std::cout << msg << std::endl;

    auto* prep_pack = reinterpret_cast<std::array<char, PACKET_SIZE>*>(packet);
    for (auto&[id, client] : clients_)
    {
        if (identifier == id)
            continue;

        client.outgoing.push_back(*prep_pack);
    }
}

std::string EchoServer::GetClientPacketId(struct sockaddr_storage *addr)
{
    socklen_t len = sizeof(sockaddr_storage);

    std::array<char, NI_MAXHOST> host;
    std::array<char, NI_MAXSERV> serv;
    auto sock_inf = getnameinfo(reinterpret_cast<struct sockaddr*>(addr), len,
            host.data(), host.size(), serv.data(), serv.size(), 0);

    is_running_ = (sock_inf != -1);
    assert(("Get socket info failed", is_running_));

    std::string identifier = std::string(host.data()) + ":" + std::string(serv.data());
    return identifier;
}

void EchoServer::RegisterNewClient(struct sockaddr_storage *addr, std::string identifier)
{
    std::cout << "New client_ " << identifier << " registered." << std::endl;
    socklen_t len = sizeof(sockaddr_storage);

    if (!clients_.contains(identifier))
    {
        Client new_client = Client{
                .addr = *addr,
                .len = len,
                .outgoing = {},
                .last_alive = std::chrono::steady_clock::now(),
        };
        clients_.insert(std::make_pair(identifier, std::move(new_client)));
    }
}

void EchoServer::Run()
{
    std::array<struct epoll_event, 16> events;
    while (is_running_)
    {
        auto ev_count = epoll_wait(epoll_.FD(), events.data(), events.size(), -1);

        is_running_ = ev_count != -1;
        assert(("Event wait failed", is_running_));

        for (int i = 0; i < ev_count; ++i)
        {
            auto &event = events[i];

            if (event.data.fd == kill_timer_.FD())
            {
                KillAllSilentClients();
            }
            else if (event.events & EPOLLIN)
            {
                ReadReceivedPackets();
            }
            else if (event.events & EPOLLOUT)
            {
                SendAllNewMessages();
            }
        }
    }
}



