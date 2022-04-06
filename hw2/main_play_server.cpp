#include <PlayingServer.hpp>

int main(int argc, char *argv[])
{
    PlayingServer* play_server = new PlayingServer(2022);
    play_server->Run();

    return 1;
}
