#include "MessageBroker.h"

int main()
{
    MessageBroker server;
    server.SendMessagesToClientsLoop();
    return 0;
}