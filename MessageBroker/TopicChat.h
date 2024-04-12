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

class TopicRoom
{
    string m_szTopic;

public:

    TopicRoom(string& szTopic)
    {
        m_szTopic = szTopic;
    }

    list<ConnectedClient> m_ClientList;
    list<MessageAndPriority> m_Messages;

    void AddClient(ConnectedClient cc);
    void RemoveClient(ConnectedClient cc);
    bool AnyMessagesToSend(bool bHiPriority);
    void SendMessagesOfPriority(bool bHiPriority);
    void SendMessage(MessageAndPriority& msg);
};

