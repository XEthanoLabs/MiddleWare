#pragma once
#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <list>

#include "../common/DataStructs/DataStructs.h"
#include "ConnectedClient.h"
#include "session.h"

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

    list<ConnectedClient*> m_ClientList;    // who is connected
    list<MessageAndPriority> m_MessagesToSend;

    void AddClient(ConnectedClient* cc);
    void RemoveClient(ConnectedClient* cc);
    bool HasAnyParticipants();
    bool AnyMessagesToSend(bool bHiPriority);
    void SendMessagesOfPriority(bool bHiPriority);
    void SendMessage(MessageAndPriority& msg);
    void AddMessageToSend(MessageAndPriority msg);
};

