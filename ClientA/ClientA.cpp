#include "../common/Client/Client.h"

int main(int argc, char* argv[])
{
    Sleep(1000);
    shared_ptr<Client> cl = make_shared<Client>("A");
    cl->Connect();
    cl->CreateTopic("TopicA");
    Sleep(1000);
    cout << "sending message from client A" << endl;
    cl->SendMessageW("TopicA", "Hello from client A", false);
    while (true)
    {
        bool bOk = cl->process_some_io();
        if (!bOk) break;
        Sleep(100);
        if (rand() % 100 == 1)
        {
            cl->SendMessageW("TopicA", "Client A wants to say hello...", false);
        }
    }

    cl->Shutdown();
    return 0;
}