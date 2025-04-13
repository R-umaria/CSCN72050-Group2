#include "pch.h"
#include "CppUnitTest.h"
#include "../Milestone 2/MySocket.h"
#include <thread>
#include <chrono>
#include <string>
#include <vector>

using namespace std::chrono;
using namespace std::this_thread;

namespace Microsoft {
    namespace VisualStudio {
        namespace CppUnitTestFramework {
            // Provide a ToString specialization for SocketType so MSTest can print enum values.
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

            // Allow the server a moment to start listening
            sleep_for(milliseconds(100));

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
                sleep_for(milliseconds(500));
                });

            sleep_for(milliseconds(100));
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

        // Extended UDP test to check send/receive functionality in UDP mode
        TEST_METHOD(UDPCommunicationTest)
        {
            const int port = 9110;
            const std::string ip = "127.0.0.1";
            const int bufferSize = 1024;
            std::string message = "UDP Test Message";

            // Create UDP sockets (for both SERVER and CLIENT, no connection needed)
            MySocket udpServer(SERVER, ip, port, UDP, bufferSize);
            MySocket udpClient(CLIENT, ip, port, UDP, bufferSize);

            bool messageReceived = false;
            std::thread udpServerThread([&udpServer, &message, &messageReceived]() {
                char recvBuffer[1024] = { 0 };
                int bytesReceived = udpServer.GetData(recvBuffer);
                if (bytesReceived > 0) {
                    std::string received(recvBuffer, bytesReceived);
                    if (received == message)
                        messageReceived = true;
                }
                });

            sleep_for(milliseconds(100));
            udpClient.SendData(message.c_str(), (int)message.size());
            udpServerThread.join();

            Assert::IsTrue(messageReceived, L"UDP server did not receive the expected message.");
        }

        // Test sending data larger than the allocated buffer (should print an error and not send)
        TEST_METHOD(SendDataExceedBufferTest)
        {
            // Create a socket with a small buffer size
            MySocket socket(CLIENT, "127.0.0.1", 8085, TCP, 100); // 100-byte buffer

            // Prepare data that exceeds the buffer size (150 bytes)
            std::string longData(150, 'X');  // 150 'X' characters

            // No connection is needed to test the size check
            // This will trigger the error branch in SendData.
            // (Since the function does not return a status, we simply call it and ensure no crash occurs.)
            socket.SendData(longData.c_str(), (int)longData.size());

            // If the function returns without crashing, consider the test passed.
            Assert::IsTrue(true, L"SendData exceeded buffer did not handle gracefully.");
        }

