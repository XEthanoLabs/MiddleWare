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

interface IReceiveCallback
{
    HRESULT virtual OnData(boost::asio::streambuf& buffer, boost::asio::ip::tcp::socket& socket) = 0;
    HRESULT virtual IncomingCreateTopic(string szClient, string szTopic) = 0;
    HRESULT virtual IncomingSubscribeTopic(string szClient, string szTopic) = 0;
    HRESULT virtual IncomingMessage(bool bHiPriority, string szClient, string szTopic, string szMesg) = 0;
};

class session
{
public:

    basic_stream_socket<tcp> m_socket;

    // our session holds the socket
    session(basic_stream_socket<tcp> s, IReceiveCallback* pCallback)
        : m_socket(std::move(s))
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
            m_pCallback->IncomingCreateTopic(szClient, szTopicName);
        }
        if (szCommand == "SUBSCRIBE")
        {
            string szTopicName = szPieces.front();
            m_pCallback->IncomingSubscribeTopic(szClient, szTopicName);
        }
        if (szCommand == "MESG")
        {
            string szPriority = szPieces.front();
            szPieces.pop_front();
            bool bHiPriority = szPriority == "H";
            string szTopic = szPieces.front();
            szPieces.pop_front();
            string szMesg = szPieces.front();
            m_pCallback->IncomingMessage(bHiPriority, szClient, szTopic, szMesg);
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
        // and now call the lambda once data arrives
        // we read a string until the null termination character
        boost::asio::async_read_until(m_socket, m_buffer, "\n",
            [this](boost::system::error_code ec, size_t length)
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
    boost::asio::streambuf m_buffer;
    IReceiveCallback* m_pCallback;
};
