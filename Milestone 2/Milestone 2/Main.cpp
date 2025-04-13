#include <iostream>
#include "MySocket.h"

int main() {
    // For instance, create a TCP client MySocket and attempt a connection:
    MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);

    std::cout << "Socket created with IP: " << socket.GetIPAddr() << " and Port: " << socket.GetPort() << std::endl;
    // For more thorough testing, call ConnectTCP(), SendData(), etc. here.

    return 0;
}
