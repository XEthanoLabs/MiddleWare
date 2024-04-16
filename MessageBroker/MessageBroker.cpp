#include "MessageBroker.h"

MessageBroker::MessageBroker()
{
    // just boost basics stuff. Can't use socket stuff w/o an IoService to do some gruntwork
    m_pIoService = new io_service();
    m_pAcceptor = new tcp::acceptor(*m_pIoService, tcp::endpoint(tcp::v4(), 8087));
    // call this before run() below
    AsyncAllowClientToJoin();
}

void MessageBroker::AsyncAllowClientToJoin()
{
    m_pAcceptor->async_accept([this](boost::system::error_code ec, tcp::socket sk)
        {
            if (!ec)
            {
                cout << "creating a socket on: "
                    << sk.remote_endpoint().address().to_string()
                    << ":" << sk.remote_endpoint().port() << "\n";

                // create a pointer to a session. We pass in the (new) socket.
                // The session will store the socket for us which maintains the connection.
                // The 'shared_ptr" thing makes it so that the allocation of the session (normally, calling 'new')
                // stays around until it needs to. Kind of like magic.
                shared_ptr<ConnectedClient> pCC = make_shared<ConnectedClient>(sk, this); // you HAVE to use move
                this->m_Clients.push_back(pCC);


                // go process all calls to this socket until the socket it closed.
                // This is implicit multithreading?
                pCC->QueueReceiveCallback();
            }
            else
            {
                cout << "error: " << ec << endl;
            }

            // since we just accepted one (or failed one), we want other clients to connect,
            // so we'll call this again
            AsyncAllowClientToJoin();
        });

    // this happens even if the socket hasn't accepted yet. falls through and keeps going. 
    // This allows the main constructor to go on and do more stuff.
}

void MessageBroker::SendMessagesToClientsLoop()
{
    // m_pIoService->run();

    // run until there is nothing left to do and the service is shut down
    while (true)
    {
        // are there any incoming requests? Service at most one of them. This will process an incoming connect
        // or message send
        //
        io_context::count_type count = m_pIoService->poll_one();

        // now go service our queues
        char chSendPriority = '0' + 10; // 10 levels max?
        bool bAnyLesser = false;
        while( true )
        {
            bool bSentSomething = false;

            for (list<TopicChat*>::iterator tr = m_Topics.begin(); tr != m_Topics.end(); tr++)
            {
                TopicChat* trp = *tr;
                bAnyLesser |= trp->SendMessagesOfPriority(chSendPriority, bSentSomething);
            }
            if( !bSentSomething )
            {
                if( chSendPriority == '0' )
                {
                    break; // all done
                }

                // where there any at a lower priority?
                if( !bAnyLesser )
                {
                    break; // nope. all done.
                }

                // yep. Lower the priority and try some more
                chSendPriority--;
                bAnyLesser = false;
            }
        }
    }

    delete m_pAcceptor;
    delete m_pIoService;
}

void MessageBroker::IncomingSetClientName(string& szClient, tcp::socket& s )
{
    for (shared_ptr<ConnectedClient> cc : m_Clients)
    {
        if (cc->m_socket.native_handle() == s.native_handle() )
        {
            // found it
            cc->m_szClientName = szClient;
            break;
        }
    }
}

void MessageBroker::IncomingCreateTopic(string& szClient, string& szTopicName)
{
    TopicChat* pTopic = new TopicChat(szTopicName);
    m_Topics.push_back(pTopic);

    MessageAndPriority msg;
    msg.From = "Server";
    msg.Topic = szTopicName;
    msg.Priority = '0';
    msg.Text = "Client " + szClient + " created topic.";
    pTopic->AddMessageToSend( msg );
}

// if the topic doesn't exist?

TopicChat* MessageBroker::FindTopicRoom(string& szTopicName)
{
    for (TopicChat* ptr : m_Topics)
    {
        if (ptr->m_szTopic == szTopicName)
        {
            return ptr;
        }
    }
    return nullptr;
}

// immediately subscribe to a topic. Don't have a priority for this

void MessageBroker::IncomingSubscribeTopic(string& szClient, string& szTopicName)
{
    shared_ptr<ConnectedClient>& pCC = GetConnectedClientFromName(szClient);
    if (pCC == nullptr)
    {
        return;
    }

    TopicChat* ptr = FindTopicRoom(szTopicName);
    if (ptr == nullptr)
    {
        string sz = "Server: The topic " + szTopicName + " doesn't exist.";
        pCC->Write(sz);

        return;
    }

    ptr->AddClient(pCC);
}

