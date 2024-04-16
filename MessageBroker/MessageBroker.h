#pragma once

#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "../common/DataStructs/DataStructs.h"
#include "../common/MiscFuncs.h"
#include "TopicChat.h"
#include "ConnectedClient.h"

using namespace std;
using namespace boost::asio;
using ip::tcp;

TopicRoom* m_pTopicRoom = nullptr;

class Server
{
    io_service* m_pIoService;       // mandatory for using anything in asio
    tcp::acceptor* m_pAcceptor;     // accepts connections to incoming sockets on a certain port
    list<TopicRoom*> m_Topics;
    list<shared_ptr<ConnectedClient>> m_Clients;

    void AsyncAllowClientToJoin();
    void IncomingSetClientName(string& szClient, tcp::socket& s );
    void IncomingCreateTopic(string& szClient, string& szTopicName);
    TopicRoom* FindTopicRoom(string& szTopicName);
    void IncomingSubscribeTopic(string& szClient, string& szTopicName);
    void IncomingUnsubscribeTopic(string& szClient, string& szTopicName);
    void IncomingMessage(char chPriority, string& szClient, string& szTopicName, string& szMesg);
    shared_ptr<ConnectedClient>& GetConnectedClientFromName(string& szClient);
    shared_ptr<ConnectedClient>& GetConnectedClientFromSocket(tcp::socket& socket);
    void _ProcessSingleCommand(string& szSingleLine, tcp::socket& socket);

public:

    Server();
    void SendMessagesToClientsLoop();
    // called by ConnectedClient. Probably should be 'protected' and ConnectedClient declared as friend class?
    void OnData(string& szMesg, tcp::socket& socket);
    void OnSocketClose(tcp::socket& socket);
};
