#include "../common/Client/Client.h"

int main(int argc, char* argv[])
{
    Sleep(1000);
    Client c("C");
    c.Connect();
    Sleep(10000);
    c.Shutdown();
    return 0;
}