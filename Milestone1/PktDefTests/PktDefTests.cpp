// Rishi Umaria & Tyler Phillips - Group Project - Milestone 1 - Mobile & Networked Systems

#include "pch.h"
#include "CppUnitTest.h"
#include <vector>
#include "../PktDefProject/PktDef.h"  

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace PktDefTests
{
    TEST_CLASS(PktDefUnitTests)
    {
    public:
        // Verifies the default constructor initializes members correctly
        TEST_METHOD(DefaultConstructor)
        {
            PktDef pkt;
            // Default PktCount should be 0.
            Assert::AreEqual(0, pkt.GetPktCount());
            // Length should be 0 when initialized (no body and no CRC set yet)
            Assert::AreEqual(0, pkt.GetLength());
            // Body data pointer should be nullptr.
            Assert::IsNull(pkt.GetBodyData());
            // Ack flag should be false.
            Assert::IsFalse(pkt.GetAck());
        }

        // Verifies the packet count can be correctly Set and Get
        TEST_METHOD(PktCount)
        {
            PktDef pkt;
            pkt.SetPktCount(1234);
            Assert::AreEqual(1234, pkt.GetPktCount());
        }

        // Verifies the commands can be properly Set and Get
        TEST_METHOD(Set_and_GetCmd)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::DRIVE);
            Assert::AreEqual(static_cast<int>(PktDef::DRIVE), static_cast<int>(pkt.GetCmd()));
            Assert::IsTrue(pkt.GetAck());

            pkt.SetCmd(PktDef::SLEEP);
            Assert::AreEqual(static_cast<int>(PktDef::SLEEP), static_cast<int>(pkt.GetCmd()));
            Assert::IsTrue(pkt.GetAck());

            pkt.SetCmd(PktDef::RESPONSE);
            Assert::AreEqual(static_cast<int>(PktDef::RESPONSE), static_cast<int>(pkt.GetCmd()));
            Assert::IsTrue(pkt.GetAck());
        }

        // Verifies the body data is correctly Set and Get
        TEST_METHOD(Set_and_GetBodyData)
        {
            PktDef pkt;
            unsigned char driveParams[3] = { FORWARD, 10, 80 };         // Create sample drive command parameters
            pkt.SetBodyData(reinterpret_cast<char*>(driveParams), 3);

            // Expected Length = HEADERSIZE (4) + 3 (body) + 1 (CRC) = 8 bytes.
            Assert::AreEqual(HEADERSIZE + 3 + 1, pkt.GetLength());

            // Verify the data is correctly copied.
            char* body = pkt.GetBodyData();
            Assert::IsNotNull(body);
            unsigned char* driveBody = reinterpret_cast<unsigned char*>(body);
            Assert::AreEqual(static_cast<int>(FORWARD), static_cast<int>(driveBody[0]));
            Assert::AreEqual(10, static_cast<int>(driveBody[1]));
            Assert::AreEqual(80, static_cast<int>(driveBody[2]));
        }

        // Validates a proper packet and CRC are generated
        TEST_METHOD(GenPacketAndCRC)
        {
            PktDef pkt;
            pkt.SetPktCount(55);
            pkt.SetCmd(PktDef::DRIVE);

            unsigned char driveParams[3] = { RIGHT, 5, 90 };
            pkt.SetBodyData(reinterpret_cast<char*>(driveParams), 3);

            char* rawPacket = pkt.GenPacket();
            int length = pkt.GetLength();

            // Verify that the generated packet passes the CRC check.
            Assert::IsTrue(pkt.CheckCRC(rawPacket, length));
        }

        // Verify the overloaded constructor can correctly parse the information from a provided packet
        TEST_METHOD(OverloadedConstructor)
        {
            // Build a packet using the default constructor.
            PktDef pkt1;
            pkt1.SetPktCount(100);
            pkt1.SetCmd(PktDef::DRIVE);
            unsigned char driveParams[3] = { LEFT, 10, 85 };
            pkt1.SetBodyData(reinterpret_cast<char*>(driveParams), 3);
            char* rawPacket = pkt1.GenPacket();
            int length = pkt1.GetLength();

            // Create a new object using the overloaded constructor.
            PktDef pkt2(rawPacket);

            // Validate that the parsed values match.
            Assert::AreEqual(100, pkt2.GetPktCount());
            Assert::AreEqual(length, pkt2.GetLength());
            Assert::AreEqual(static_cast<int>(PktDef::DRIVE), static_cast<int>(pkt2.GetCmd()));

            // Verify drive command parameters.
            char* body = pkt2.GetBodyData();
            Assert::IsNotNull(body);
            unsigned char* driveBody = reinterpret_cast<unsigned char*>(body);
            Assert::AreEqual(static_cast<int>(LEFT), static_cast<int>(driveBody[0]));
            Assert::AreEqual(10, static_cast<int>(driveBody[1]));
            Assert::AreEqual(85, static_cast<int>(driveBody[2]));
        }

        // Verify the CheckCRC method correctly deduces an incorrect CRC
        TEST_METHOD(CheckCRCFail)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::DRIVE);
            pkt.SetPktCount(1);

            unsigned char data[3] = { FORWARD, 10, 80 };
            pkt.SetBodyData(reinterpret_cast<char*>(data), 3);

            char* buff = pkt.GenPacket();
            int length = pkt.GetLength();

            // Change the CRC to simulate corruption.
            buff[length - 1] ^= 0xFF;
            Assert::IsFalse(pkt.CheckCRC(buff, length));
        }

        // Verify the CheckCRC method correctly handles an invalid size (0)
        TEST_METHOD(CheckCRCInvalidSize)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::DRIVE);
            pkt.SetPktCount(1);

            unsigned char data[3] = { FORWARD, 10, 80 };
            pkt.SetBodyData(reinterpret_cast<char*>(data), 3);

            char* buff = pkt.GenPacket();
            int length = 0;

            Assert::IsFalse(pkt.CheckCRC(buff, length));
        }

        // Verify that SetCmd with an invalid command value does not set the Ack flag.
        TEST_METHOD(SetCmdFail)
        {
            PktDef pkt;
            pkt.SetCmd(static_cast<PktDef::CmdType>(10)); // Invalid command
            Assert::IsFalse(pkt.GetAck());
        }

        // Additional Tests for Robust Coverage

        // Test when SetBodyData is called with size 0; should result in no body data and Length = HEADERSIZE + 1
        TEST_METHOD(EmptyBodyData)
        {
            PktDef pkt;
            pkt.SetBodyData(nullptr, 0);
            // Expected Length = header (4) + CRC (1) = 5 bytes.
            Assert::AreEqual(HEADERSIZE + 1, pkt.GetLength());
            Assert::IsNull(pkt.GetBodyData());
        }

        // Test GetCmd on a default-constructed packet (with no command set).
        // Our implementation defaults to returning DRIVE.
        TEST_METHOD(DefaultGetCmd)
        {
            PktDef pkt;
            Assert::AreEqual(static_cast<int>(PktDef::RESPONSE), static_cast<int>(pkt.GetCmd()));
        }

        // Test calling GenPacket twice: subsequent calls should generate the same serialized packet.
        TEST_METHOD(MultipleGenPacketCalls)
        {
            PktDef pkt;
            pkt.SetPktCount(200);
            pkt.SetCmd(PktDef::DRIVE);
            unsigned char driveParams[3] = { RIGHT, 8, 95 };
            pkt.SetBodyData(reinterpret_cast<char*>(driveParams), 3);

            char* rawPacket1 = pkt.GenPacket();
            int length1 = pkt.GetLength();
            std::vector<char> copyPacket(rawPacket1, rawPacket1 + length1);

            char* rawPacket2 = pkt.GenPacket(); // call GenPacket again
            int length2 = pkt.GetLength();
            Assert::AreEqual(length1, length2);
            for (int i = 0; i < length1; i++) {
                Assert::AreEqual(copyPacket[i], rawPacket2[i]);
            }
        }

        // Directly test CalcCRC by forcing a CRC calculation and verifying via CheckCRC.
        TEST_METHOD(DirectCalcCRC)
        {
            PktDef pkt;
            pkt.SetPktCount(50);
            pkt.SetCmd(PktDef::DRIVE);
            unsigned char driveParams[3] = { FORWARD, 12, 80 };
            pkt.SetBodyData(reinterpret_cast<char*>(driveParams), 3);
            // Explicitly calculate the CRC.
            pkt.CalcCRC();
            char* rawPacket = pkt.GenPacket();
            int length = pkt.GetLength();
            Assert::IsTrue(pkt.CheckCRC(rawPacket, length));
        }
    };
}
