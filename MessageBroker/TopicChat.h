#pragma once
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <list>

#include "../common/DataStructs/DataStructs.h"
#include "ConnectedClient.h"

using namespace std;
using namespace boost::asio;
using ip::tcp;

// this is a class that holds the info for a specific chat topic. who is involved with the chat,
// and what messages need to be sent

class TopicRoom
{
public:

    string m_szTopic;

    TopicRoom(string& szTopic)
    {
        m_szTopic = szTopic;
    }

    list<shared_ptr<ConnectedClient>> m_ClientList;    // who is connected
    list<MessageAndPriority> m_MessagesToSend;

    void AddClient(shared_ptr<ConnectedClient> cc);
    void RemoveClient(string& szClientName);
    bool HasAnyParticipants();
    bool AnyMessagesToSend(bool bHiPriority);
    void SendMessagesOfPriority(bool bHiPriority);
    void SendMessage(MessageAndPriority& msg);
    void AddMessageToSend(MessageAndPriority msg);
};

