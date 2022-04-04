#pragma once

#include <Packet/Packet.hpp>
#include <dumb_stuff.hpp>

#include <string>
#include <array>
#include <deque>

class Client {
public:
    explicit Client(int server_port);
    ~Client();
    void Run();

private:
    void InitClient();
    void InitSocket();
    void ScheduleInitialEvents();

    void SchedulePacketSend();
    void SendEmptyMessage(PacketType type);

    void InputClientMessage();
    void SendClientMessage();

    void ReceiveServerMessages();
    void DisplayServerMessages();

    bool is_running_;
    const int server_port_;

    File input_;
    File output_;
    File server_socket_;

    File live_timer_;
    File epoll_;

    std::deque<std::string> incoming_messages {};
    std::deque<std::array<std::byte, PACKET_SIZE>> outgoing_packets {};
};
