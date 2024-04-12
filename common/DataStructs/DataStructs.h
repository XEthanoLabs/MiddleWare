#pragma once
#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <list>

using namespace std;
using namespace boost::asio;
using ip::tcp;

struct MessageAndPriority
{
    string Text;
    byte Priority;
};

