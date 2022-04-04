#include "Client.hpp"

int main(int argc, char *argv[])
{
    //int server_port = static_cast<int>(std::stoi(std::string(argv[1])));
    int server_port = 2022;
    Client* app = new Client(server_port);
    app->Run();

    return 1;
}
