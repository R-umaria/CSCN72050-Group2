#include "MySocket.h"

// Constructor
MySocket::MySocket(SocketType sType, std::string ip, unsigned int port, ConnectionType cType, unsigned int size)
    : mySocket(sType), connectionType(cType), IPAddr(ip), Port(port), bTCPConnect(false)
{
    if (size > 0)
        MaxSize = size;
    else
        MaxSize = DEFAULT_SIZE;

    // Allocate and initialize the buffer.
    Buffer = new char[MaxSize];
    std::memset(Buffer, 0, MaxSize);

    // Set up SvrAddr.
    std::memset(&SvrAddr, 0, sizeof(SvrAddr));
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(Port);
    SvrAddr.sin_addr.s_addr = inet_addr(IPAddr.c_str());

    // Determine connection type.
    if (cType == TCP) {
        if (sType == SERVER) {
            WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (WelcomeSocket == INVALID_SOCKET) {
                std::cerr << "TCP Server socket creation failed: " << strerror(errno) << std::endl;
                exit(1);
            }
            if (bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
                std::cerr << "TCP Server bind failed: " << strerror(errno) << std::endl;
                close(WelcomeSocket);
                exit(1);
            }
            if (listen(WelcomeSocket, 1) == SOCKET_ERROR) {
                std::cerr << "TCP Server listen failed: " << strerror(errno) << std::endl;
                close(WelcomeSocket);
                exit(1);
            }
            std::cout << "TCP Server initialized and listening at IP: " << IPAddr << ", Port: " << Port << std::endl;
        }
        else if (sType == CLIENT) {
            ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (ConnectionSocket == INVALID_SOCKET) {
                std::cerr << "TCP Client socket creation failed: " << strerror(errno) << std::endl;
                exit(1);
            }
            std::cout << "TCP Client initialized for server at IP: " << IPAddr << ", Port: " << Port << std::endl;
        }
    }
    else if (cType == UDP) {
        if (sType == SERVER) {
            ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (ConnectionSocket == INVALID_SOCKET) {
                std::cerr << "UDP Server socket creation failed: " << strerror(errno) << std::endl;
                exit(1);
            }
            if (bind(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
                std::cerr << "UDP Server bind failed: " << strerror(errno) << std::endl;
                close(ConnectionSocket);
                exit(1);
            }
            std::cout << "UDP Server initialized at IP: " << IPAddr << ", Port: " << Port << std::endl;
        }
        else if (sType == CLIENT) {
            ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (ConnectionSocket == INVALID_SOCKET) {
                std::cerr << "UDP Client socket creation failed: " << strerror(errno) << std::endl;
                exit(1);
            }
            std::cout << "UDP Client initialized for server at IP: " << IPAddr << ", Port: " << Port << std::endl;
        }
    }
}

// Destructor
MySocket::~MySocket() {
    // If TCP:
    if (connectionType == TCP) {
        if (bTCPConnect) {
            DisconnectTCP();
        }
        if (mySocket == SERVER && WelcomeSocket != INVALID_SOCKET) {
            close(WelcomeSocket);
        }
    }
    if (ConnectionSocket != INVALID_SOCKET) {
        close(ConnectionSocket);
    }
    delete[] Buffer;
}

// Establishes a TCP connection.
void MySocket::ConnectTCP() {
    if (connectionType != TCP) {
        std::cerr << "Cannot connect non-TCP socket" << std::endl;
        return;
    }
    if (bTCPConnect) {
        std::cerr << "TCP already connected" << std::endl;
        return;
    }

    if (mySocket == CLIENT) {
        if (connect(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
            std::cerr << "TCP Client connection failed: " << strerror(errno) << std::endl;
            close(ConnectionSocket);
            return;
        }
        bTCPConnect = true;
        std::cout << "TCP Client successfully connected to server." << std::endl;
    }
    else if (mySocket == SERVER) {
        socklen_t addrlen = sizeof(SvrAddr);
        ConnectionSocket = accept(WelcomeSocket, (struct sockaddr*)&SvrAddr, &addrlen);
        if (ConnectionSocket == INVALID_SOCKET) {
            std::cerr << "TCP Server accept failed: " << strerror(errno) << std::endl;
            return;
        }
        bTCPConnect = true;
        std::cout << "TCP Server accepted client connection" << std::endl;
    }
}

// Disconnects an established TCP connection.
void MySocket::DisconnectTCP() {
    if (connectionType != TCP) {
        std::cerr << "Cannot disconnect non-TCP socket" << std::endl;
        return;
    }
    if (!bTCPConnect) {
        std::cerr << "No TCP connection to disconnect" << std::endl;
        return;
    }
    shutdown(ConnectionSocket, SHUT_RDWR);
    close(ConnectionSocket);
    ConnectionSocket = INVALID_SOCKET;
    bTCPConnect = false;
}

// Sends raw data over the socket.
void MySocket::SendData(const char* rawData, int bytes) {
    if (rawData == nullptr) {
        std::cerr << "SendData error: rawData is nullptr." << std::endl;
        return;
    }
    if (bytes > MaxSize) {
        std::cerr << "SendData error: bytes (" << bytes << ") > MaxSize (" << MaxSize << ")." << std::endl;
        return;
    }
    if (bytes <= 0) {
        std::cerr << "SendData error: bytes is non-positive (" << bytes << ")." << std::endl;
        return;
    }
    
    int bytesSent = 0;
    if (connectionType == TCP) {
        if (!bTCPConnect) {
            std::cerr << "No TCP connection established for sending data" << std::endl;
            return;
        }
        bytesSent = send(ConnectionSocket, rawData, bytes, 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "TCP send failure: " << strerror(errno) << std::endl;
            return;
        }
    }
    else if (connectionType == UDP) {
        bytesSent = sendto(ConnectionSocket, rawData, bytes, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "UDP send failure: " << strerror(errno) << std::endl;
            return;
        }
    }
    std::cout << "Sent " << bytesSent << " bytes of data" << std::endl;
}

// Sets the IP address (if not connected).
void MySocket::SetIPAddr(std::string ip) {
    if (bTCPConnect || (mySocket == SERVER && connectionType == TCP && WelcomeSocket != INVALID_SOCKET)) {
        std::cerr << "Cannot change IP address; connection already established" << std::endl;
        return;
    }
    unsigned long addr = inet_addr(ip.c_str());
    if (addr == INADDR_NONE) {
        std::cerr << "Invalid IP address provided" << std::endl;
        return;
    }
    IPAddr = ip;
    SvrAddr.sin_addr.s_addr = inet_addr(IPAddr.c_str());
    std::cout << "IP address changed to: " << IPAddr << std::endl;
}

// Sets the port number (if not connected).
void MySocket::SetPort(int port) {
    if (bTCPConnect || (mySocket == SERVER && connectionType == TCP && WelcomeSocket != INVALID_SOCKET)) {
        std::cerr << "Cannot change port; connection already established" << std::endl;
        return;
    }
    Port = port;
    SvrAddr.sin_port = htons(Port);
    std::cout << "Port changed to: " << Port << std::endl;
}

// Sets the socket type (if not connected).
void MySocket::SetType(SocketType type) {
    if (bTCPConnect || (mySocket == SERVER && connectionType == TCP && WelcomeSocket != INVALID_SOCKET)) {
        std::cerr << "Cannot change socket type; connection already established" << std::endl;
        return;
    }
    mySocket = type;
    std::cout << "Socket type changed." << std::endl;
}

// Getter for port.
int MySocket::GetPort() {
    return Port;
}

// Getter for socket type.
SocketType MySocket::GetType() {
    return mySocket;
}

// Getter for IP address.
std::string MySocket::GetIPAddr() {
    return IPAddr;
}

// Receives data into the provided buffer.
int MySocket::GetData(char* rawData) {
    std::memset(Buffer, 0, MaxSize);
    int bytesReceived = 0;
    if (connectionType == TCP) {
        if (!bTCPConnect) {
            std::cerr << "No TCP connection established for receiving data" << std::endl;
            return -1;
        }
        bytesReceived = recv(ConnectionSocket, Buffer, MaxSize, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "TCP receive failure: " << strerror(errno) << std::endl;
            return -1;
        }
    }
    else if (connectionType == UDP) {
        socklen_t addrlen = sizeof(SvrAddr);
        bytesReceived = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr*)&SvrAddr, &addrlen);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "UDP receive failed: " << strerror(errno) << std::endl;
            return -1;
        }
    }
    std::memcpy(rawData, Buffer, bytesReceived);
    std::cout << "Received " << bytesReceived << " bytes of data." << std::endl;
    return bytesReceived;
}

// --- New functions for Milestone3 ---

// Configures the socket by updating the IP address and port.
// Returns false if already connected.
bool MySocket::configure(const std::string& ip, int port) {
    if (bTCPConnect) {
        std::cerr << "Socket already connected." << std::endl;
        return false;
    }
    SetIPAddr(ip);
    SetPort(port);
    return true;
}

// Sends a complete packet (as a string) over the socket.
bool MySocket::sendPacket(const std::string& packet) {
    SendData(packet.c_str(), packet.size());
    // In a more robust implementation, check the return value.
    return true;
}

// Receives data from the socket and returns it as a string.
std::string MySocket::receiveResponse() {
    char recvBuffer[DEFAULT_SIZE];
    int bytesReceived = GetData(recvBuffer);
    if (bytesReceived > 0) {
        return std::string(recvBuffer, bytesReceived);
    }
    return "";
}

int MySocket::GetUDPSocket() {
    return ConnectionSocket;
}