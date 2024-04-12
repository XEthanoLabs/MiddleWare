#include "../common/Client/Client.h"

int main(int argc, char* argv[])
{
    Sleep(1500);
    Client cl("C");
    cl.Connect();
    cl.SubscribeTopic("TopicA");
    Sleep(10000);
    cl.Shutdown();
    return 0;
}