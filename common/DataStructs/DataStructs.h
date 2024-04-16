#pragma once
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <list>

using namespace std;
using namespace boost::asio;
using ip::tcp;

struct MessageAndPriority
{
    string From;
    string Topic;
    string Text;
    char Priority; // lowest priority is ASCII '0'. It goes up from there. '6' would be 6 levels higher than '0'. 

    bool operator< ( const MessageAndPriority& other )
    {
        return this->Priority < other.Priority;
    }
};

