#pragma once

#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "../common/DataStructs/DataStructs.h"
#include "../common/MiscFuncs.h"
#include "MessageBroker.h"

using namespace std;
using namespace boost::asio;
using ip::tcp;

#define interface struct

// a ConnectedClient is a connected socket. 
//
// this was created as shared ptr and we need later `this`
// therefore we need to inherit from enable_shared_from_this
//
class ConnectedClient : public enable_shared_from_this<ConnectedClient>
{
public:

    tcp::socket m_socket;
	string m_szClientName;
    Server* m_pServer;

    // our ConnectedClient holds the socket. We pass in a socket reference, but boost makes it so only one person or thing
    // can see the socket at a time. You must std::move it from one place to another. It's kind of stupid.
    ConnectedClient(tcp::socket& s, Server* pServer)
        : m_socket(std::move(s))
    {
        m_pServer = pServer;
    }

    void QueueReceiveCallback()
    {
        async_wait_for_request();
    }

    void Write(const string& szMesg)
    {
        string msg = szMesg + "\n";
        auto result = boost::asio::write(m_socket, boost::asio::buffer(msg, msg.length()));
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
                    m_pServer->OnData(data, m_socket);

                    // go wait for the next thing. The 'self' thing keeps a reference count alive on us,
                    // so we don't die.

                    async_wait_for_request();
                }
                else
                {
                    cout << "server: error or EOS: " << ec << endl;
                    m_socket.close();
                    m_pServer->OnSocketClose(m_socket);
                }
            });
    }

    boost::asio::streambuf m_buffer;
};
