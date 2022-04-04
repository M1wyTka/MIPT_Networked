#include <SocketTools.hpp>
#include <cstdlib>
#include <cassert>

namespace muhsockets {

    int epoll_ctl(const File &epoll, const File &file, int op, uint32_t events)
    {
        struct epoll_event ev{
            .events = events,
            .data = {
                    .fd = file.FD(),
            },
    };
    return epoll_ctl(epoll.FD(), op, file.FD(), &ev);
    }

    namespace tools {
        File create_dgram_socket(
                const char *dst_addr,
                const char *dst_port)
        {
            addrinfo hints{
                    .ai_flags = dst_addr == nullptr ? AI_PASSIVE : 0,
                    .ai_family = AF_INET,
                    .ai_socktype = SOCK_DGRAM,
                    .ai_protocol = IPPROTO_UDP,
                    .ai_addrlen = 0,
                    .ai_addr = nullptr,
                    .ai_canonname = nullptr,
                    .ai_next = nullptr,
            };

            addrinfo *list = nullptr;
            assert(("Error getting addrinfo", getaddrinfo(dst_addr, dst_port, &hints, &list) != -1));
            Defer freelist{[&](){ freeaddrinfo(list); }};

            File result = pick_addrinfo_and_create_socket(list, dst_addr != nullptr);
            assert(("Addrinfo pick failed", result.valid()));

            assert(("Nonblock setup on socket failed", fcntl(result.FD(), F_SETFL, O_NONBLOCK) != -1));

            int trueVal = 1;
            setsockopt(result.FD(), SOL_SOCKET, SO_REUSEADDR, &trueVal, sizeof(int));

            return result;
        }


        File pick_addrinfo_and_create_socket(addrinfo* list, bool client)
        {
            for (auto info = list; info != nullptr; info = info->ai_next)
            {
                File result{socket(info->ai_family, info->ai_socktype, info->ai_protocol)};
                if (!result.valid()) continue;

                if (client)
                {
                    if (connect(result.FD(), info->ai_addr, info->ai_addrlen) == -1)
                    {
                        continue;
                    }

                    return result;
                }
                else
                {
                    // Receive packets from our chosen address
                    if (bind(result.FD(), info->ai_addr, info->ai_addrlen) == -1)
                    {
                        continue;
                    }

                    // Send to "anywhere"
                    return result;
                }
            }

            return File{};
        }

    }
}



File make_timer(time_t interval_sec)
{
    File timer{timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)};

    if (!timer.valid()) return timer;

    struct itimerspec spec{
            .it_interval = {
                    .tv_sec = interval_sec,
                    .tv_nsec = 0,
            },
            .it_value = {
                    .tv_sec = interval_sec,
                    .tv_nsec = 0,
            },
    };

    if (timerfd_settime(timer.FD(), 0, &spec, nullptr) == -1)
    {
        return {};
    }

    return timer;
}
