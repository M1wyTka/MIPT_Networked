#pragma once

#include <array>
#include <unordered_map>
#include <deque>
#include <cstdint>
#include <string>

#include <utility>
#include <chrono>

extern "C"
{
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/socket.h>
}

#include <Packet/Packet.hpp>
#include <SocketTools.hpp>



class EchoServer {
private:
    struct Client
            {
                struct sockaddr_storage addr;
                socklen_t len = sizeof(addr);
                std::deque<std::array<char, PACKET_SIZE>> outgoing;
                std::chrono::steady_clock::time_point last_alive;
            };

public:
    explicit EchoServer(int port);
    ~EchoServer();

    void Run();

private:
    void InitServer();
    void InitSocket();
    void ScheduleInitialEvents();

    void SendAllNewMessages();
    bool SendOutgoing(Client& client);
    void KillAllSilentClients();

    void ReadReceivedPackets();
    std::string GetClientPacketId(struct sockaddr_storage* addr);
    void RegisterNewClient(struct sockaddr_storage* addr, std::string identifier);

    void UpdateOutgoingMessages(Packet* packet, std::string identifier);

    const int port_;
    File kill_timer_;
    File listener_;
    File epoll_;

    bool new_messages;
    bool is_running_;
    std::unordered_map<std::string, Client> clients_;
};

