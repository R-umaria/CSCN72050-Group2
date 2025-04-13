#include <iostream>
#include "MySocket.h"

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    // For instance, create a TCP client MySocket and attempt a connection:
    MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
    MySocket socket2(SERVER, "127.0.0.1", 8080, TCP, 1024);

    std::cout << "Socket created with IP: " << socket.GetIPAddr() << " and Port: " << socket.GetPort() << std::endl;
    std::cout << "Socket created with IP: " << socket2.GetIPAddr() << " and Port: " << socket2.GetPort() << std::endl;
    // For more thorough testing, call ConnectTCP(), SendData(), etc. here.

    return 0;
}
