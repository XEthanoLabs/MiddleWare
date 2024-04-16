#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <list>
#include "../MiscFuncs.h"
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;
using ip::tcp;

extern string szEmergencyTopic;

// we have to derive from this in order to stay alive, per Boost's requirements. Otherwise,
// when requesting a read request, we'll "go of scope" and accidentally die.

class Client : public enable_shared_from_this<Client>
{
    boost::asio::io_context m_pIoContext;   // engine that processes all IO
    string m_szUs;                          // for debugging
    tcp::socket m_Socket;                   // the socket we're connected with
    tcp::resolver m_Resolver;               // reolve IP addresses
    boost::asio::streambuf m_buffer;        // buffer to read data into

    void async_read_request();

public:

    Client(string szName);
    ~Client();
    void Connect();
    void CreateTopic(string szTopic);
    void SubscribeTopic(string szTopic);
    void UnsubscribeTopic(string szTopic );
    void SendMessage(string szTopic, string szMessage, char chPriority); // ASCII '0' is 'normal' and anything higher is higher
    void Shutdown();
    void process_io_until_done();
    bool process_some_io();

};

