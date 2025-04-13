#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

enum SocketType { CLIENT, SERVER };
enum ConnectionType { TCP, UDP };

const int DEFAULT_SIZE = 1024;  // Default buffer size

class MySocket {
private:
    char* Buffer;               // dynamically allocated raw buffer space for communication activities
    SOCKET WelcomeSocket;       // used by MySocket configured as TCP/IP Server
    SOCKET ConnectionSocket;    // used for client/server communications (TCP & UDP)
    struct sockaddr_in SvrAddr; // store connection information
    SocketType mySocket;        // holds type of socket (CLIENT/SERVER)
    std::string IPAddr;         // holds IPv4 IP Address string
    int Port;                   // port number to be used
    ConnectionType connectionType; // TCP or UDP
    bool bTCPConnect;           // flag to determine if a TCP connection is established
    int MaxSize;                // maximum bytes allocated to the Buffer

    // Static variable to count active socket instances.
    static int instanceCount;

public:
    MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int);
    ~MySocket();

    void ConnectTCP();
    void DisconnectTCP();
    void SendData(const char*, int);

    void SetIPAddr(std::string);
    void SetPort(int);
    void SetType(SocketType);

    int GetPort();
    SocketType GetType();
    std::string GetIPAddr();
    int GetData(char*);
};

#endif