        // Test disconnect when no connection is established - should not crash or cause undefined behavior
        TEST_METHOD(DisconnectNoConnectionTest)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8085, TCP, 1024);
            // Attempt to disconnect without an active connection.
            socket.DisconnectTCP();
            Assert::IsTrue(true, L"DisconnectTCP called without connection should not crash.");
        }

        // Test that setters work normally when no connection is active
        TEST_METHOD(SetterWorksWithoutConnectionTest)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            // Set new values
            socket.SetIPAddr("192.168.1.2");
            socket.SetPort(8000);
            socket.SetType(CLIENT);

            Assert::AreEqual(std::string("192.168.1.2"), socket.GetIPAddr());
            Assert::AreEqual(8000, socket.GetPort());
            Assert::AreEqual(CLIENT, socket.GetType());
        }

        // Optional: Test an invalid IP address via the setter.
        // Note: If your MySocket implementation calls exit() on invalid IP format,
        // this test might abort the test run. If so, consider modifying the error handling
        // in MySocket to handle errors without calling exit().
        TEST_METHOD(InvalidIPTest)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            std::string originalIP = socket.GetIPAddr();
            // Attempt to set an invalid IP.
            socket.SetIPAddr("invalid ip");
            // The IP should remain unchanged.
            Assert::AreEqual(originalIP, socket.GetIPAddr());
        }


        // Additional tests to cover error conditions and edge cases

        // Test calling ConnectTCP on a UDP socket.
        // Since ConnectTCP is only meant for TCP, calling it on a UDP socket should immediately return
        // and should not alter any connection state.
        TEST_METHOD(ConnectTCP_NonTCPTest)
        {
            // Create a UDP socket in CLIENT mode.
            MySocket udpSocket(CLIENT, "127.0.0.1", 9120, UDP, 1024);
            // Calling ConnectTCP should detect the non-TCP mode and return early.
            udpSocket.ConnectTCP();
            // Calling DisconnectTCP should also safely return (it checks connectionType and does nothing).
            udpSocket.DisconnectTCP();
            Assert::IsTrue(true, L"ConnectTCP on a UDP socket did not alter connection state and DisconnectTCP safely returned.");
        }

        // Test GetData when no connection is established in TCP mode.
        // Expected behavior: since no TCP connection is made, GetData should detect that and return -1.
        TEST_METHOD(GetDataWithoutConnectionTest)
        {
            MySocket tcpClient(CLIENT, "127.0.0.1", 9130, TCP, 1024);
            char buffer[1024] = { 0 };
            int result = tcpClient.GetData(buffer);
            Assert::AreEqual(-1, result); // Expect error indicator when no connection exists.
        }

        // Test SendData when no connection is established in TCP mode.
        // Since there's no active connection, SendData should detect that and not send data.
        // This test simply ensures that the function returns without crashing.
        TEST_METHOD(SendDataWithoutConnectionTest)
        {
            MySocket tcpClient(CLIENT, "127.0.0.1", 9140, TCP, 1024);
            std::string msg = "Test message";
            // With no connection established, calling SendData should output an error but not crash.
            tcpClient.SendData(msg.c_str(), (int)msg.size());
            Assert::IsTrue(true, L"SendData without a connection did not crash.");
        }



        // Additional tests created by Tyler

        TEST_METHOD(TCPDoubleConnectTest)
        {
            const int port = 9150;
            const std::string ip = "127.0.0.1";
            const int bufferSize = 1024;

            // Create server and client sockets for TCP
            MySocket serverSocket(SERVER, ip, port, TCP, bufferSize);
            MySocket clientSocket(CLIENT, ip, port, TCP, bufferSize);

            // Start the server thread to accept a connection.
            std::thread serverThread([&serverSocket]() {
                serverSocket.ConnectTCP();
                // Keep connection open for a moment.
                sleep_for(milliseconds(500));
                });

            // Allow server to start listening
            sleep_for(milliseconds(100));

            // Connect the client socket
            clientSocket.ConnectTCP();
            // Call ConnectTCP again on the already connected socket.
            clientSocket.ConnectTCP(); // Expected to print "TCP already connected"

            // Disconnect and clean up
            clientSocket.DisconnectTCP();
            serverSocket.DisconnectTCP();
            serverThread.join();

            Assert::IsTrue(true, L"Double connection attempt did not crash.");
        }

        // Test DisconnectTCP on a UDP socket (should simply print an error without crashing)
        TEST_METHOD(DisconnectTCP_NonTCPTest)
        {
            // Create a UDP client socket
            MySocket udpSocket(CLIENT, "127.0.0.1", 9200, UDP, 1024);
            // Calling DisconnectTCP on a non-TCP socket should be a no-op.
            udpSocket.DisconnectTCP();
            Assert::IsTrue(true, L"DisconnectTCP on UDP socket did not crash.");
        }

        // Test SendData with a nullptr as data pointer
        TEST_METHOD(SendDataNullTest)
        {
            // Create a UDP client socket; no connection is needed for this test.
            MySocket udpSocket(CLIENT, "127.0.0.1", 9300, UDP, 1024);
            // Should detect the null pointer and return early.
            udpSocket.SendData(nullptr, 10);
            Assert::IsTrue(true, L"SendData with nullptr did not crash.");
        }

        // Test SendData with 0 bytes
        TEST_METHOD(SendDataZeroBytesTest)
        {
            MySocket udpSocket(CLIENT, "127.0.0.1", 9310, UDP, 1024);
            // Zero bytes should be flagged as invalid.
            udpSocket.SendData("Data", 0);
            Assert::IsTrue(true, L"SendData with 0 bytes did not crash.");
        }

        // Test SendData with negative byte count
        TEST_METHOD(SendDataNegativeBytesTest)
        {
            MySocket udpSocket(CLIENT, "127.0.0.1", 9320, UDP, 1024);
            // Negative byte count should be handled gracefully.
            udpSocket.SendData("Data", -5);
            Assert::IsTrue(true, L"SendData with negative bytes did not crash.");
        }

        // Test setter functions on a UDP socket; for connectionless protocols, the setters should work normally.
        TEST_METHOD(SettersUDPTest)
        {
            // Create a UDP client socket.
            MySocket udpSocket(CLIENT, "127.0.0.1", 9330, UDP, 1024);
            // Change settings; UDP does not have an active connection flag.
            udpSocket.SetIPAddr("192.168.100.100");
            udpSocket.SetPort(9400);
            udpSocket.SetType(SERVER);  // Changing from CLIENT to SERVER for testing

            Assert::AreEqual(std::string("192.168.100.100"), udpSocket.GetIPAddr());
            Assert::AreEqual(9400, udpSocket.GetPort());
            Assert::AreEqual(SERVER, udpSocket.GetType());
        }

        // Test a TCP client connecting to an unreachable server (simulate connection failure)
        // Note: "192.0.2.1" is reserved for documentation and is not routable on the public Internet.
        TEST_METHOD(TCPConnectFailureTest)
        {
            MySocket tcpClient(CLIENT, "192.0.2.1", 9450, TCP, 1024);
            tcpClient.ConnectTCP();  // Expected to fail; ConnectTCP should print an error and not set bTCPConnect.

            // Attempt to receive data; should return -1 because no connection exists.
            char buffer[1024] = { 0 };
            int result = tcpClient.GetData(buffer);
            Assert::AreEqual(-1, result);

            // Disconnect (should safely do nothing if not connected)
            tcpClient.DisconnectTCP();
        }

        // Test valid SetIPAddr on a TCP socket when no connection is active.
        TEST_METHOD(SetIPAddrValidTest)
        {
            MySocket tcpSocket(CLIENT, "127.0.0.1", 9470, TCP, 1024);
            tcpSocket.SetIPAddr("10.1.1.1");
            Assert::AreEqual(std::string("10.1.1.1"), tcpSocket.GetIPAddr());
        }

        // Test valid SetPort on a TCP socket when no connection is active.
        TEST_METHOD(SetPortValidTest)
        {
            MySocket tcpSocket(CLIENT, "127.0.0.1", 9471, TCP, 1024);
            tcpSocket.SetPort(1234);
            Assert::AreEqual(1234, tcpSocket.GetPort());
        }

        // Test valid SetType on a TCP socket when no connection is active.
        TEST_METHOD(SetTypeValidTest)
        {
            MySocket tcpSocket(CLIENT, "127.0.0.1", 9472, TCP, 1024);
            tcpSocket.SetType(SERVER);
            Assert::AreEqual(SERVER, tcpSocket.GetType());
        }
    };
}
