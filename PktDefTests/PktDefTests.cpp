#include "pch.h"         // Precompiled header (if used)
#include "CppUnitTest.h"
#include "../PktDefProject/PktDef.h"  // Adjust the include path as necessary

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace PktDefTests
{
    TEST_CLASS(PktDefUnitTests)
    {
    public:
        // Test that the default constructor initializes members correctly.
        TEST_METHOD(TestDefaultConstructor)
        {
            PktDef pkt;
            // Default PktCount should be 0.
            Assert::AreEqual(0, pkt.GetPktCount());
            // Without body data, the Length should be HEADERSIZE (4) + 1 (CRC) = 5 bytes.
            Assert::AreEqual(HEADERSIZE + 1, pkt.GetLength());
            // Body data pointer should be nullptr.
            Assert::IsNull(pkt.GetBodyData());
            // Ack flag should be false.
            Assert::IsFalse(pkt.GetAck());
        }

        // Test that setting the packet count works correctly.
        TEST_METHOD(TestSetPktCount)
        {
            PktDef pkt;
            pkt.SetPktCount(1234);
            Assert::AreEqual(1234, pkt.GetPktCount());
        }

        // Test setting command flags using SetCmd and reading back with GetCmd.
        TEST_METHOD(TestSetCmd)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::DRIVE);
            Assert::AreEqual(static_cast<int>(PktDef::DRIVE), static_cast<int>(pkt.GetCmd()));

            pkt.SetCmd(PktDef::SLEEP);
            Assert::AreEqual(static_cast<int>(PktDef::SLEEP), static_cast<int>(pkt.GetCmd()));

            pkt.SetCmd(PktDef::RESPONSE);
            Assert::AreEqual(static_cast<int>(PktDef::RESPONSE), static_cast<int>(pkt.GetCmd()));
        }

        // Test setting the body data and verifying the Length is updated.
        TEST_METHOD(TestSetBodyData)
        {
            PktDef pkt;
            // Create a sample drive command parameter block: {Direction, Duration, Speed}
            unsigned char driveParams[3] = { FORWARD, 10, 80 };
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

        // Test that generating a packet produces a valid packet with a correct CRC.
        TEST_METHOD(TestGenPacketAndCRC)
        {
            PktDef pkt;
            pkt.SetPktCount(55);
            pkt.SetCmd(PktDef::DRIVE);
            unsigned char driveParams[3] = { RIGHT, 20, 90 };
            pkt.SetBodyData(reinterpret_cast<char*>(driveParams), 3);
            char* rawPacket = pkt.GenPacket();
            int length = pkt.GetLength();

            // Verify that the generated packet passes the CRC check.
            Assert::IsTrue(pkt.CheckCRC(rawPacket, length));

            // Modify a byte in the raw packet (simulate corruption) and check that CRC fails.
            rawPacket[2] = rawPacket[2] ^ 0xFF; // Invert some bits in the header.
            Assert::IsFalse(pkt.CheckCRC(rawPacket, length));
        }

        // Test the overloaded constructor by generating a packet and then parsing it back.
        TEST_METHOD(TestOverloadedConstructor)
        {
            // Build a packet using the default constructor.
            PktDef pkt1;
            pkt1.SetPktCount(100);
            pkt1.SetCmd(PktDef::DRIVE);
            unsigned char driveParams[3] = { LEFT, 15, 85 };
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
            Assert::AreEqual(15, static_cast<int>(driveBody[1]));
            Assert::AreEqual(85, static_cast<int>(driveBody[2]));
        }
    };
}
