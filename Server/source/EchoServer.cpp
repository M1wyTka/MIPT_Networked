#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <cstring>
#include <cstdio>
#include <iostream>

#include <EchoServer.hpp>

void EchoServer::InitServer(int port)
{
    port_ = std::to_string(port);
    sfd_ = muhsockets::tools::create_dgram_socket(nullptr, port_.c_str(), nullptr);;
    is_running_ = !(sfd_ == -1);
}

EchoServer::EchoServer(int port)
{
    InitServer(std::forward<int>(port));
}

EchoServer::~EchoServer()
{

}

void EchoServer::ReceiveMessages()
{
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sfd_, &readSet);

    timeval timeout = { 0, 100000 }; // 100 ms
    select(sfd_ + 1, &readSet, NULL, NULL, &timeout);


    if (FD_ISSET(sfd_, &readSet))
    {
        static char buffer_[buffer_size_];
        memset(buffer_, 0, buffer_size_);

        ssize_t numBytes = recvfrom(sfd_, buffer_, buffer_size_ - 1, 0, nullptr, nullptr);
        is_running_ = (numBytes > 0);
        if (is_running_)
        {
            ParseMessage(buffer_);
        }
    }
}

void EchoServer::ReadDumbMessage(const std::string &sub_message)
{
    size_t len_idx = sub_message.find(delimiter_, 0);
    size_t msgLen = static_cast<size_t>(std::stoi(sub_message.substr(0, len_idx)));
    std::cout << sub_message.substr(len_idx+1, msgLen) << std::endl; // assume that buffer is a string
}

std::string EchoServer::ParseMessageType(const std::string& message, MessageType& type)
{
    size_t type_idx = message.find(delimiter_, 0);
    type = static_cast<MessageType>(std::stoi(message.substr(0, type_idx)));
    return message.substr(type_idx+1);
}

void EchoServer::ParseMessage(char* buffer)
{
    std::string message(buffer);
    MessageType type;
    std::string sub_message = ParseMessageType(message, type);
    switch (type) {
        case MessageType::DumbMessage:
            ReadDumbMessage(sub_message);
            break;
        default:
            break;
    }

}

void EchoServer::Run()
{
    while (is_running_)
    {
        ReceiveMessages();
    }
}

void EchoServer::ReplyMessage()
{

}

void EchoServer::SendMessage()
{


}



