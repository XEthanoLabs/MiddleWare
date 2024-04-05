#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;
using ip::tcp;

string pszUs = "ClientA:";

void async_wait_for_request( tcp::socket& s)
{
    boost::asio::streambuf m_buffer;

    boost::asio::async_read_until(s, m_buffer, "\n",
        [&s, &m_buffer](boost::system::error_code ec, size_t length)
        {
            cout << "async_read_until completed " << ec << " " << length << endl;

            // if there was no error, everything went well and for this demo
            // we print the data to stdout and wait for the next request
            if (!ec)
            {
                string data
                {
                    istreambuf_iterator<char>(&m_buffer),
                    istreambuf_iterator<char>()
                };
                // we just print the data, you can here call other api's 
                // or whatever the server needs to do with the received data
                cout << data << endl;
                async_wait_for_request(s);
            }
            else
            {
                cout << "error or EOS: " << ec << endl;
                s.close();
            }
        });
}

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
    async_wait_for_request(socket);

    // and use write(..) to send some data which is here just a string
    std::string data{ pszUs + "CREATE TheTopic\n" };
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