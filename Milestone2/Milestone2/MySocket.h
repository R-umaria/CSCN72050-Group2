#ifndef MYSOCKET_H
#define MYSOCKET_H
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <cstring>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

enum SocketType { CLIENT, SERVER };
enum ConnectionType { TCP, UDP };

const int DEFAULT_SIZE = 1024;	// temp? buff size

class MySocket {
private:
	char* Buffer;					//dynamically allocate raw buffer space for communication activities
	SOCKET WelcomeSocket;			//used by mysocket object configured as TCP/IP Server
	SOCKET ConnectionSocket;		//used for client/server communications (TCP & UDP)
	struct sockaddr_in SvrAddr;		//store connection information
	SocketType mySocket;			//holds type of socket MySocket object is initialized to
	std::string IPAddr;				//holds IPv4 IP Address string
	int Port;						//holds port number to be used
	ConnectionType connectionType;	//define the Transport Layer protocol being used (TCP/UDP)
	bool bTCPConnect;				//flag to determine if a connection is established
	int MaxSize;					//stores max number of bytes the buffer is allocated to - helps prevent overflows and synchronization issues

public:
	//--Constructor/Destructor--
	MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int);	//constructor that configures socket and connection types, sets IP address
	//and port number, and dynamically allocates memory for the buffer
	// **Note: the constructor should put servers in conditions to either accept
	// connections(TCP), or to receive messages (UDP)**
	//   **NOTE: if an invalid size is provided. the DEFAULT_SIZE should be used**
	~MySocket();			//destructor that cleans up all dynamically allocated memory space

	//--Member Functions--
	void ConnectTCP();			//used to establish TCP socket connection(3way handshake)
	//**Note: this function should have logic to prevent a UDP configured
	//MySocket object from trying to initiate a connection-oriented scenario
	void DisconnectTCP();				//used to disconnect established TCP socker connection(4way handshake)
	void SendData(const char*, int);			//used to transmit block of raw data, specified by the starting memory
	//address and # of bytes, over the socket. Should work with TCP and UDP

// --Setters--
//  Set functionality should contain logic to prevent the header information from being
//  changed if a TCP / IP connection is established or Welcome socket is open
	void SetIPAddr(std::string);		//changes default UP address within MySocket object
	//  **Note: this method should return error message if connection already been established
	void SetPort(int);		//changes default port # within MySocket object
	//  **Note: this method should return an error if a connection has already been established
	void SetType(SocketType);		//changes default SocketType within MySocket object

	//--Getters--
	int GetPort();		//returns the port # configured within the  MySocket object
	SocketType GetType();		//returns default SocketTpe configured in MySocket object
	std::string GetIPAddr();		//returns IP address within MySocket object
	int GetData(char*);			//receives last block of raw data stored in internal MySocket Buffer.
	//After getting the message into Buffer, this function will transfer
	//its contents to the provided memory address and return the total #
	// of bytes written. this function work with TCP & UDP


	// updates from milestone3
	//
	// Configures the socket by setting the IP address and port.
	bool configure(const std::string& ip, int port);
	// Sends the given packet (as a string) using SendData.
	bool sendPacket(const std::string& packet);
	// Receives data into a buffer and returns it as a std::string.
	std::string receiveResponse();


};

#endif