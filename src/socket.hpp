#pragma once

#include <cstdint>
#include <memory>

namespace appweeb
{
    class Socket
    {
    public:

        Socket();

        ~Socket();

        Socket(Socket&& other) noexcept;

        Socket& operator=(Socket&& other) noexcept;

        Socket(const Socket&) = delete;

        Socket& operator=(const Socket&) = delete;

        bool Listen(uint16_t port);

        Socket Accept();

        int Receive(
            void* buffer,
            int size);

        int Send(
            const void* buffer,
            int size);

        void Close();

        bool IsValid() const;

    private:

        class Impl;

        explicit Socket(
            void* nativeHandle);

    private:

        std::unique_ptr<Impl> m_impl;
    };
}