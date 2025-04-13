#include "MySocket.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace std::this_thread;

int main(int argc, char* argv[]) {
    // Simple menu to choose the role.
    cout << "Select role:" << endl;
    cout << "1. Server" << endl;
    cout << "2. Client" << endl;
    cout << "Enter choice: ";

    int choice = 0;
    cin >> choice;

    // Defaults for both server and client
    string ip = "127.0.0.1";
    int port = 9000;
    int bufferSize = 1024;
    ConnectionType connType = TCP; // For Milestone2 we're focusing on TCP.

    if (choice == 1) {
        cout << "\nStarting in SERVER mode on port " << port << "..." << endl;
        MySocket serverSocket(SERVER, ip, port, connType, bufferSize);
        cout << "Server socket created. Waiting for client connection..." << endl;
        serverSocket.ConnectTCP();
        cout << "Client connected." << endl;

        // Wait for a message from the client.
        char recvBuffer[1024] = { 0 };
        int bytesReceived = serverSocket.GetData(recvBuffer);
        if (bytesReceived > 0) {
            cout << "Received message: " << string(recvBuffer, bytesReceived) << endl;
        }
        else {
            cout << "No data received." << endl;
        }
        serverSocket.DisconnectTCP();
        cout << "Server shutdown." << endl;
    }
    else if (choice == 2) {
        cout << "\nStarting in CLIENT mode connecting to " << ip << ":" << port << "..." << endl;
        MySocket clientSocket(CLIENT, ip, port, connType, bufferSize);
        clientSocket.ConnectTCP();
        cout << "Connected to server." << endl;

        // Prompt user to enter a command/message.
        cin.ignore();  // Clear leftover newline from previous input
        cout << "Enter message to send: ";
        string message;
        getline(cin, message);
        clientSocket.SendData(message.c_str(), static_cast<int>(message.size()));
        cout << "Message sent." << endl;

        // Optionally, wait for a response.
        char recvBuffer[1024] = { 0 };
        int bytesReceived = clientSocket.GetData(recvBuffer);
        if (bytesReceived > 0) {
            cout << "Received response: " << string(recvBuffer, bytesReceived) << endl;
        }
        else {
            cout << "No response received." << endl;
        }
        clientSocket.DisconnectTCP();
        cout << "Client disconnected." << endl;
    }
    else {
        cout << "Invalid choice. Exiting." << endl;
    }

    // Add a short delay before closing.
    sleep_for(milliseconds(500));
    return 0;
}
