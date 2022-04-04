#pragma once

#include <dumb_stuff.hpp>

extern "C"
{
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
}
struct addrinfo;

namespace muhsockets
{
    int epoll_ctl(const File &epoll, const File &file, int op, uint32_t events);

    namespace tools {
        File create_dgram_socket(const char *dst_addr, const char *dst_port);

        File pick_addrinfo_and_create_socket(addrinfo* list, bool client);

    }// namespace tools
} // namespace muhsockets


File make_timer(time_t interval_sec);