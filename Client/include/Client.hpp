#pragma once

#include "SocketTools.hpp"
#include <string>

class Client {
public:
    explicit Client(int port);
    ~Client();
    void Run();


private:


    void TryToConnect(int port);
    void SendMessage();

    std::string FormDumbMessage();

    uint32_t is_running_;
    std::string port_;
    int sfd_;

    addrinfo resAddrInfo_;
};
