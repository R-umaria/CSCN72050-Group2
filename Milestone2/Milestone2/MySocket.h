// Rishi Umaria & Tyler Phillips - Group Project - Milestone 2 - Mobile & Networked Systems

#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <cstring>
#include <iostream>

//define socket and connection types
enum SocketType { CLIENT, SERVER };
enum ConnectionType { TCP, UDP };

const int DEFAULT_SIZE = 1024;  //default buffer size

// Define SOCKET type
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)

//complete socket class
class MySocket {
private:
    char* Buffer;                    // dynamically allocated raw buffer for communication
    SOCKET WelcomeSocket;            // used by MySocket objects configured as TCP servers
    SOCKET ConnectionSocket;         // used for client/server communications (TCP & UDP)
    struct sockaddr_in SvrAddr;      // stores connection information
    SocketType mySocket;             // socket mode (CLIENT or SERVER)
    std::string IPAddr;              // IPv4 address string
    int Port;                        // port number to be used
    ConnectionType connectionType;   // transport protocol (TCP or UDP)
    bool bTCPConnect;                // flag to indicate if a TCP connection is established
    int MaxSize;                     // max bytes allocated to Buffer

public:
    //constructor initializes the socket with given parameters
    MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int);
    //destructor
    ~MySocket();

    //member Functions
    void ConnectTCP();              // establishes a TCP connection (3-way handshake)
    void DisconnectTCP();           // disconnects an established TCP connection (4-way handshake)
    void SendData(const char*, int); // sends raw data (works with both TCP and UDP)

    //setters
    void SetIPAddr(std::string);
    void SetPort(int);
    void SetType(SocketType);

    //getters
    int GetPort();
    SocketType GetType();
    std::string GetIPAddr();
    int GetData(char*);             // receives data into provided buffer and returns bytes received

    //return status of UDP socket
    int GetUDPSocket();
};

#endif // MYSOCKET_H
