#include "../common/Client/Client.h"

int main(int argc, char* argv[])
{
    Sleep(1500);
    shared_ptr<Client> cl = make_shared<Client>("C");
    cl->Connect();
    cl->SubscribeTopic("TopicA");
    cl->process_io_until_done();
    cl->Shutdown();
    return 0;
}