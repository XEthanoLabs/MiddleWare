#include "../common/Client/Client.h"
#include <thread>

int main(int argc, char* argv[])
{
    std::this_thread::sleep_for(2000ms);
    shared_ptr<Client> cl = make_shared<Client>("C");
    cl->Connect();
    cl->SubscribeTopic(szEmergencyTopic);
    cl->process_io_until_done();
    cl->Shutdown();
    return 0;
}