#pragma once
#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "../common/DataStructs/DataStructs.h"
#include "MiscFuncs.h"

using namespace std;
using namespace boost::asio;
using ip::tcp;

// This interface is so the session's async hook for receiving data, can call back the Server class.
// The server implements this class, and passes the session this interface in order to call it back.
// The server must implement each of these pure virtual methods!
interface IReceiveCallback
{
    void virtual OnData(string& szMesg, tcp::socket& socket) = 0;
    void virtual OnSocketClose(tcp::socket& socket) = 0;
};

// a session is a connected socket. 
//
// this was created as shared ptr and we need later `this`
// therefore we need to inherit from enable_shared_from_this
//
class session : public enable_shared_from_this<session>
{
public:

    tcp::socket m_socket;

    // our session holds the socket. We pass in a socket reference, but boost makes it so only one person or thing
    // can see the socket at a time. You must std::move it from one place to another. It's kind of stupid.
    session(tcp::socket& s, IReceiveCallback* pCallback)
        : m_socket(std::move(s))
    {
        m_pCallback = pCallback;
    }

    void QueueReceiveCallback()
    {
        async_wait_for_request();
    }

private:

    void async_wait_for_request()
    {
        // since we capture `this` in the callback, we need to call shared_from_this()
        auto self(shared_from_this());

        // since we capture `this` in the callback, we need to call shared_from_this()
        // and now call the lambda once data arrives
        // we read a string until the null termination character
        boost::asio::async_read_until(m_socket, m_buffer, "\n",
            [this, self](boost::system::error_code ec, size_t length)
            {
                if (!ec)
                {
                    // convert the data into a string
                    string data
                    {
                        istreambuf_iterator<char>(&m_buffer),
                        istreambuf_iterator<char>()
                    };

                    // tell the server we received something
                    m_pCallback->OnData(data, m_socket);

                    // go wait for the next thing. The 'self' thing keeps a reference count alive on us,
                    // so we don't die.

                    async_wait_for_request();
                }
                else
                {
                    cout << "server: error or EOS: " << ec << endl;
                    m_socket.close();
                    m_pCallback->OnSocketClose(m_socket);
                }
            });
    }

private:
    boost::asio::streambuf m_buffer;
    IReceiveCallback* m_pCallback;
};