#include "TopicChat.h"

void TopicChat::AddClient(shared_ptr<ConnectedClient> cc)
{
    m_ClientList.push_back(cc);
}

void TopicChat::RemoveClient(string& szClientName)
{
    MessageAndPriority msgClientLeft;
    msgClientLeft.Priority = '0';
    msgClientLeft.Text = "Client " + szClientName + " has left the topic.";

    for (list<shared_ptr<ConnectedClient>>::iterator i = m_ClientList.begin(); i != m_ClientList.end(); i++)
    {
        if (szClientName == i->get()->m_szClientName)
        {
            i = m_ClientList.erase(i);
            if (i == m_ClientList.end())
            {
                break;
            }
        }
        else
        {
            SendMessage( msgClientLeft );
        }
    }
}

bool TopicChat::HasAnyParticipants()
{
    return m_ClientList.size() > 0;
}

void TopicChat::SendMessage(MessageAndPriority& msg)
{
    for (shared_ptr<ConnectedClient>& pClient : m_ClientList)
    {
        string szEncoded = msg.From + "|" + msg.Topic + "|";
        szEncoded += msg.Priority;
        szEncoded += "|" + msg.Text;

        cout << "Sending message to " + pClient->m_szClientName + ", message = " + msg.Text << endl;

        pClient->Write(szEncoded);
    }
}

char TopicChat::HighestPriorityInSendQueue()
{
    if (m_MessageQueue.size() == 0)
    {
        return 0;
    }
    
    return m_MessageQueue.top().Priority;
}

bool TopicChat::SendMessagesOfPriority(char chPriority, bool& bSentSomething)
{
    if (m_MessageQueue.size() == 0)
    {
        return false;
    }

    if( m_MessageQueue.top().Priority < chPriority )
    {
        // the very thing at the top is too low to send. 
        return true; // more to send
    }

    MessageAndPriority msgTop = m_MessageQueue.top( );
    SendMessage( msgTop );
    m_MessageQueue.pop();
    bSentSomething = true;

    if( m_MessageQueue.empty())
    {
        return false;
    }

    return true;
}

void TopicChat::AddMessageToSend(MessageAndPriority msg)
{
    m_MessageQueue.push(move(msg));
}

