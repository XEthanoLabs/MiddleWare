#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;
using ip::tcp;

string pszUs = "ClientA:";

class Client
{
    string m_szUs;
    boost::asio::io_context m_pIoContext;
    tcp::socket m_Socket;
    tcp::resolver m_Resolver;

    void async_read_request();

public:

    Client(string szName);
    void Connect();
    void CreateTopic(string szTopic);
    void SubscribeTopic(string szTopic);
    void SendMessage(string szTopic, string szMessage);
    void Shutdown();

};

Client::Client(string szName)
    : m_pIoContext()
    , m_Socket( m_pIoContext )
    , m_Resolver( m_pIoContext )
{
    m_szUs = szName;
}

void Client::Connect()
{
    boost::asio::connect(m_Socket, m_Resolver.resolve("127.0.0.1", "8087"));
    // now we can use connect(..)
    async_read_request();
}

void Client::async_read_request()
{
    boost::asio::streambuf m_buffer;

    boost::asio::async_read_until(m_Socket, m_buffer, "\n",
        [this, &m_buffer](boost::system::error_code ec, size_t length)
        {
            cout << "async_read_until completed " << ec << " " << length << endl;

            // if there was no error, everything went well and for this demo
            // we print the data to stdout and wait for the next request
            if (!ec)
            {
                string data
                {
                    istreambuf_iterator<char>(&m_buffer),
                    istreambuf_iterator<char>()
                };
                // we just print the data, you can here call other api's 
                // or whatever the server needs to do with the received data
                cout << data << endl;
                async_read_request();
            }
            else
            {
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

    // the result represents the size of the sent data
    std::cout << pszUs << "data sent: " << data.length() << '/' << result << std::endl;
}

void Client::SubscribeTopic(string szTopic)
{
    // and use write(..) to send some data which is here just a string
    std::string data{ m_szUs + "|SUBSCRIBE|" + szTopic + "\n" };
    auto result = boost::asio::write(m_Socket, boost::asio::buffer(data, data.length()));

    // the result represents the size of the sent data
    std::cout << pszUs << "data sent: " << data.length() << '/' << result << std::endl;

}

void Client::SendMessage(string szTopic, string szMessage)
{
    std::string data{ m_szUs + "|MESG|" + szTopic + "|" + szMessage + "\n"};
    auto result = boost::asio::write(m_Socket, boost::asio::buffer(data, data.length()));

    // the result represents the size of the sent data
    std::cout << pszUs << "data sent: " << data.length() << '/' << result << std::endl;
}

void Client::Shutdown()
{
    // and close the connection now
    boost::system::error_code ec;

    cout << pszUs << "Shutting down socket" << endl;
    m_Socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, /*ref*/ ec);
    cout << pszUs << "Shut down socket, calling close" << endl;
    m_Socket.close();
    cout << pszUs << "closed socket" << endl;
}

