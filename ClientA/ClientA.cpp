#include "../common/Client/Client.h"

int main(int argc, char* argv[])
{
    Sleep(1000);
    Client cl("A");
    cl.Connect();
    cl.CreateTopic("TopicA");
    cl.SendMessageW("TopicA", "Hello from client A", false);
    Sleep(10000);
    cl.Shutdown();
    return 0;
}