void MessageBroker::IncomingUnsubscribeTopic(string& szClient, string& szTopicName)
{
    shared_ptr<ConnectedClient>& pCC = GetConnectedClientFromName(szClient);
    if (pCC == nullptr)
    {
        return;
    }

    TopicChat* ptr = FindTopicRoom(szTopicName);
    if (ptr == nullptr)
    {
        string sz = "Server: The topic " + szTopicName + " doesn't exist.";
        pCC->Write(sz);

        return;
    }

    ptr->RemoveClient(szClient);
}

// add in coming message to list of messages to send out. We'll send it when we have time

void MessageBroker::IncomingMessage(char chPriority, string& szClient, string& szTopicName, string& szMesg)
{
    TopicChat* ptr = FindTopicRoom(szTopicName);
    if (ptr == nullptr)
    {
        return;
    }

    MessageAndPriority mp;
    mp.From = szClient;
    mp.Topic = szTopicName;
    mp.Priority = chPriority;
    mp.Text = szMesg;
    ptr->AddMessageToSend(mp);
}

shared_ptr<ConnectedClient>& MessageBroker::GetConnectedClientFromName(string& szClient)
{
    for (shared_ptr<ConnectedClient>& cc : m_Clients)
    {
        if (cc->m_szClientName == szClient)
        {
            return cc;
        }
    }
    throw std::invalid_argument("No matching connecting client");
}

shared_ptr<ConnectedClient>& MessageBroker::GetConnectedClientFromSocket(tcp::socket& socket)
{
    for (shared_ptr<ConnectedClient>& cc : m_Clients)
    {
        if (cc->m_socket.native_handle() == socket.native_handle())
        {
            return cc;
        }
    }
    throw std::invalid_argument("No matching connecting client");
}

void MessageBroker::_ProcessSingleCommand(string& szSingleLine, tcp::socket& socket)
{
    // parse what the incoming command wanted to do.
    list<string> szPieces;
    SplitString(szSingleLine, '|', szPieces);
    string szClient = szPieces.front();
    szPieces.pop_front();
    string szCommand = szPieces.front();
    szPieces.pop_front();

    cout << "server got command from client " + szClient + ", command = " + szCommand << endl;

    if (szCommand == "CONNECT")
    {
        IncomingSetClientName(szClient, socket);
    }

    if (szCommand == "CREATE")
    {
        // create a new topic. other clients can now subscribe to it
        string szTopicName = szPieces.front();
        IncomingCreateTopic(szClient, szTopicName);
    }
    if (szCommand == "SUBSCRIBE")
    {
        string szTopicName = szPieces.front();
        IncomingSubscribeTopic(szClient, szTopicName);
    }
    if( szCommand == "UNSUBSCRIBE")
    {
        string szTopicName = szPieces.front();
        IncomingUnsubscribeTopic( szClient, szTopicName);
    }
    if (szCommand == "MESG")
    {
        string szPriority = szPieces.front();
        szPieces.pop_front();
        char chPriority = szPriority[0];
        string szTopic = szPieces.front();
        szPieces.pop_front();
        string szMesg = szPieces.front();
        IncomingMessage(chPriority, szClient, szTopic, szMesg);
    }
}

void MessageBroker::OnData(string& szMesg, tcp::socket& socket)
{
    while (szMesg.size())
    {
        string szFirst;
        PullOutStringUntilDelimiter(szMesg, '\n', szFirst);
        if (szFirst.empty())
        {
            szFirst = szMesg;
        }
        if (szFirst.empty())
        {
            break;
        }
        _ProcessSingleCommand(szFirst, socket);
    }
}

void MessageBroker::OnSocketClose(tcp::socket& socket)
{
    // the remote client lost their socket. 
    shared_ptr<ConnectedClient>& pCC = GetConnectedClientFromSocket(socket);

    cout << "Server lost connection to client " << pCC->m_szClientName << endl;
    
    // find all topic rooms that have that client in them, and remove the client.
    // If the topic room has no more participants, remove it
    for (list<TopicChat*>::iterator trp = m_Topics.begin() ; trp != m_Topics.end() ; trp++ )
    {
        cout << "Removing client from topic " << (*trp)->m_szTopic << endl;
        (*trp)->RemoveClient(pCC->m_szClientName);
        if (!(*trp)->HasAnyParticipants())
        {
            cout << "Topic " + (*trp)->m_szTopic + " has no more clients. Removing." << endl;

            trp = m_Topics.erase(trp);
            if (trp == m_Topics.end())
            {
                break;
            }
        }
    }

    // remove the connected client
    for (list<shared_ptr<ConnectedClient>>::iterator pcc = m_Clients.begin(); pcc != m_Clients.end(); pcc++)
    {
        if (*pcc == pCC)
        {
            // same pointer
            pcc = m_Clients.erase(pcc);
            if (pcc == m_Clients.end())
            {
                cout << "Removing client from m_Clients list" << endl;
                break;
            }
        }
    }
}

