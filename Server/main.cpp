#include "EchoServer.hpp"

int main(int argc, char *argv[])
{
    //int server_port = static_cast<int>(std::stoi(std::string(argv[1])));
    int server_port = 2022;
    EchoServer* server = new EchoServer(server_port);
    server->Run();

    return 1;
}