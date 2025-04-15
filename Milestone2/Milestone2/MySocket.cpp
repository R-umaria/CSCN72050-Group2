#include "MySocket.h"

MySocket::MySocket(SocketType sType, std::string ip, unsigned int port, ConnectionType cType, unsigned int size)
	: mySocket(sType), connectionType(cType), IPAddr(ip), Port(port), bTCPConnect(false) {

	//determine size
	if (size > 0)
		MaxSize = size;
	else
		MaxSize = DEFAULT_SIZE;

	//initialize buffer
	Buffer = new char[MaxSize];
	std::memset(Buffer, 0, MaxSize);

	//set up SvrAddr
	memset(&SvrAddr, 0, sizeof(SvrAddr));
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(Port);
	SvrAddr.sin_addr.s_addr = inet_addr(IPAddr.c_str());

	//determine connection type
	if (cType == TCP) {
		//determine socket type
		if (sType == SERVER) {
			WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (WelcomeSocket == INVALID_SOCKET) {
				std::cerr << "TCP Server socket creation failed: " << WSAGetLastError() << std::endl;
				exit(1);
			}
			if (bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
				std::cerr << "TCP Server bind failed: " << WSAGetLastError() << std::endl;
				closesocket(WelcomeSocket);
				exit(1);
			}
			if (listen(WelcomeSocket, 1) == SOCKET_ERROR) {
				std::cerr << "TCP Server listen failed: " << WSAGetLastError() << std::endl;
				closesocket(WelcomeSocket);
				exit(1);
			}
			std::cout << "TCP Server initialized and listening at ip: " << IPAddr << ", port: " << Port << std::endl;
		}
		else if (sType == CLIENT) {
			ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (ConnectionSocket == INVALID_SOCKET) {
				std::cerr << "TCP Client socket creation failed: " << WSAGetLastError() << std::endl;
				exit(1);
			}
			std::cout << "TCP Client initialized for server at ip: " << IPAddr << ", port: " << Port << std::endl;
		}
	}
	else if (cType == UDP) {
		if (sType == SERVER) {
			ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (ConnectionSocket == INVALID_SOCKET) {
				std::cerr << "UDP Server bind failed: " << WSAGetLastError() << std::endl;
				exit(1);
			}
			if (bind(ConnectionSocket, (sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
				std::cerr << "UDP Server bind failed: " << WSAGetLastError() << std::endl;
				closesocket(ConnectionSocket);
				exit(1);
			}
			std::cout << "UDP Server initialized at ip: " << IPAddr << ", port: " << Port << std::endl;
		}
		else if (sType == CLIENT) {
			ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (ConnectionSocket == INVALID_SOCKET) {
				std::cerr << "UDP Client socket creation failed: " << WSAGetLastError() << std::endl;
				exit(1);
			}
			std::cout << "UDP Client initialized for server at " << IPAddr << ", port " << Port << std::endl;
		}
	}
}

MySocket::~MySocket() {
	//if TCP
	if (connectionType == TCP) {
		//if connected
		if (bTCPConnect) {
			DisconnectTCP();
		}
		//if server and not already closed
		if (mySocket == SERVER && WelcomeSocket != INVALID_SOCKET) {
			closesocket(WelcomeSocket);
		}
	}
	//if not already closed
	if (ConnectionSocket != INVALID_SOCKET) {
		closesocket(ConnectionSocket);
	}
	delete[] Buffer;
}

void MySocket::ConnectTCP() {
	//if not TCP
	if (connectionType != TCP) {
		std::cerr << "Cannot connect non-TCP socket" << std::endl;
		return;
	}
	//if already connected
	if (bTCPConnect) {
		std::cerr << "TCP already connected" << std::endl;
	}

	//determine client or server
	if (mySocket == CLIENT) {
		//connect client
		if ((connect(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
			std::cerr << "TCP Client connection failed: " << WSAGetLastError() << std::endl;
			closesocket(ConnectionSocket);
			return;
		}
		//set connected to true
		bTCPConnect = true;
		std::cout << "TCP Client successfully connected to server." << std::endl;
	}
	else if (mySocket == SERVER) {
		//accept incoming connection request
		int addrlen = sizeof(SvrAddr);
		ConnectionSocket = accept(WelcomeSocket, (sockaddr*)&SvrAddr, &addrlen);
		if (ConnectionSocket == INVALID_SOCKET) {
			std::cerr << "TCP Server accept failed: " << WSAGetLastError() << std::endl;
			return;
		}
		//set connected to true;
		bTCPConnect = true;
		std::cout << "TCP Server accepted client connection" << std::endl;
	}
}

void MySocket::DisconnectTCP() {
	//if not TCP
	if (connectionType != TCP) {
		std::cerr << "Cannot disconnect non-TCP socket" << std::endl;
		return;
	}
	//if not connected
	if (!bTCPConnect) {
		std::cerr << "Cannot disconnect socket that isn't connected" << std::endl;
		return;
	}

	//initiate 4-way handshake
	shutdown(ConnectionSocket, SD_BOTH);
	closesocket(ConnectionSocket);
	ConnectionSocket = INVALID_SOCKET;
	bTCPConnect = false;
}

void MySocket::SendData(const char* rawData, int bytes) {
	if (rawData == nullptr || bytes > MaxSize || bytes <= 0) {
		std::cerr << "Invalid data or bytes do not fit space within buffer" << std::endl;
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
			std::cerr << "TCP send failure: " << WSAGetLastError() << std::endl;
			return;
		}
	}
	else if (connectionType == UDP) {
		bytesSent = sendto(ConnectionSocket, rawData, bytes, 0, (sockaddr*)&SvrAddr, sizeof(SvrAddr));
		if (bytesSent == SOCKET_ERROR) {
			std::cerr << "UDP send failure: " << WSAGetLastError() << std::endl;
			return;
		}
	}
	std::cout << "Sent " << bytesSent << " bytes of data" << std::endl;
}

void MySocket::SetIPAddr(std::string ip) {
	if (bTCPConnect || (mySocket == SERVER && connectionType == TCP && WelcomeSocket != INVALID_SOCKET)) {
		std::cerr << "Cannot change IP address, already connected" << std::endl;
		return;
	}
	unsigned long addr = inet_addr(ip.c_str());
	if (addr == INADDR_NONE) {  // Note: INADDR_NONE may indicate an invalid IP on many systems.
		std::cerr << "Invalid IP address provided" << std::endl;
		return;
	}
	IPAddr = ip;
	SvrAddr.sin_addr.s_addr = inet_addr(IPAddr.c_str());
	std::cout << "IP address changed to: " << IPAddr << std::endl;
}

void MySocket::SetPort(int port) {
	if (bTCPConnect || (mySocket == SERVER && connectionType == TCP && WelcomeSocket != INVALID_SOCKET)) {
		std::cerr << "Cannot change port, already connected" << std::endl;
		return;
	}
	Port = port;
	SvrAddr.sin_port = htons(Port);
	std::cout << "Port changed to " << Port << std::endl;
}

void MySocket::SetType(SocketType type) {
	if (bTCPConnect || (mySocket == SERVER && connectionType == TCP && WelcomeSocket != INVALID_SOCKET)) {
		std::cerr << "Cannot change type, already connected" << std::endl;
		return;
	}
	mySocket = type;
	std::cout << "Socket type changed to " << mySocket << std::endl;
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

int MySocket::GetData(char* rawData) {
	memset(Buffer, 0, MaxSize);
	int bytesReceived = 0;
	if (connectionType == TCP) {
		if (!bTCPConnect) {
			std::cerr << "No TCP connection established for receiving data" << std::endl;
			return -1;
		}
		bytesReceived = recv(ConnectionSocket, Buffer, MaxSize, 0);
		if (bytesReceived == SOCKET_ERROR) {
			std::cerr << "TCP receive failure: " << WSAGetLastError() << std::endl;
			return -1;
		}
	}
	else if (connectionType == UDP) {
		int addrlen = sizeof(SvrAddr);
		bytesReceived = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (sockaddr*)&SvrAddr, &addrlen);
		if (bytesReceived == SOCKET_ERROR) {
			std::cerr << "UDP receive failed: " << WSAGetLastError() << std::endl;
			return -1;
		}
	}
	memcpy(rawData, Buffer, bytesReceived);
	std::cout << "Received " << bytesReceived << " bytes of data." << std::endl;
	return bytesReceived;
}

// updates from Milestone3
// New function: Configure the socket.
bool MySocket::configure(const std::string& ip, int port) {
	// Do not allow changes if a connection is already established.
	if (bTCPConnect) {
		std::cerr << "Socket already connected." << std::endl;
		return false;
	}
	SetIPAddr(ip);
	SetPort(port);
	return true;
}

// New function: Send a string packet over the socket.
bool MySocket::sendPacket(const std::string& packet) {
	// Use the existing SendData method.
	SendData(packet.c_str(), packet.size());
	// In production, you’d check the return value and handle errors.
	return true;
}

// New function: Receive a response from the socket.
std::string MySocket::receiveResponse() {
	// Use a fixed-size buffer (you might wish to improve this).
	char recvBuffer[DEFAULT_SIZE];
	int bytesReceived = GetData(recvBuffer);
	if (bytesReceived > 0) {
		return std::string(recvBuffer, bytesReceived);
	}
	return "";
}