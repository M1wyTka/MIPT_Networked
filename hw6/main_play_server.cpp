#include <AgarServer.hpp>
#include <cstdlib>

int main(int argc, char *argv[])
{
    int x = atoi(argv[1]);

    AgarServer play_server(2022+x);
    play_server.Run();

    return 1;
}
