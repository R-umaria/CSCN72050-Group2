// Rishi Umaria & Tyler Phillips - Group Project - Milestone 1 - Mobile & Networked Systems

#include "pch.h"         
#include "CppUnitTest.h"
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
            // Length should be 0 when initialized
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
            unsigned char driveParams[3] = { FORWARD, 10, 80 };         // Create sample commands
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

            // Change the CRC
            buff[length - 1] ^= 0xFF;
            Assert::IsFalse(pkt.CheckCRC(buff, length));
        }

        // Verifies the CheckCRC method can correctly deduce an incorrect size
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

        // Verify the SetCmd method correctly deduces an incorrect CmdType
        TEST_METHOD(SetCmdFail)
        {
            PktDef pkt;
            pkt.SetCmd(static_cast<PktDef::CmdType>(10));

            Assert::IsFalse(pkt.GetAck());
        }
    };
}
