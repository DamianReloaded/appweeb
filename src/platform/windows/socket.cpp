#include "../../socket.hpp"
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

namespace appweeb
{
    class Socket::Impl
    {
    public:

        SOCKET handle = INVALID_SOCKET;
    };

    struct WinsockInitializer
    {
        WinsockInitializer()
        {
            WSADATA data;

            WSAStartup(
                MAKEWORD(2, 2),
                &data);
        }

        ~WinsockInitializer()
        {
            WSACleanup();
        }
    };

    WinsockInitializer g_winsock;

    Socket::Socket()
        : m_impl(std::make_unique<Impl>())
    {
    }

    Socket::Socket(
        void* nativeHandle)
        : m_impl(std::make_unique<Impl>())
    {
        m_impl->handle =
            static_cast<SOCKET>(
                reinterpret_cast<uintptr_t>(
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
                IPPROTO_TCP);

        if (m_impl->handle == INVALID_SOCKET)
        {
            return false;
        }

        sockaddr_in address {};

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(port);

        if (
            bind(
                m_impl->handle,
                reinterpret_cast<sockaddr*>(&address),
                sizeof(address)) == SOCKET_ERROR)
        {
        std::cout
            << "Bind failed: "
            << WSAGetLastError()
            << " - Already running? Check the task manager."
            << std::endl;		
            
            Close();
            return false;
        }

        if (
            listen(
                m_impl->handle,
                SOMAXCONN) == SOCKET_ERROR)
        {
            Close();
            return false;
        }

        return true;
    }

    Socket Socket::Accept()
    {
        SOCKET client =
            accept(
                m_impl->handle,
                nullptr,
                nullptr);

        return Socket(
            reinterpret_cast<void*>(
                static_cast<uintptr_t>(
                    client)));
    }

    int Socket::Receive(
        void* buffer,
        int size)
    {
        return recv(
            m_impl->handle,
            static_cast<char*>(buffer),
            size,
            0);
    }

    int Socket::Send(
        const void* buffer,
        int size)
    {
        return send(
            m_impl->handle,
            static_cast<const char*>(buffer),
            size,
            0);
    }

    void Socket::Close()
    {
        if (!m_impl)
        {
            return;
        }

        if (m_impl->handle != INVALID_SOCKET)
        {
            closesocket(
                m_impl->handle);

            m_impl->handle =
                INVALID_SOCKET;
        }
    }

    bool Socket::IsValid() const
    {
        return
            m_impl &&
            m_impl->handle != INVALID_SOCKET;
    }
}