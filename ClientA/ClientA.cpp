#include "../common/Client/Client.h"
#include <thread>


int main(int argc, char* argv[])
{
    std::this_thread::sleep_for(1000ms);
    shared_ptr<Client> cl = make_shared<Client>("A");
    cl->Connect();
    cl->CreateTopic(szEmergencyTopic);
    std::this_thread::sleep_for(1000ms);

    string szToSend;
    int nStringCounter = 0;
    int nLastSeenStringCounter = 0;

    thread keyReadThread([&szToSend, &nStringCounter]{
        while(true)
        {
            char buffer[256];
            cin.getline( buffer, 256 );
            szToSend = string(buffer);
            // how to notify the main thread that we got a new string?
            nStringCounter++;
            if( szToSend == "q" || szToSend == "quit" )
            {
                break;
            }
        }

        cout << "Exiting keyboard read thread" << endl;
    });

    string szCurrentTopic = szEmergencyTopic;

    while (true)
    {
        bool bOk = cl->process_some_io();
        if (!bOk) break;

        if( nLastSeenStringCounter != nStringCounter )
        {
            // send the string the user input on the background thread
            if( szToSend[0] == '!' )
            {
                // high priority
                szToSend = szToSend.substr(1);
                cl->SendMessage(szCurrentTopic, szToSend, '1' );
            }
            else
            {
                cl->SendMessage(szCurrentTopic, szToSend, '0' );
            }
            nLastSeenStringCounter = nStringCounter;
        }

        std::this_thread::sleep_for(100ms);
    }

    cl->Shutdown();
    return 0;
}