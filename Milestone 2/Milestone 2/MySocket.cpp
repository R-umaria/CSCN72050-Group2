#include "MySocket.h"

MySocket::MySocket(SocketType type, std::string ip, unsigned int port, ConnectionType connection, unsigned int size)
	: mySocket(type), IPAddr(ip), Port(port), connectionType(connection), bTCPConnect(false) {

	if (size > 0)
		MaxSize = size;
	else
		MaxSize = DEFAULT_SIZE;

	Buffer = new char[MaxSize];
}

MySocket::~MySocket() {

}

void MySocket::ConnectTCP() {

}

void MySocket::DisconnectTCP() {

}

void MySocket::SendData(const char* rawData, int bytes) {

}

void MySocket::SetIPAddr(std::string ip) {

}

void MySocket::SetPort(int port) {

}

void MySocket::SetType(SocketType type) {

}

int MySocket::GetPort() {

}

SocketType MySocket::GetType() {

}

std::string MySocket::GetIPAddr() {

}

int MySocket::GetData(char* rawData) {

}