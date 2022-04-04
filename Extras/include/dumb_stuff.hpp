#pragma once
#include <functional>
#include <unistd.h>

class Defer
{
    std::function<void()> f;

public:
    explicit  Defer(std::function<void()> def_ex) : f(def_ex) {};
    ~Defer() { f(); }
};


class File {
    constexpr static int FD_NONE = -1;

    int fd { FD_NONE };

public:
    File() = default;
    explicit File(int value) : fd(value) {};
    File(const File &) = delete;
    File &operator=(const File &) = delete;

    File(File &&other) : fd{ std::exchange(other.fd, FD_NONE) } {}

    File &operator=(File &&other)
    {
        if (this != &other)
        {
            std::swap(fd, other.fd);
        }

        return *this;
    }

    int FD() const { return fd; }
    bool valid() const { return fd != FD_NONE; }
    void release() { fd = FD_NONE; }

    ~File()
    {
        if (fd != FD_NONE)
            close(fd);
    }
};
