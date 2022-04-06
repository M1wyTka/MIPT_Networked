#include <LobbyClient.hpp>

int main(int argc, char *argv[])
{
    LobbyClient* client = new LobbyClient(2021);
    client->Run();

    return 1;
}
