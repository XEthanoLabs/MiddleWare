// MessageBroker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <zmq.hpp>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;
using ip::tcp;

#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "libzmq-v143-mt-gd-4_3_6.lib" )

// this was created as shared ptr and we need later `this`
// therefore we need to inherit from enable_shared_from_this
class session : public enable_shared_from_this<session>
{
public:

    // our session holds the socket
    session(tcp::socket socket)
        : m_socket(move(socket)) 
    { 
    }

    void QueueReceiveCallback() 
    {
        cout << "session:run" << endl;
        async_wait_for_request();
        cout << "session:run exiting" << endl;
    }

private:
    void async_wait_for_request()
    {
        // since we capture `this` in the callback, we need to call shared_from_this()
        auto self(shared_from_this());
        // and now call the lambda once data arrives
        // we read a string until the null termination character
        boost::asio::async_read_until(m_socket, m_buffer, "\n",
            [this, self](boost::system::error_code ec, size_t length)
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
                    async_wait_for_request();
                }
                else 
                {
                    cout << "error or EOS: " << ec << endl;
                    m_socket.close();
                }
            });
    }
private:
    tcp::socket m_socket;
    boost::asio::streambuf m_buffer;
};


class Server
{
    io_service* m_pIoService;       // mandatory for using anything in asio
    tcp::socket* m_pSocket;
    tcp::acceptor* m_pAcceptor;     // accepts connections to incoming sockets on a certain port

public:

    Server()
    {
        // just boost basics stuff. Can't use socket stuff w/o an IoService to do some gruntwork
        m_pIoService = new io_service();
        m_pAcceptor = new tcp::acceptor(*m_pIoService, tcp::endpoint(tcp::v4(), 8087));
        // call this before run() below
        AsyncAllowClientToJoin();
        // run until there is nothing left to do and the service is shut down
        while (true)
        {
            auto ret = m_pIoService->run_one();
            if (ret == 1)
            {
                continue;
            }
        }

        delete m_pAcceptor;
        delete m_pIoService;

        // m_pIoService->run();
    }

    void AsyncAllowClientToJoin()
    {
        m_pAcceptor->async_accept([this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    cout << "creating a socket on: "
                        << socket.remote_endpoint().address().to_string()
                        << ":" << socket.remote_endpoint().port() << "\n";

                    // create a pointer to a session. We pass in the (new) socket.
                    // The session will store the socket for us which maintains the connection.
                    // The 'shared_ptr" thing makes it so that the allocation of the session (normally, calling 'new')
                    // stays around until it needs to. Kind of like magic.
                    shared_ptr<session> pSession = make_shared<session>(move(socket)); // you HAVE to use move
                    // go process all calls to this socket until the socket it closed.
                    // This is implicit multithreading?
                    pSession->QueueReceiveCallback();
                }
                else
                {
                    cout << "error: " << ec << endl;
                }

                // since we just accepted one (or failed one), we want other clients to connect,
                // so we'll call this again
                AsyncAllowClientToJoin();
            });

        // this happens even if the socket hasn't accepted yet. falls through and keeps going. 
        // This allows the main constructor to go on and do more stuff.
    }

    void ReadLoop()
    {
    }
};

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    Server m_Server;
    m_Server.ReadLoop();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
