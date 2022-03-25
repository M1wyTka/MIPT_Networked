#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

struct addrinfo;

namespace muhsockets
{
namespace tools {
    int create_dgram_socket(const char *address, const char *port, addrinfo *res_addr);

} // namespace tools
} // namespace muhsockets