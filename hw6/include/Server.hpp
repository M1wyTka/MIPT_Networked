#pragma once
#include <enet/enet.h>
#include <iostream>
#include <deque>
#include <vector>

class Server
{
public:
    Server(int port, int peer_count, int channel_limit) :
                            server_port_(port),
                            is_running_(true)
    {
        InitEnet();
        InitHostAndAddress(peer_count, channel_limit);
        client_peers_.reserve(16);
    }
    ~Server()
    {
        enet_host_destroy(server);
        atexit(enet_deinitialize);
    };

    virtual void Run() {};
protected:
    void InitEnet()
    {
        if (enet_initialize() != 0)
        {
            std::cout << "Cannot init ENet" << std::endl;
        }
    };

    void InitHostAndAddress(int peer_count, int channel_count)
    {
        address.host = ENET_HOST_ANY;
        address.port = server_port_;

        server = enet_host_create(&address, peer_count, channel_count, 0, 0);

        if (!server)
        {
            std::cout <<"Cannot create ENet server" << std::endl;
        }
    };

    ENetHost *server;
    ENetAddress address;

    std::vector<ENetPeer*> client_peers_{};

    bool is_running_;
    int server_port_;
};