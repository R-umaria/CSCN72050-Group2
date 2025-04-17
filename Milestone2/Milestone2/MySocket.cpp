// Rishi Umaria & Tyler Phillips - Group Project - Milestone 2 - Mobile & Networked Systems

#include "MySocket.h"

//constructor
MySocket::MySocket(SocketType sType, std::string ip, unsigned int port, ConnectionType cType, unsigned int size)
    : mySocket(sType), connectionType(cType), IPAddr(ip), Port(port), bTCPConnect(false)
{
    if (size > 0)
        MaxSize = size;
    else
        MaxSize = DEFAULT_SIZE;

    //allocate and initialize buffer
    Buffer = new char[MaxSize];
    std::memset(Buffer, 0, MaxSize);

    //set up SvrAddr
    std::memset(&SvrAddr, 0, sizeof(SvrAddr));
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(Port);
    SvrAddr.sin_addr.s_addr = inet_addr(IPAddr.c_str());

    //determine connection type
    if (cType == TCP) {
        //determine socket type
        if (sType == SERVER) {
            WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (WelcomeSocket == INVALID_SOCKET) {  //if socket not created
                std::cerr << "TCP Server socket creation failed: " << strerror(errno) << std::endl;
                exit(1);
            }
            if (bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) { //if couldn't bind
                std::cerr << "TCP Server bind failed: " << strerror(errno) << std::endl;
                close(WelcomeSocket);
                exit(1);
            }
            if (listen(WelcomeSocket, 1) == SOCKET_ERROR) { //if not listening
                std::cerr << "TCP Server listen failed: " << strerror(errno) << std::endl;
                close(WelcomeSocket);
                exit(1);
            }
            std::cout << "TCP Server initialized and listening at IP: " << IPAddr << ", Port: " << Port << std::endl;
        }
        else if (sType == CLIENT) {
            ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (ConnectionSocket == INVALID_SOCKET) {   //if socket not created
                std::cerr << "TCP Client socket creation failed: " << strerror(errno) << std::endl;
                exit(1);
            }
            std::cout << "TCP Client initialized for server at IP: " << IPAddr << ", Port: " << Port << std::endl;
        }
    }
    else if (cType == UDP) {
        //determine socket type
        if (sType == SERVER) {
            ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (ConnectionSocket == INVALID_SOCKET) {   //if socket not created
                std::cerr << "UDP Server socket creation failed: " << strerror(errno) << std::endl;
                exit(1);
            }
            if (bind(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {  //if couldn't bind
                std::cerr << "UDP Server bind failed: " << strerror(errno) << std::endl;
                close(ConnectionSocket);
                exit(1);
            }
            std::cout << "UDP Server initialized at IP: " << IPAddr << ", Port: " << Port << std::endl;
        }
        else if (sType == CLIENT) {
            ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (ConnectionSocket == INVALID_SOCKET) {   //if socket not created
                std::cerr << "UDP Client socket creation failed: " << strerror(errno) << std::endl;
                exit(1);
            }
            std::cout << "UDP Client initialized for server at IP: " << IPAddr << ", Port: " << Port << std::endl;
        }
    }
}

//destructor
MySocket::~MySocket() {
    //if TCP, disconnect
    if (connectionType == TCP) {
        if (bTCPConnect) {
            DisconnectTCP();
        }
        if (mySocket == SERVER && WelcomeSocket != INVALID_SOCKET) {    //if valid server socket
            close(WelcomeSocket);
        }
    }
    if (ConnectionSocket != INVALID_SOCKET) {   //if UDP, and not invalid
        close(ConnectionSocket);
    }
    delete[] Buffer;
}

//establish TCP connection
void MySocket::ConnectTCP() {
    if (connectionType != TCP) {    //if UDP
        std::cerr << "Cannot connect non-TCP socket" << std::endl;
        return;
    }
    if (bTCPConnect) {  //ifalready connected
        std::cerr << "TCP already connected" << std::endl;
        return;
    }

    //determine socket type
    if (mySocket == CLIENT) {
        if (connect(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {   //if couldn't connect
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
        if (ConnectionSocket == INVALID_SOCKET) {   //if couldn't accept
            std::cerr << "TCP Server accept failed: " << strerror(errno) << std::endl;
            return;
        }
        bTCPConnect = true;
        std::cout << "TCP Server accepted client connection" << std::endl;
    }
}

//disconnect TCP connection
void MySocket::DisconnectTCP() {
    if (connectionType != TCP) {    //if UDP
        std::cerr << "Cannot disconnect non-TCP socket" << std::endl;
        return;
    }
    if (!bTCPConnect) { //if not connected
        std::cerr << "No TCP connection to disconnect" << std::endl;
        return;
    }
    shutdown(ConnectionSocket, SHUT_RDWR);
    close(ConnectionSocket);
    ConnectionSocket = INVALID_SOCKET;
    bTCPConnect = false;
}

//sends raw data over socket
void MySocket::SendData(const char* rawData, int bytes) {
    if (rawData == nullptr) {   //if data is empty
        std::cerr << "SendData error: rawData is nullptr." << std::endl;
        return;
    }
    if (bytes > MaxSize) {  //if bytes are larger than max size
        std::cerr << "SendData error: bytes (" << bytes << ") > MaxSize (" << MaxSize << ")." << std::endl;
        return;
    }
    if (bytes <= 0) {   //if no bytes
        std::cerr << "SendData error: bytes is non-positive (" << bytes << ")." << std::endl;
        return;
    }
    
    int bytesSent = 0;
    //determine connection type
    if (connectionType == TCP) {
        if (!bTCPConnect) { //if not connected
            std::cerr << "No TCP connection established for sending data" << std::endl;
            return;
        }
        bytesSent = send(ConnectionSocket, rawData, bytes, 0);
        if (bytesSent == SOCKET_ERROR) {    //if couldn't send
            std::cerr << "TCP send failure: " << strerror(errno) << std::endl;
            return;
        }
    }
    else if (connectionType == UDP) {
        bytesSent = sendto(ConnectionSocket, rawData, bytes, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
        if (bytesSent == SOCKET_ERROR) {    //if couldn't send
            std::cerr << "UDP send failure: " << strerror(errno) << std::endl;
            return;
        }
    }
    std::cout << "Sent " << bytesSent << " bytes of data" << std::endl;
}

//sets ip address
void MySocket::SetIPAddr(std::string ip) {
    if (bTCPConnect || (mySocket == SERVER && connectionType == TCP && WelcomeSocket != INVALID_SOCKET)) {    //if connected
        std::cerr << "Cannot change IP address; connection already established" << std::endl;
        return;
    }
    unsigned long addr = inet_addr(ip.c_str());
    if (addr == INADDR_NONE) {  //if invalid ip
        std::cerr << "Invalid IP address provided" << std::endl;
        return;
    }
    IPAddr = ip;
    SvrAddr.sin_addr.s_addr = inet_addr(IPAddr.c_str());
    std::cout << "IP address changed to: " << IPAddr << std::endl;
}

//sets port number
void MySocket::SetPort(int port) {
    if (bTCPConnect || (mySocket == SERVER && connectionType == TCP && WelcomeSocket != INVALID_SOCKET)) {  //if connected
        std::cerr << "Cannot change port; connection already established" << std::endl;
        return;
    }
    Port = port;
    SvrAddr.sin_port = htons(Port);
    std::cout << "Port changed to: " << Port << std::endl;
}

//sets socket type
void MySocket::SetType(SocketType type) {
    if (bTCPConnect || (mySocket == SERVER && connectionType == TCP && WelcomeSocket != INVALID_SOCKET)) {  //if connected
        std::cerr << "Cannot change socket type; connection already established" << std::endl;
        return;
    }
    mySocket = type;
    std::cout << "Socket type changed." << std::endl;
}

//gets port number
int MySocket::GetPort() {
    return Port;
}

//gets socket type
SocketType MySocket::GetType() {
    return mySocket;
}

//gets ip
std::string MySocket::GetIPAddr() {
    return IPAddr;
}

//receives data
int MySocket::GetData(char* rawData) {
    std::memset(Buffer, 0, MaxSize);
    int bytesReceived = 0;
    //determine connection type
    if (connectionType == TCP) {
        if (!bTCPConnect) { //if not connected
            std::cerr << "No TCP connection established for receiving data" << std::endl;
            return -1;
        }
        bytesReceived = recv(ConnectionSocket, Buffer, MaxSize, 0);
        if (bytesReceived == SOCKET_ERROR) {    //if couldn't receive
            std::cerr << "TCP receive failure: " << strerror(errno) << std::endl;
            return -1;
        }
    }
    else if (connectionType == UDP) {
        socklen_t addrlen = sizeof(SvrAddr);
        bytesReceived = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr*)&SvrAddr, &addrlen);
        if (bytesReceived == SOCKET_ERROR) {    //if couldn;t receive
            std::cerr << "UDP receive failed: " << strerror(errno) << std::endl;
            return -1;
        }
    }
    std::memcpy(rawData, Buffer, bytesReceived);
    std::cout << "Received " << bytesReceived << " bytes of data." << std::endl;
    return bytesReceived;
}

//returns status of UDP socket
int MySocket::GetUDPSocket() {
    return ConnectionSocket;
}