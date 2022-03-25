#include <Client.hpp>
#include <MessageTypes.hpp>

#include <iostream>
#include <cstring>

Client::Client(int port) : is_running_(true)
{
    TryToConnect(std::forward<int>(port));
}

Client::~Client()
{

}

std::string Client::FormDumbMessage()
{
    std::string message;
    message += std::to_string(static_cast<int>(MessageType::DumbMessage)) + ",";

    std::string input;
    printf(">");
    std::getline(std::cin, input);

    message += std::to_string(input.size()) + "," + input;
    return message;
}

void Client::SendMessage()
{
    std::string message = FormDumbMessage();
    ssize_t res = sendto(sfd_, message.c_str(), message.size(), 0, resAddrInfo_.ai_addr, resAddrInfo_.ai_addrlen);
    if (res == -1)
    {
        std::cout << std::strerror(errno) << std::endl;
        is_running_ = false;
    }
}

void Client::Run()
{
    if(!is_running_)
    {
        std::cout << "Connection failed. Stopping." << std::endl;
        return;
    }

    std::cout << "Client running..." << std::endl;
    while(is_running_)
    {
        SendMessage();
    }
    std::cout << "Client stopped." << std::endl;
}

void Client::TryToConnect(int port)
{
    port_ = std::to_string(port);
    sfd_ = muhsockets::tools::create_dgram_socket("localhost", port_.c_str(), &resAddrInfo_);
    is_running_ = (sfd_ != -1);
}