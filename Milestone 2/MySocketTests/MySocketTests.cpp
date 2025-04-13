#include "pch.h"
#include "CppUnitTest.h"
#include "../Milestone 2/MySocket.h"
#include <thread>
#include <chrono>
#include <string>

namespace Microsoft {
    namespace VisualStudio {
        namespace CppUnitTestFramework {
            template<>
            std::wstring ToString<SocketType>(const SocketType& value)
            {
                switch (value)
                {
                case CLIENT:
                    return L"CLIENT";
                case SERVER:
                    return L"SERVER";
                default:
                    return L"UNKNOWN";
                }
            }
        }
    }
}


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std::chrono;

namespace MySocketTests
{
    TEST_CLASS(MySocketTests)
    {
    public:
        // Test that the constructor initializes IP, Port, and buffer size correctly
        TEST_METHOD(ConstructorTest)
        {
            MySocket tcpSocket(CLIENT, "127.0.0.1", 8080, TCP, 2048);
            Assert::AreEqual(std::string("127.0.0.1"), tcpSocket.GetIPAddr());
            Assert::AreEqual(8080, tcpSocket.GetPort());

            MySocket udpSocket(CLIENT, "127.0.0.1", 8081, UDP, 1024);
            Assert::AreEqual(std::string("127.0.0.1"), udpSocket.GetIPAddr());
            Assert::AreEqual(8081, udpSocket.GetPort());
        }

        // Test that setting IP, Port, and Type before any connection works as expected
        TEST_METHOD(GetterSetterTest)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            // Change values before connection
            socket.SetIPAddr("192.168.1.1");
            socket.SetPort(9000);
            socket.SetType(SERVER);

            Assert::AreEqual(std::string("192.168.1.1"), socket.GetIPAddr());
            Assert::AreEqual(9000, socket.GetPort());
            Assert::AreEqual(SERVER, socket.GetType());
        }

        // Test a full TCP connection and data transmission between a server and a client
        TEST_METHOD(TCPCommunicationTest)
        {
            const int port = 9001;
            const std::string ip = "127.0.0.1";
            const int bufferSize = 1024;
            std::string message = "Hello";

            // Create a server socket (SERVER mode) for TCP and a client socket (CLIENT mode)
            MySocket serverSocket(SERVER, ip, port, TCP, bufferSize);
            MySocket clientSocket(CLIENT, ip, port, TCP, bufferSize);

            // Start the server thread to accept a connection and receive data
            std::thread serverThread([&serverSocket, &message]() {
                serverSocket.ConnectTCP();
                char recvBuffer[1024] = { 0 };
                int bytesReceived = serverSocket.GetData(recvBuffer);
                Assert::IsTrue(bytesReceived > 0, L"Server did not receive any data");
                std::string received(recvBuffer, bytesReceived);
                Assert::AreEqual(message, received);
                });

            // Allow server a moment to start listening
            std::this_thread::sleep_for(milliseconds(100));

            clientSocket.ConnectTCP();
            clientSocket.SendData(message.c_str(), (int)message.size());

            serverThread.join();
            clientSocket.DisconnectTCP();
            serverSocket.DisconnectTCP();
        }

        // Test that the setter functions do not alter the settings when a TCP connection is active
        TEST_METHOD(SetterFailTest)
        {
            const int port = 9002;
            const std::string ip = "127.0.0.1";
            const int bufferSize = 1024;

            MySocket serverSocket(SERVER, ip, port, TCP, bufferSize);
            MySocket clientSocket(CLIENT, ip, port, TCP, bufferSize);

            // Start the server in its own thread to establish a connection
            std::thread serverThread([&serverSocket]() {
                serverSocket.ConnectTCP();
                // Wait to simulate an active connection
                std::this_thread::sleep_for(milliseconds(500));
                });

            std::this_thread::sleep_for(milliseconds(100));
            clientSocket.ConnectTCP();

            // Store original settings
            std::string originalIP = clientSocket.GetIPAddr();
            int originalPort = clientSocket.GetPort();
            SocketType originalType = clientSocket.GetType();

            // Attempt to change settings after connection; these should be rejected
            clientSocket.SetIPAddr("10.0.0.1");
            clientSocket.SetPort(9999);
            clientSocket.SetType(SERVER);

            Assert::AreEqual(originalIP, clientSocket.GetIPAddr());
            Assert::AreEqual(originalPort, clientSocket.GetPort());
            Assert::AreEqual(originalType, clientSocket.GetType());

            clientSocket.DisconnectTCP();
            serverSocket.DisconnectTCP();
            serverThread.join();
        }
    };
}
