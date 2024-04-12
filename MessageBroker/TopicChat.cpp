#include "TopicChat.h"

void TopicRoom::AddClient(ConnectedClient cc)
{
    m_ClientList.push_back(cc);
}

void TopicRoom::RemoveClient(ConnectedClient cc)
{
    for (list<ConnectedClient>::iterator i = m_ClientList.begin(); i != m_ClientList.end(); i++)
    {
        if (cc.m_szClientName == i->m_szClientName)
        {
            i = m_ClientList.erase(i);
        }
    }
}

bool TopicRoom::AnyMessagesToSend(bool bHiPriority)
{
    for (const MessageAndPriority& msg : m_Messages)
    {
        if ((msg.Priority == 0) && bHiPriority)
        {
            return true;
        }
        if ((msg.Priority == 1) && !bHiPriority)
        {
            return true;
        }
    }
    return false;
}

void TopicRoom::SendMessage(MessageAndPriority& msg)
{
    for (ConnectedClient& pClient : m_ClientList)
    {
        auto s = std::move(pClient.m_pSession->m_socket);
        auto result = boost::asio::write(s, boost::asio::buffer(msg.Text, msg.Text.length()));
    }
}

void TopicRoom::SendMessagesOfPriority(bool bHiPriority)
{
    bool bFoundHighPriority = false;

    for (list<MessageAndPriority>::iterator i = m_Messages.begin(); i != m_Messages.end(); i++)
    {
        if ((i->Priority == 0) && bHiPriority)
        {
            bFoundHighPriority = true;
            SendMessage(*i);
            i = m_Messages.erase(i);
        }
    }

    if (bFoundHighPriority)
    {
        return;
    }

    for (list<MessageAndPriority>::iterator i = m_Messages.begin(); i != m_Messages.end(); i++)
    {
        if ((i->Priority == 1) && !bHiPriority)
        {
            SendMessage(*i);
            i = m_Messages.erase(i);
        }
    }
}

