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

class MessageBroker
{
    io_service* m_pIoService;       // mandatory for using anything in asio
    tcp::acceptor* m_pAcceptor;     // accepts connections to incoming sockets on a certain port
    list<TopicChat*> m_Topics;
    list<shared_ptr<ConnectedClient>> m_Clients;

    shared_ptr<ConnectedClient>& GetConnectedClientFromName(string& szClient);
    shared_ptr<ConnectedClient>& GetConnectedClientFromSocket(tcp::socket& socket);
    TopicChat* FindTopicRoom(string& szTopicName);

    // start accepting clients
    void AsyncAllowClientToJoin();

    // process a single line received from a socket. This will call the IncomingXXX below
    void _ProcessSingleCommand(string& szSingleLine, tcp::socket& socket);

    // process a specific command that was parsed from the single line above
    void IncomingSetClientName(string& szClient, tcp::socket& s );
    void IncomingCreateTopic(string& szClient, string& szTopicName);
    void IncomingSubscribeTopic(string& szClient, string& szTopicName);
    void IncomingUnsubscribeTopic(string& szClient, string& szTopicName);
    void IncomingMessage(char chPriority, string& szClient, string& szTopicName, string& szMesg);

public:

    MessageBroker();
    void SendMessagesToClientsLoop();
    // called by ConnectedClient. Probably should be 'protected' and ConnectedClient declared as friend class?
    void OnData(string& szMesg, tcp::socket& socket);
    void OnSocketClose(tcp::socket& socket);
};
