#pragma once
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <list>
#include <queue>

#include "../common/DataStructs/DataStructs.h"
#include "ConnectedClient.h"

using namespace std;
using namespace boost::asio;
using ip::tcp;

// this is a class that holds the info for a specific chat topic. who is involved with the chat,
// and what messages need to be sent

struct MessageLess
{
    bool operator()(MessageAndPriority& l, MessageAndPriority& r) 
    { 
        return l.Priority > r.Priority; 
    }
};

class TopicRoom
{
    void SendMessage(MessageAndPriority& msg);

public:

    string m_szTopic;

    TopicRoom(string& szTopic)
    {
        m_szTopic = szTopic;
    }

    list<shared_ptr<ConnectedClient>> m_ClientList;    // who is connected
    priority_queue<MessageAndPriority, vector<MessageAndPriority>, MessageLess> m_MessageQueue;
    list<MessageAndPriority> m_MessagesToSend;

    void AddClient(shared_ptr<ConnectedClient> cc);
    void RemoveClient(string& szClientName);
    bool HasAnyParticipants();
    char HighestPriorityInSendQueue();
    bool SendMessagesOfPriority(char chPriority, bool& bSomethingSent); // returns true if there are messages left of a lower priority
    void AddMessageToSend(MessageAndPriority msg);
};

