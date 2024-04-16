#include "../common/Client/Client.h"
#include <thread>


int main(int argc, char* argv[])
{
    std::this_thread::sleep_for(1000ms);
    shared_ptr<Client> cl = make_shared<Client>("A");
    cl->Connect();
    cl->CreateTopic(szEmergencyTopic);
    

    string szToSend;
    int nStringCounter = 0;
    int nLastSeenStringCounter = 0;

    // this is some simple parallel programming, so we can enter things to send
    // at the same time as reading and displaying what other clients could be typing.
    // It uses the above variables to detect in the main thread, when something changed
    // and a new string needs to be sent. If the other clients never type anything, and
    // we don't receive from them, this code is pointless.
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

    cout << "Press enter to begin." << endl;
    char bufferDummy[256];
    cin.getline( bufferDummy, 256 );
    

    // while A has something to type, keep sending it.

    while (true)
    {
        bool bOk = cl->process_some_io();
        if (!bOk) break;

        if( nLastSeenStringCounter != nStringCounter )
        {
            if( szToSend == "q" || szToSend == "quit" )
            {
                break; // will lose the socket connection
            }

            // process the input string to see if we've entered a subscribe topic or just a normal message to send.
            // maybe it's easy as: First time through, if we've never set the topic, we create it.
            // second time, we sent any message typed?

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