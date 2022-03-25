#pragma once

#include <cstdint>
#include <string>
#include <sys/socket.h>

#include <SocketTools.hpp>
#include <MessageTypes.hpp>

class EchoServer {
public:
    explicit EchoServer(int port);
    ~EchoServer();

    void Run();

private:
    void InitServer(int port);
    void ReceiveMessages();

    void ParseMessage(char* buffer);
    std::string ParseMessageType(const std::string& message, MessageType& type);

    void ReadDumbMessage(const std::string& sub_message);

    void SendMessage();
    void ReplyMessage();


    std::string port_;
    bool is_running_;
    int sfd_;

    static constexpr char delimiter_ = ',';
    static constexpr int buffer_size_ = 1000;
};

