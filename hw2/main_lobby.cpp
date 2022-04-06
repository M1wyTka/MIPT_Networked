#include <LobbyServer.hpp>

int main(int argc, char *argv[])
{
    LobbyServer* lobby = new LobbyServer(2021, 2022);
    lobby->Run();

    return 1;
}

