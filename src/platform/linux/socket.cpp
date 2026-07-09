#ifndef _WIN32

#include <socket.hpp>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace appweeb
{
    class Socket::Impl
    {
    public:

        int handle = -1;
    };

    Socket::Socket()
        : m_impl(std::make_unique<Impl>())
    {
    }

    Socket::Socket(
        void* nativeHandle)
        : m_impl(std::make_unique<Impl>())
    {
        m_impl->handle =
            static_cast<int>(
                reinterpret_cast<intptr_t>(
                    nativeHandle));
    }

    Socket::~Socket()
    {
        Close();
    }

    Socket::Socket(
        Socket&& other) noexcept
        : m_impl(std::move(other.m_impl))
    {
    }

    Socket& Socket::operator=(
        Socket&& other) noexcept
    {
        if (this != &other)
        {
            Close();
            m_impl = std::move(other.m_impl);
        }

        return *this;
    }

    bool Socket::Listen(
        uint16_t port)
    {
        m_impl->handle =
            socket(
                AF_INET,
                SOCK_STREAM,
                0);

        if (m_impl->handle < 0)
        {
            return false;
        }

        int reuse = 1;

        setsockopt(
            m_impl->handle,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuse,
            sizeof(reuse));

        sockaddr_in address {};

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port);

        if (
            bind(
                m_impl->handle,
                reinterpret_cast<sockaddr*>(&address),
                sizeof(address)) < 0)
        {
            std::cout
                << "Bind failed: "
                << std::strerror(errno)
                << std::endl;

            Close();

            return false;
        }

        if (
            listen(
                m_impl->handle,
                SOMAXCONN) < 0)
        {
            Close();

            return false;
        }

        return true;
    }
    
Socket Socket::Accept()
{
    int client =
        accept(
            m_impl->handle,
            nullptr,
            nullptr);

    std::cout
        << "Accepted socket "
        << client
        << '\n';

    int flags =
        fcntl(client, F_GETFL, 0);

    std::cout
        << "Flags: "
        << flags
        << '\n';

    return Socket(
        reinterpret_cast<void*>(
            static_cast<intptr_t>(client)));
}

    int Socket::Receive(
        void* buffer,
        int size)
    {
        int result =
            static_cast<int>(
                recv(
                    m_impl->handle,
                    buffer,
                    static_cast<size_t>(size),
                    0));

        if (result < 0)
        {
            std::cout
                << "recv failed: "
                << errno
                << " "
                << std::strerror(errno)
                << '\n';
        }

        return result;
    }

    int Socket::Send(
        const void* buffer,
        int size)
    {
        return static_cast<int>(
            send(
                m_impl->handle,
                buffer,
                static_cast<size_t>(size),
                MSG_NOSIGNAL));
    }

    void Socket::Close()
    {
        if (!m_impl)
        {
            return;
        }

        if (m_impl->handle >= 0)
        {
            close(
                m_impl->handle);

            m_impl->handle = -1;
        }
    }

    bool Socket::IsValid() const
    {
        return
            m_impl &&
            m_impl->handle >= 0;
    }
}
#endif