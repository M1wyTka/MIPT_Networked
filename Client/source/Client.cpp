#include <Client.hpp>
#include <SocketTools.hpp>

extern "C"
{
#include <sys/epoll.h>
}

#include <iostream>
#include <cstring>
#include <cassert>
#include <array>

Client::Client(int server_port) :
                server_port_(server_port),
                is_running_(true)
{
    InitClient();

    if(!is_running_)
    {
        std::cout << "Initialize failed" << std::endl;
        return;
    }

}

Client::~Client()
{
    input_.release();
    output_.release();
}

void Client::InitClient()
{
    input_ = File{0};
    output_ =  File{1};

    InitSocket();
    ScheduleInitialEvents();

    SendEmptyMessage(PacketType::Connect);
}

void Client::ScheduleInitialEvents()
{
    epoll_ = File{epoll_create1(0)};
    is_running_ = epoll_.valid();

    assert(("Epoll is not valid", is_running_));

    is_running_ = muhsockets::epoll_ctl(epoll_, live_timer_, EPOLL_CTL_ADD, EPOLLIN) != -1;
    assert(("Timer schedule failed", is_running_));

    is_running_ = muhsockets::epoll_ctl(epoll_, server_socket_, EPOLL_CTL_ADD, EPOLLIN) != -1;
    assert(("Socket reading schedule failed", is_running_));

    is_running_ = muhsockets::epoll_ctl(epoll_, input_,EPOLL_CTL_ADD, EPOLLIN) != -1;
    assert(("Input reading schedule failed", is_running_));
}

void Client::InitSocket()
{
    live_timer_ = make_timer(1);

    is_running_= live_timer_.valid();
    assert(("Timer not valid", is_running_));

    server_socket_ = muhsockets::tools::create_dgram_socket("localhost", std::to_string(server_port_).c_str());

    is_running_ = server_socket_.valid();
    assert(("Socket not valid", is_running_));
}

void Client::SendEmptyMessage(PacketType type)
{
    uint64_t count;
    read(live_timer_.FD(), &count, sizeof(count));
    SchedulePacketSend();
    PacketHeader head = {
            .type = type,
            .size = sizeof(PacketHeader)
    };
    Packet pack = {
            .header = head
    };

    std::array<std::byte, PACKET_SIZE>* entry = reinterpret_cast<std::array<std::byte, PACKET_SIZE>*>(&pack);
    outgoing_packets.push_back(*entry);
}

void Client::SchedulePacketSend()
{
    if (!outgoing_packets.empty())
        return;

    is_running_ = muhsockets::epoll_ctl(epoll_, server_socket_, EPOLL_CTL_MOD, EPOLLIN | EPOLLOUT) != -1;
    assert(("Packet send schedule failed", is_running_));
}

void Client::InputClientMessage()
{
    // Data came from stdin
    std::array<std::byte, PACKET_SIZE> buffer;
    const size_t maxsize = PACKET_SIZE - sizeof(PacketHeader);
    ssize_t readed = read(input_.FD(), buffer.data() + sizeof(PacketHeader), maxsize);

    auto packet = reinterpret_cast<Packet*>(buffer.data());
    packet->header.type = PacketType::Dumb;
    packet->header.size = readed + sizeof(PacketHeader);

    SchedulePacketSend();

    outgoing_packets.push_back(std::move(buffer));
}

void Client::SendClientMessage()
{
    // We are ready to send packets
    while (!outgoing_packets.empty())
    {
        auto& raw = outgoing_packets.front();
        auto* packet = reinterpret_cast<Packet*>(raw.data());
        auto res =
                send(server_socket_.FD(), raw.data(), packet->header.size, 0);

        if (res != packet->header.size) break;

        outgoing_packets.pop_front();
    }

    if (outgoing_packets.empty())
    {
        is_running_ = muhsockets::epoll_ctl(epoll_, server_socket_, EPOLL_CTL_MOD, EPOLLIN) != -1;
        assert(("Receiving schedule failed", is_running_));
    }
}

void Client::DisplayServerMessages()
{
    // We are ready to write to stdout
    while (!incoming_messages.empty())
    {
        auto& str = incoming_messages.front();
        auto written = write(output_.FD(), str.data(), str.size());
        //VERIFY(written >= 0);
        if (static_cast<size_t>(written) < str.size())
        {
            str = str.substr(written);
            break;
        }
        incoming_messages.pop_front();
    }

    if (incoming_messages.empty())
    {
        is_running_ = muhsockets::epoll_ctl(epoll_, output_, EPOLL_CTL_DEL, {}) != -1;
        assert(("Output deshedule failed", is_running_));
    }
}

void Client::ReceiveServerMessages()
{
    // Packets were received
    std::array<std::byte, PACKET_SIZE> buffer;
    ssize_t packet_size =
            recv(server_socket_.FD(), buffer.data(), buffer.size(), 0);

    is_running_ = packet_size > 0;
    assert(("Error on receiving", is_running_));

    packet_size -= sizeof(PacketHeader);
    Packet* packet = reinterpret_cast<Packet*>(buffer.data());

    if (packet->header.type != PacketType::Dumb)
        return;

    if (incoming_messages.empty())
    {
        is_running_ = muhsockets::epoll_ctl(epoll_, output_, EPOLL_CTL_ADD, EPOLLOUT) != -1;
        assert(("Message display schedule failed", is_running_));
    }

    incoming_messages.emplace_back(
            reinterpret_cast<char*>(packet->data), packet_size);
}

void Client::Run()
{
    std::array<struct epoll_event, 16> events;
    while(is_running_)
    {
        auto ev_count = epoll_wait(epoll_.FD(), events.data(), events.size(), -1);

        for (int i = 0; i < ev_count; ++i)
        {
            auto& event = events[i];

            if (event.data.fd == live_timer_.FD())
            {
                SendEmptyMessage(PacketType::Alive);
            }
            else if (event.data.fd == 0)
            {
                InputClientMessage();
            }
            else if (event.data.fd == 1)
            {
                DisplayServerMessages();
            }
            else if (event.events & EPOLLOUT)
            {
                SendClientMessage();
            }
            else if (event.events & EPOLLIN)
            {
                ReceiveServerMessages();
            }
        }
    }
}
