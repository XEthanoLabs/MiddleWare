#pragma once

#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "../common/DataStructs/DataStructs.h"
#include "../common/MiscFuncs.h"

using namespace std;
using namespace boost::asio;
using ip::tcp;

class MessageBroker;

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
    MessageBroker* m_pServer;

    // our ConnectedClient holds the socket. We pass in a socket reference, but boost makes it so only one person or thing
    // can see the socket at a time. You must std::move it from one place to another. It's kind of stupid.
    ConnectedClient(tcp::socket& s, MessageBroker* pServer)
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

    void async_wait_for_request();

    boost::asio::streambuf m_buffer;
};
