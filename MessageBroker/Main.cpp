#include "MessageBroker.h"

int main()
{
    Server m_Server;
    m_Server.SendMessagesToClientsLoop();
    return 0;
}