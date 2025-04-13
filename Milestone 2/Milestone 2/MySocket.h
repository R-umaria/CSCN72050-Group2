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
    char* Buffer;               // dynamically allocated raw buffer space
    SOCKET WelcomeSocket;       // used by MySocket configured as TCP/IP Server
    SOCKET ConnectionSocket;    // used for client/server communications (TCP & UDP)
    struct sockaddr_in SvrAddr; // store connection information
    SocketType mySocket;        // type of socket (CLIENT/SERVER)
    std::string IPAddr;         // IPv4 IP Address string
    int Port;                   // Port number to be used
    ConnectionType connectionType; // TCP or UDP
    bool bTCPConnect;           // flag to determine if a connection is established
    int MaxSize;                // max bytes allocated to Buffer

public:
    // Constructor/Destructor
    MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int);
    ~MySocket();

    // Member Functions
    void ConnectTCP();
    void DisconnectTCP();
    void SendData(const char*, int);

    // Setters (with connection-state checks)
    void SetIPAddr(std::string);
    void SetPort(int);
    void SetType(SocketType);

    // Getters
    int GetPort();
    SocketType GetType();
    std::string GetIPAddr();
    int GetData(char*);
};

#endif
