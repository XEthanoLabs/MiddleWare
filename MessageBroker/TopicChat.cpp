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

bool TopicRoom::HasAnyParticipants()
{
    return m_ClientList.size() > 0;
}

bool TopicRoom::AnyMessagesToSend(bool bHiPriority)
{
    for (const MessageAndPriority& msg : m_MessagesToSend)
    {
        if (((int)msg.Priority == 0) && bHiPriority)
        {
            return true;
        }
        if (((int)msg.Priority == 1) && !bHiPriority)
        {
            return true;
        }
    }
    return false;
}

void TopicRoom::SendMessage(MessageAndPriority& msg)
{
    for (ConnectedClient* pClient : m_ClientList)
    {
        cout << "Sending message to " + pClient->m_szClientName + ", message = " + msg.Text << endl;

        pClient->m_pSession->Write(msg.Text);
    }
}

void TopicRoom::SendMessagesOfPriority(bool bHiPriority)
{
    if (m_MessagesToSend.size() == 0)
    {
        return;
    }

    bool bFoundHighPriority = false;

    // send all high priority messages first, if hiPriority is set on input
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

    // send all non-high priority messages, if hiPriority is NOT set
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

