#include "TopicChat.h"

void TopicRoom::AddClient(ConnectedClient* cc)
{
    m_ClientList.push_back(cc);
}

void TopicRoom::RemoveClient(ConnectedClient* cc)
{
    for (list<ConnectedClient*>::iterator i = m_ClientList.begin(); i != m_ClientList.end(); i++)
    {
        if (cc->m_szClientName == (*i)->m_szClientName)
        {
            i = m_ClientList.erase(i);
            if (i == m_ClientList.end())
            {
                break;
            }
        }
    }
}

bool TopicRoom::AnyMessagesToSend(bool bHiPriority)
{
    for (const MessageAndPriority& msg : m_MessagesToSend)
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
    msg.Text += "\n";

    for (ConnectedClient* pClient : m_ClientList)
    {
        cout << "Sending message to " + pClient->m_szClientName + ", message = " + msg.Text << endl;

        auto result = boost::asio::write(pClient->m_pSession->m_socket, boost::asio::buffer(msg.Text, msg.Text.length()));
    }
}

void TopicRoom::SendMessagesOfPriority(bool bHiPriority)
{
    if (m_MessagesToSend.size() == 0)
    {
        return;
    }

    bool bFoundHighPriority = false;

    for (list<MessageAndPriority>::iterator i = m_MessagesToSend.begin(); i != m_MessagesToSend.end(); i++)
    {
        if ((i->Priority == 0) && bHiPriority)
        {
            bFoundHighPriority = true;
            SendMessage(*i);
            i = m_MessagesToSend.erase(i);
            if (i == m_MessagesToSend.end())
            {
                break;
            }
        }
    }

    if (bFoundHighPriority)
    {
        return;
    }

    for (list<MessageAndPriority>::iterator i = m_MessagesToSend.begin(); i != m_MessagesToSend.end(); i++)
    {
        if ((i->Priority == 1) && !bHiPriority)
        {
            SendMessage(*i);
            i = m_MessagesToSend.erase(i);
            if (i == m_MessagesToSend.end())
            {
                break;
            }
        }
    }
}

void TopicRoom::AddMessageToSend(MessageAndPriority msg)
{
    m_MessagesToSend.push_back(msg);
}

