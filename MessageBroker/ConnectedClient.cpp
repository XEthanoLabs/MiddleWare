#include "ConnectedClient.h"
#include "MessageBroker.h"

void ConnectedClient::async_wait_for_request()
{
    // since we capture `this` in the callback, we need to call shared_from_this()
    auto self(shared_from_this());

    // since we capture `this` in the callback, we need to call shared_from_this()
    // and now call the lambda once data arrives
    // we read a string until the null termination character
    boost::asio::async_read_until(m_socket, m_buffer, "\n",
        [this, self](boost::system::error_code ec, size_t length)
        {
            if (!ec)
            {
                // convert the data into a string
                string data
                {
                    istreambuf_iterator<char>(&m_buffer),
                    istreambuf_iterator<char>()
                };

                // tell the server we received something
                m_pServer->OnData(data, m_socket);

                // go wait for the next thing. The 'self' thing keeps a reference count alive on us,
                // so we don't die.

                async_wait_for_request();
            }
            else
            {
                cout << "server: error or EOS: " << ec << endl;
                m_socket.close();
                m_pServer->OnSocketClose(m_socket);
            }
        });
}
