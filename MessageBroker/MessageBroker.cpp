// MessageBroker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

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

class Server : public IReceiveCallback
{
    io_service* m_pIoService;       // mandatory for using anything in asio
    tcp::acceptor* m_pAcceptor;     // accepts connections to incoming sockets on a certain port
    list<TopicRoom*> m_Topics;
    list<ConnectedClient*> m_Clients;

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
        m_pAcceptor->async_accept([this](boost::system::error_code ec, tcp::socket sk)
            {
                if (!ec)
                {
                    cout << "creating a socket on: "
                        << sk.remote_endpoint().address().to_string()
                        << ":" << sk.remote_endpoint().port() << "\n";

                    // create a pointer to a session. We pass in the (new) socket.
                    // The session will store the socket for us which maintains the connection.
                    // The 'shared_ptr" thing makes it so that the allocation of the session (normally, calling 'new')
                    // stays around until it needs to. Kind of like magic.
                    shared_ptr<session> pSession = make_shared<session>(sk, this); // you HAVE to use move
                    ConnectedClient* pCC = new ConnectedClient(pSession, "");
                    this->m_Clients.push_back(pCC);


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

    void SendMessagesToClientsLoop()
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
            for (list<TopicRoom*>::iterator tr = m_Topics.begin(); tr != m_Topics.end(); tr++)
            {
                TopicRoom* trp = *tr;
                if (trp->AnyMessagesToSend(true))
                {
                    trp->SendMessagesOfPriority(true);
                }
                else
                {
                    trp->SendMessagesOfPriority(false);
                }
            }
        }

        delete m_pAcceptor;
        delete m_pIoService;
    }

    void IncomingSetClientName(string& szClient, tcp::socket& s )
    {
        for (ConnectedClient* cc : m_Clients)
        {
            if (cc->m_pSession->m_socket.native_handle() == s.native_handle() )
            {
                // found it
                cc->m_szClientName = szClient;
                break;
            }
        }
    }

    void IncomingCreateTopic(string& szClient, string& szTopicName)
    {
        TopicRoom* pTopic = new TopicRoom(szTopicName);
        m_Topics.push_back(pTopic);
    }

    // if the topic doesn't exist?

    TopicRoom* FindTopicRoom(string& szTopicName)
    {
        for (TopicRoom* ptr : m_Topics)
        {
            if (ptr->m_szTopic == szTopicName)
            {
                return ptr;
            }
        }
        return nullptr;
    }

    // immediately subscribe to a topic. Don't have a priority for this

    void IncomingSubscribeTopic(string& szClient, string& szTopicName)
    {
        ConnectedClient* pCC = GetConnectedClientFromName(szClient);
        if (pCC == nullptr)
        {
            return;
        }

        TopicRoom* ptr = FindTopicRoom(szTopicName);
        if (ptr == nullptr)
        {
            string sz = "Server: The topic " + szTopicName + " doesn't exist.";
            pCC->m_pSession->Write(sz);

            return;
        }

        ptr->AddClient(pCC);
    }

    // add in coming message to list of messages to send out. We'll send it when we have time

    void IncomingMessage(bool bHiPri, string& szClient, string& szTopicName, string& szMesg)
    {
        TopicRoom* ptr = FindTopicRoom(szTopicName);
        if (ptr == nullptr)
        {
            return;
        }

        ConnectedClient* pCC = GetConnectedClientFromName(szClient);
        if (pCC == nullptr)
        {
            return;
        }

        MessageAndPriority mp;
        mp.Priority = bHiPri ? 0 : 1;
        mp.Text = "(" + szClient + ")" + szMesg;
        if (bHiPri)
        {
            mp.Text = "!!! " + mp.Text; // mark it as high priority
        }

        ptr->AddMessageToSend(mp);
    }

    ConnectedClient* GetConnectedClientFromName(string& szClient)
    {
        for (ConnectedClient* cc : m_Clients)
        {
            if (cc->m_szClientName == szClient)
            {
                return cc;
            }
        }
        return nullptr;
    }

    ConnectedClient* GetConnectedClientFromSocket(tcp::socket& socket)
    {
        for (ConnectedClient* cc : m_Clients)
        {
            if (cc->m_pSession->m_socket.native_handle() == socket.native_handle())
            {
                return cc;
            }
        }
        return nullptr;
    }

    void _ProcessSingleCommand(string& szSingleLine, tcp::socket& socket)
    {
        // parse what the incoming command wanted to do.
        list<string> szPieces;
        SplitString(szSingleLine, '|', szPieces);
        string szClient = szPieces.front();
        szPieces.pop_front();
        string szCommand = szPieces.front();
        szPieces.pop_front();

        cout << "server got command from client " + szClient + ", command = " + szCommand << endl;

        if (szCommand == "CONNECT")
        {
            IncomingSetClientName(szClient, socket);
        }

        if (szCommand == "CREATE")
        {
            // create a new topic. other clients can now subscribe to it
            string szTopicName = szPieces.front();
            IncomingCreateTopic(szClient, szTopicName);
        }
        if (szCommand == "SUBSCRIBE")
        {
            string szTopicName = szPieces.front();
            IncomingSubscribeTopic(szClient, szTopicName);
        }
        if (szCommand == "MESG")
        {
            string szPriority = szPieces.front();
            szPieces.pop_front();
            bool bHiPriority = szPriority == "H";
            string szTopic = szPieces.front();
            szPieces.pop_front();
            string szMesg = szPieces.front();
            IncomingMessage(bHiPriority, szClient, szTopic, szMesg);
        }
    }

    void OnData(string& szMesg, tcp::socket& socket)
    {
        while (szMesg.size())
        {
            string szFirst;
            PullOutStringUntilDelimiter(szMesg, '\n', szFirst);
            if (szFirst.empty())
            {
                szFirst = szMesg;
            }
            if (szFirst.empty())
            {
                break;
            }
            _ProcessSingleCommand(szFirst, socket);
        }
    }

    void OnSocketClose(tcp::socket& socket)
    {
        // the remote client lost their socket. 
        ConnectedClient* pCC = GetConnectedClientFromSocket(socket);
        if (!pCC)
        {
            // ??? now what?
            return;
        }

        // find all topic rooms that have that client in them, and remove the client.
        // If the topic room has no more participants, remove it
        for (list<TopicRoom*>::iterator trp = m_Topics.begin() ; trp != m_Topics.end() ; trp++ )
        {
            cout << "Removing client from topic " << (*trp)->m_szTopic << endl;
            (*trp)->RemoveClient(pCC);
            if (!(*trp)->HasAnyParticipants())
            {
                cout << "Topic " + (*trp)->m_szTopic + " has no more clients. Removing." << endl;

                trp = m_Topics.erase(trp);
                if (trp == m_Topics.end())
                {
                    break;
                }
            }
        }

        // remove the connected client
        for (list<ConnectedClient*>::iterator pcc = m_Clients.begin(); pcc != m_Clients.end(); pcc++)
        {
            if (*pcc == pCC)
            {
                // same pointer
                pcc = m_Clients.erase(pcc);
                if (pcc == m_Clients.end())
                {
                    cout << "Removing client from m_Clients list" << endl;
                    break;
                }
            }
        }
    }
};

int main()
{
    Server m_Server;
    m_Server.SendMessagesToClientsLoop();
    return 0;
}
