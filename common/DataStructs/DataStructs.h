#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <list>

using namespace std;
using namespace boost::asio;
using ip::tcp;

struct MessageAndPriority
{
    string Text;
    byte Priority;
};

class TopicRoom
{
    void _SendMessage(list<MessageAndPriority>::iterator& i)
    {
#if true
        for (list<tcp::socket*>::iterator s = m_SocketList.begin(); s != m_SocketList.end(); s++)
        {
            auto result = boost::asio::write(**s, boost::asio::buffer(i->Text, i->Text.length()));
        }
#endif

        m_Messages.erase(i);
    }

public:

    // who is subscribed to this topic?
    list<tcp::socket*> m_SocketList;
    list<MessageAndPriority> m_Messages;

    void AddSubscriber(tcp::socket* pSocket);
    void RemSubscriber(tcp::socket pSocket);
    bool HasMessagesToSend();
    bool SendMessages();
    void AddMessage(MessageAndPriority msg);

};

void TopicRoom::AddSubscriber(tcp::socket* pSocket)
{
    m_SocketList.push_back(pSocket);
}

void TopicRoom::RemSubscriber(tcp::socket pSocket)
{
    // find it and remove it
    // m_SocketList.remove(pSocket);
}

bool TopicRoom::HasMessagesToSend()
{
    return m_Messages.size() != 0;
}

bool TopicRoom::SendMessages()
{
    for (list<MessageAndPriority>::iterator i = m_Messages.begin(); i != m_Messages.end(); i++)
    {
        if (i->Priority == 0)
        {
            _SendMessage(i); // removes it from the list
        }
    }

    for (list<MessageAndPriority>::iterator i = m_Messages.begin(); i != m_Messages.end(); i++)
    {
        if (i->Priority == 0)
        {
            _SendMessage(i); // removes it from the list
        }
    }

    return true;
}
