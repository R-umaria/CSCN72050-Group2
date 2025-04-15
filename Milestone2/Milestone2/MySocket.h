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

// Define types and constants for Linux
enum SocketType { CLIENT, SERVER };
enum ConnectionType { TCP, UDP };

const int DEFAULT_SIZE = 1024;  // default buffer size

// Define SOCKET type and error constants for Linux.
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)

class MySocket {
private:
    char* Buffer;                    // dynamically allocated raw buffer for communication
    SOCKET WelcomeSocket;            // used by MySocket objects configured as TCP servers
    SOCKET ConnectionSocket;         // used for client/server communications (TCP & UDP)
    struct sockaddr_in SvrAddr;      // stores connection information
    SocketType mySocket;             // socket mode (CLIENT or SERVER)
    std::string IPAddr;              // IPv4 address string
    int Port;                      // port number to be used
    ConnectionType connectionType;   // transport protocol (TCP or UDP)
    bool bTCPConnect;                // flag to indicate if a TCP connection is established
    int MaxSize;                     // max bytes allocated to Buffer

public:
    // Constructor: initializes the socket with provided parameters.
    MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int);
    // Destructor: cleans up allocated memory.
    ~MySocket();

    // Member Functions:
    void ConnectTCP();              // establishes a TCP connection (3-way handshake)
    void DisconnectTCP();           // disconnects an established TCP connection (4-way handshake)
    void SendData(const char*, int); // sends raw data (works with both TCP and UDP)

    // Setters: (prevent changes if a connection is already established)
    void SetIPAddr(std::string);
    void SetPort(int);
    void SetType(SocketType);

    // Getters:
    int GetPort();
    SocketType GetType();
    std::string GetIPAddr();
    int GetData(char*);             // receives data into provided buffer and returns bytes received

    // Additional convenience functions (used in Milestone3):
    // Configures the socket by updating IP address and port (if not connected).
    bool configure(const std::string& ip, int port);
    // Sends a complete packet provided as a std::string.
    bool sendPacket(const std::string& packet);
    // Receives data and returns it as a std::string.
    std::string receiveResponse();
    int GetUDPSocket();
};

#endif // MYSOCKET_H
