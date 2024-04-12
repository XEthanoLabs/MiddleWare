// MessageBroker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

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

#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "libzmq-v143-mt-gd-4_3_6.lib" )

interface IReceiveCallback
{
    HRESULT virtual OnData(boost::asio::streambuf& buffer, tcp::socket& socket) = 0;
    HRESULT virtual OnCreateTopic(string szClient, string szTopic) = 0;
    HRESULT virtual OnSubscribeTopic(string szClient, string szTopic) = 0;
    HRESULT virtual OnMessage(string szClient, string szTopic, string szMesg) = 0;
};

// this was created as shared ptr and we need later `this`
// therefore we need to inherit from enable_shared_from_this
class session : public enable_shared_from_this<session>
{
public:

    // our session holds the socket
    session(tcp::socket socket, IReceiveCallback* pCallback)
        : m_socket(move(socket)) 
    { 
        m_pCallback = pCallback;
    }

    void QueueReceiveCallback() 
    {
        cout << "session:run" << endl;
        async_wait_for_request();
        cout << "session:run exiting" << endl;
    }

private:
    void _ProcessSingleCommand(string& szSingleLine)
    {
        // parse what the incoming command wanted to do.
        list<string> szPieces;
        SplitString(szSingleLine, '|', szPieces);
        string szClient = szPieces.front();
        szPieces.pop_front();
        string szCommand = szPieces.front();
        szPieces.pop_front();

        if (szCommand == "CREATE")
        {
            // create a new topic. other clients can now subscribe to it
            string szTopicName = szPieces.front();
            m_pCallback->OnCreateTopic(szClient, szTopicName);
        }
        if (szCommand == "SUBSCRIBE")
        {
            string szTopicName = szPieces.front();
            m_pCallback->OnSubscribeTopic(szClient, szTopicName);
        }
        if (szCommand == "MESG")
        {
            string szTopic = szPieces.front();
            szPieces.pop_front();
            string szMesg = szPieces.front();
            m_pCallback->OnMessage(szClient, szTopic, szMesg);
        }
    }

    void _ProcessReadString(string& szRead)
    {
        while (szRead.size())
        {
            string szFirst;
            PullOutStringUntilDelimiter(szRead, '\n', szFirst);
            if (szFirst.empty())
            {
                szFirst = szRead;
            }
            if (szFirst.empty())
            {
                break;
            }
            _ProcessSingleCommand(szFirst);
        }
    }

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

                    _ProcessReadString(data);

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
    IReceiveCallback* m_pCallback;
};


class Server : public IReceiveCallback
{
    io_service* m_pIoService;       // mandatory for using anything in asio
    tcp::socket* m_pSocket;
    tcp::acceptor* m_pAcceptor;     // accepts connections to incoming sockets on a certain port
    list<TopicRoom> m_Topics;

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

        m_pAcceptor->async_accept([this](boost::system::error_code ec, tcp::socket socket)
            {
                cout << "AsyncAllowClientToJoin got accept, thread id = " << GetCurrentThreadId() << endl;

                if (!ec)
                {
                    cout << "creating a socket on: "
                        << socket.remote_endpoint().address().to_string()
                        << ":" << socket.remote_endpoint().port() << "\n";

                    // create a pointer to a session. We pass in the (new) socket.
                    // The session will store the socket for us which maintains the connection.
                    // The 'shared_ptr" thing makes it so that the allocation of the session (normally, calling 'new')
                    // stays around until it needs to. Kind of like magic.
                    shared_ptr<session> pSession = make_shared<session>(move(socket), this); // you HAVE to use move
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
                tr->SendMessages();
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

    HRESULT OnCreateTopic(string szClient, string szTopic)
    {
        return S_OK;
    }

    HRESULT OnSubscribeTopic(string szClient, string szTopic)
    {
        return S_OK;
    }

    HRESULT OnMessage(string szClient, string szTopic, string szMesg)
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
