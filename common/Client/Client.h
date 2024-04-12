#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;
using ip::tcp;

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
    void SendMessage(string szTopic, string szMessage, bool bHiPriority);
    void Shutdown();
    void process_io_until_done();
    bool process_some_io();

};

Client::Client(string szName)
    : m_pIoContext()                // this is the IO request processing engine
    , m_Socket( m_pIoContext )      // start with a blank socket
    , m_Resolver( m_pIoContext )    // dumb class to resolve IP addresses. Does the job though.
{
    m_szUs = szName;
}

Client::~Client()
{
    cout << "Client " + m_szUs + " destructor" << endl;
}

void Client::Connect()
{
    // connect to the local server. The server should be running already. Over port 8087.
    // if this connects, the socket is now valid
    boost::asio::connect(m_Socket, m_Resolver.resolve("127.0.0.1", "8087"));

    // tell the server who we are upon initial connect. This enables it to identify a socket # with a client name
    std::string data{ m_szUs + "|CONNECT" + "\n" };
    auto result = boost::asio::write(m_Socket, boost::asio::buffer(data, data.length()));
    
    // submit a read request, which will then submit another one if it reads something successfully
    async_read_request();
}

void Client::async_read_request()
{
    // since we capture `this` in the callback, we need to call shared_from_this()
    auto self(shared_from_this());

    // read until the server sends a \n. The server MIGHT have sent SEVERAL strings ending in \n!
    // It is our job to separate them if we care!

    boost::asio::async_read_until(m_Socket, m_buffer, "\n",
        [this, self](boost::system::error_code ec, size_t length)
        {
            if (!ec)
            {
                // convert the incoming buffer into a string
                string data
                {
                    istreambuf_iterator<char>(&m_buffer),
                    istreambuf_iterator<char>()
                };

                cout << "client " + m_szUs + " received message: " + data << endl;

                // read some more. This keeps the read chain alive
                async_read_request();
            }
            else
            {
                // this will cause the IO thingie to exit the run loop
                cout << "error or EOS: " << ec << endl;
                m_Socket.close();
            }
        });

}

void Client::CreateTopic(string szTopic)
{
    // and use write(..) to send some data which is here just a string
    std::string data{ m_szUs + "|CREATE|" + szTopic + "\n" };
    auto result = boost::asio::write(m_Socket, boost::asio::buffer(data, data.length()));
}

void Client::SubscribeTopic(string szTopic)
{
    // and use write(..) to send some data which is here just a string
    std::string data{ m_szUs + "|SUBSCRIBE|" + szTopic + "\n" };
    auto result = boost::asio::write(m_Socket, boost::asio::buffer(data, data.length()));
}

void Client::SendMessage(string szTopic, string szMessage, bool bHiPriority)
{
    char chPriority = 'M';
    if (bHiPriority)
    {
        chPriority = 'H';
    }

    std::string data{ m_szUs + "|MESG|" + chPriority + "|" + szTopic + "|" + szMessage + "\n"};
    auto result = boost::asio::write(m_Socket, boost::asio::buffer(data, data.length()));
}

void Client::process_io_until_done()
{
    cout << "Client " + m_szUs + " processing all IO requests" << endl;
    m_pIoContext.run();
    cout << "Client " + m_szUs + " done processing, exiting" << endl;
}

bool Client::process_some_io()
{
    // m_pIoService->run();
    io_context::count_type count = m_pIoContext.poll_one();
    return !m_pIoContext.stopped();
}

void Client::Shutdown()
{
    // and close the connection now
    boost::system::error_code ec;

    cout << m_szUs << "Shutting down socket" << endl;
    m_Socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, /*ref*/ ec);
    cout << m_szUs << "Shut down socket, calling close" << endl;
    m_Socket.close();
    cout << m_szUs << "closed socket" << endl;
}

