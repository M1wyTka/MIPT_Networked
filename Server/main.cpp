#include "EchoServer.hpp"

int main()
{
    EchoServer* server = new EchoServer(2022);
    server->Run();

    return 1;
}