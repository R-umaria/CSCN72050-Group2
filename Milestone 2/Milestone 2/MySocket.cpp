#include "MySocket.h"

// Define static variable for instance counting.
int MySocket::instanceCount = 0;

MySocket::MySocket(SocketType type, std::string ip, unsigned int port, ConnectionType connection, unsigned int size)
    : mySocket(type), IPAddr(ip), Port(port), connectionType(connection), bTCPConnect(false)
{
    // Initialize Winsock only once per application.
    if (instanceCount == 0) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    instanceCount++;

    // Determine buffer size.
    if (size > 0)
        MaxSize = size;
    else
        MaxSize = DEFAULT_SIZE;
    Buffer = new char[MaxSize];

    // Initialize socket(s) based on type and connection.
    if (connectionType == TCP) {
        if (mySocket == SERVER) {
            // Create a TCP listening (welcome) socket.
            WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (WelcomeSocket == INVALID_SOCKET) {
                std::cerr << "Failed to create welcome socket" << std::endl;
                exit(EXIT_FAILURE);
            }
            // Setup server address.
            SvrAddr.sin_family = AF_INET;
            if (InetPtonA(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr) != 1) {
                std::cerr << "Invalid IP address format: " << IPAddr << std::endl;
                exit(EXIT_FAILURE);
            }
            SvrAddr.sin_port = htons(Port);

            // Bind the welcome socket.
            if (bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
                std::cerr << "Bind failed" << std::endl;
                closesocket(WelcomeSocket);
                exit(EXIT_FAILURE);
            }

            // Listen for incoming connections.
            if (listen(WelcomeSocket, SOMAXCONN) == SOCKET_ERROR) {
                std::cerr << "Listen failed" << std::endl;
                closesocket(WelcomeSocket);
                exit(EXIT_FAILURE);
            }
            // For a server, ConnectionSocket will be set in ConnectTCP() when a client connects.
        }
        else { // TCP Client.
            // Create a TCP socket for the client; connection happens later in ConnectTCP().
            ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (ConnectionSocket == INVALID_SOCKET) {
                std::cerr << "Failed to create client socket" << std::endl;
                exit(EXIT_FAILURE);
            }
            // Prepare SvrAddr for later use in connect().
            SvrAddr.sin_family = AF_INET;
            if (InetPtonA(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr) != 1) {
                std::cerr << "Invalid IP address format: " << IPAddr << std::endl;
                exit(EXIT_FAILURE);
            }
            SvrAddr.sin_port = htons(Port);
        }
    }
    else { // UDP.
        // Create a UDP socket (for both CLIENT and SERVER).
        ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (ConnectionSocket == INVALID_SOCKET) {
            std::cerr << "Failed to create UDP socket" << std::endl;
            exit(EXIT_FAILURE);
        }
        // Setup server address for UDP transmissions.
        SvrAddr.sin_family = AF_INET;
        if (InetPtonA(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr) != 1) {
            std::cerr << "Invalid IP address format: " << IPAddr << std::endl;
            exit(EXIT_FAILURE);
        }
        SvrAddr.sin_port = htons(Port);

        // If in SERVER mode, bind the UDP socket so it can receive data.
        if (mySocket == SERVER) {
            if (bind(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
                std::cerr << "UDP Bind failed" << std::endl;
                closesocket(ConnectionSocket);
                exit(EXIT_FAILURE);
            }
        }
    }
}

MySocket::~MySocket() {
    // Clean up buffer.
    delete[] Buffer;

    // Close sockets if open.
    if (connectionType == TCP) {
        if (mySocket == SERVER && WelcomeSocket != INVALID_SOCKET) {
            closesocket(WelcomeSocket);
        }
        if (bTCPConnect && ConnectionSocket != INVALID_SOCKET) {
            closesocket(ConnectionSocket);
        }
    }
    else {  // UDP socket.
        if (ConnectionSocket != INVALID_SOCKET) {
            closesocket(ConnectionSocket);
        }
    }

    // Decrement instance count and cleanup Winsock if this was the last instance.
    instanceCount--;
    if (instanceCount == 0) {
        WSACleanup();
    }
}

void MySocket::ConnectTCP() {
    if (connectionType != TCP) {
        std::cerr << "ConnectTCP called on a non-TCP socket" << std::endl;
        return;
    }
    if (bTCPConnect) {
        std::cerr << "TCP Connection already established" << std::endl;
        return;
    }
    if (mySocket == CLIENT) {
        // For client, attempt to connect to server.
        if (connect(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
            std::cerr << "Connect failed" << std::endl;
            return;
        }
        bTCPConnect = true;
    }
    else if (mySocket == SERVER) {
        // For server, accept an incoming connection on WelcomeSocket.
        int addrLen = sizeof(SvrAddr);
        ConnectionSocket = accept(WelcomeSocket, (struct sockaddr*)&SvrAddr, &addrLen);
        if (ConnectionSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed" << std::endl;
            return;
        }
        bTCPConnect = true;
    }
}

void MySocket::DisconnectTCP() {
    if (connectionType != TCP) {
        std::cerr << "DisconnectTCP called on a non-TCP socket" << std::endl;
        return;
    }
    if (!bTCPConnect) {
        std::cerr << "No TCP connection to disconnect" << std::endl;
        return;
    }
    // Gracefully shutdown the connection.
    shutdown(ConnectionSocket, SD_BOTH);
    closesocket(ConnectionSocket);
    ConnectionSocket = INVALID_SOCKET;
    bTCPConnect = false;
}

void MySocket::SendData(const char* rawData, int bytes) {
    if (bytes > MaxSize) {
        std::cerr << "Data size exceeds allocated buffer" << std::endl;
        return;
    }
    if (connectionType == TCP) {
        if (!bTCPConnect) {
            std::cerr << "TCP connection not established. Cannot send data." << std::endl;
            return;
        }
        int sent = send(ConnectionSocket, rawData, bytes, 0);
        if (sent == SOCKET_ERROR)
            std::cerr << "TCP Send failed" << std::endl;
    }
    else { // UDP.
        int sent = sendto(ConnectionSocket, rawData, bytes, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
        if (sent == SOCKET_ERROR)
            std::cerr << "UDP Send failed" << std::endl;
    }
}

int MySocket::GetData(char* rawData) {
    int received = 0;
    if (connectionType == TCP) {
        if (!bTCPConnect) {
            std::cerr << "TCP connection not established. Cannot receive data." << std::endl;
            return -1;
        }
        received = recv(ConnectionSocket, Buffer, MaxSize, 0);
        if (received == SOCKET_ERROR) {
            std::cerr << "TCP Receive failed" << std::endl;
            return -1;
        }
    }
    else { // UDP receive.
        int addrLen = sizeof(SvrAddr);
        received = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr*)&SvrAddr, &addrLen);
        if (received == SOCKET_ERROR) {
            std::cerr << "UDP Receive failed" << std::endl;
            return -1;
        }
    }
    memcpy(rawData, Buffer, received);
    return received;
}

void MySocket::SetIPAddr(std::string ip) {
    if (bTCPConnect || (mySocket == SERVER && WelcomeSocket != INVALID_SOCKET)) {
        std::cerr << "Cannot change IP address when connection is active" << std::endl;
        return;
    }
    struct in_addr tempAddr;
    if (InetPtonA(AF_INET, ip.c_str(), &tempAddr) != 1) {
        std::cerr << "Invalid IP address format: " << ip << std::endl;
        return;
    }
    IPAddr = ip;
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_addr = tempAddr;
}

void MySocket::SetPort(int port) {
    if (bTCPConnect || (mySocket == SERVER && WelcomeSocket != INVALID_SOCKET)) {
        std::cerr << "Cannot change port number when connection is active" << std::endl;
        return;
    }
    Port = port;
    SvrAddr.sin_port = htons(Port);
}

void MySocket::SetType(SocketType type) {
    if (bTCPConnect || (mySocket == SERVER && WelcomeSocket != INVALID_SOCKET)) {
        std::cerr << "Cannot change socket type when connection is active" << std::endl;
        return;
    }
    mySocket = type;
}

int MySocket::GetPort() {
    return Port;
}

SocketType MySocket::GetType() {
    return mySocket;
}

std::string MySocket::GetIPAddr() {
    return IPAddr;
}
