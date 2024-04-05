#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>

using namespace std;

string pszUs = "ClientB:";

int main(int argc, char* argv[])
{
    Sleep(1000);

    using boost::asio::ip::tcp;
    boost::asio::io_context io_context;

    // we need a socket and a resolver
    tcp::socket socket(io_context);
    tcp::resolver resolver(io_context);

    // now we can use connect(..)
    boost::asio::connect(socket, resolver.resolve("127.0.0.1", "8087"));

    // and use write(..) to send some data which is here just a string
    std::string data{ pszUs + "writing some client data ...\n" };
    auto result = boost::asio::write(socket, boost::asio::buffer(data, data.length()));
        
    // the result represents the size of the sent data
    std::cout << pszUs << "data sent: " << data.length() << '/' << result << std::endl;

    // Sleep(100000);

    // and close the connection now
    boost::system::error_code ec;

    cout << pszUs << "Shutting down socket" << endl;
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, /*ref*/ ec);
    cout << pszUs << "Shut down socket, calling close" << endl;
    socket.close();
    cout << pszUs << "closed socket" << endl;

    return 0;
}