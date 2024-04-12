#pragma once
#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <list>

#include "../common/DataStructs/DataStructs.h"
#include "session.h"

using namespace std;
using namespace boost::asio;
using ip::tcp;

// all this class does it associate a client name with a connected session (socket)
class ConnectedClient
{
public:

	shared_ptr<session> m_pSession;
	string m_szClientName;

	ConnectedClient(shared_ptr<session> pSession, string szClientName)
	{
		m_pSession = pSession;
		m_szClientName = szClientName;
	}

	~ConnectedClient()
	{
	}
};

