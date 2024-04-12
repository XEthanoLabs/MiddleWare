// MessageBroker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "../common/DataStructs/DataStructs.h"
#include "MiscFuncs.h"
#include "TopicChat.h"
#include "session.h"

using namespace std;
using namespace boost::asio;
using ip::tcp;

#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "libzmq-v143-mt-gd-4_3_6.lib" )

class Server : public IReceiveCallback
{
    io_service* m_pIoService;       // mandatory for using anything in asio
    tcp::acceptor* m_pAcceptor;     // accepts connections to incoming sockets on a certain port
    list<TopicRoom> m_Topics;
    list<ConnectedClient> m_Clients;

    void _ServiceQueues()
    {
    }

public:

    Server()
    {
        // just boost basics stuff. Can't use socket stuff w/o an IoService to do some gruntwork
        m_pIoService = new io_service();
        m_pAcceptor = new tcp::acceptor(*m_pIoService, tcp::endpoint(tcp::v4(), 8087));
        // call this before run() below
        AsyncAllowClientToJoin();
    }

    void AsyncAllowClientToJoin()
    {
        cout << "AsyncAllowClientToJoin, threadid = " << GetCurrentThreadId() << endl;

        m_pAcceptor->async_accept([this](boost::system::error_code ec, tcp::socket sk)
            {
                cout << "AsyncAllowClientToJoin got accept, thread id = " << GetCurrentThreadId() << endl;

                if (!ec)
                {
                    cout << "creating a socket on: "
                        << sk.remote_endpoint().address().to_string()
                        << ":" << sk.remote_endpoint().port() << "\n";

                    // create a pointer to a session. We pass in the (new) socket.
                    // The session will store the socket for us which maintains the connection.
                    // The 'shared_ptr" thing makes it so that the allocation of the session (normally, calling 'new')
                    // stays around until it needs to. Kind of like magic.
                    session* pSession = new session(std::move(sk), this); // you HAVE to use move
                    ConnectedClient* pCC = new ConnectedClient(pSession, "");

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

        cout << "AsyncAllowClientToJoin, continuing threadid = " << GetCurrentThreadId() << endl;

        // this happens even if the socket hasn't accepted yet. falls through and keeps going. 
        // This allows the main constructor to go on and do more stuff.
    }

    void ReadLoop()
    {
        // m_pIoService->run();

        // run until there is nothing left to do and the service is shut down
        while (true)
        {
            // are there any incoming requests? Service at most one of them. This will process an incoming connect
            // or message send
            //
            io_context::count_type count = m_pIoService->poll_one();

            // now go service our queues
            for (list<TopicRoom>::iterator tr = m_Topics.begin(); tr != m_Topics.end(); tr++)
            {
                if (tr->AnyMessagesToSend(true))
                {
                    tr->SendMessagesOfPriority(true);
                }
                else
                {
                    tr->SendMessagesOfPriority(false);
                }
            }
        }

        delete m_pAcceptor;
        delete m_pIoService;
    }

    HRESULT OnData(boost::asio::streambuf& buffer, tcp::socket& socket)
    {
        // associate the socket with a session?
        return S_OK;
    }

    HRESULT IncomingCreateTopic(string szClient, string szTopic)
    {
        return S_OK;
    }

    HRESULT IncomingSubscribeTopic(string szClient, string szTopic)
    {
        return S_OK;
    }

    HRESULT IncomingMessage(bool bHiPriority, string szClient, string szTopic, string szMesg)
    {
        return S_OK;
    }
};

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    Server m_Server;
    m_Server.ReadLoop();
}
