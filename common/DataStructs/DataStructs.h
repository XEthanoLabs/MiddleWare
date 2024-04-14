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
    string Text;
    char Priority;
};

