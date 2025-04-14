// Rishi Umaria & Tyler Phillips - Group Project - Milestone 1 - Mobile & Networked Systems

// This main.cpp file is temporary and is used to check the functionality of the PktDef class.

#include <iostream>
#include <iomanip>
#include "PktDef.h"

int main() {
    // Create an instance using the default constructor.
    PktDef pkt;

    // Set the packet count.
    pkt.SetPktCount(1);

    // Set the command flag to DRIVE.
    pkt.SetCmd(PktDef::DRIVE);

    // Create drive command parameters.
    unsigned char driveParams[3];
    driveParams[0] = FORWARD;   // Direction: FORWARD (1)
    driveParams[1] = 10;        // Duration: 10 seconds
    driveParams[2] = 80;        // Speed: 80%

    // Set the packet body data with drive parameters.
    pkt.SetBodyData(reinterpret_cast<char*>(driveParams), sizeof(driveParams));

    // Generate the complete packet.
    char* rawPacket = pkt.GenPacket();
    int packetLength = pkt.GetLength();

    // Print the generated packet in hexadecimal format.
    std::cout << "Generated Packet (" << packetLength << " bytes):" << std::endl;
    for (int i = 0; i < packetLength; i++) {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
            << (static_cast<unsigned int>(static_cast<unsigned char>(rawPacket[i]))) << " ";
    }
    std::cout << std::dec << std::endl;

    // Test the CRC function using the generated packet.
    if (pkt.CheckCRC(rawPacket, packetLength))
        std::cout << "CRC Check: PASSED" << std::endl;
    else
        std::cout << "CRC Check: FAILED" << std::endl;

    // Now, use the overloaded constructor to parse the generated raw packet.
    PktDef parsedPkt(rawPacket);

    // Display parsed packet values.
    std::cout << "Parsed Packet Count: " << parsedPkt.GetPktCount() << std::endl;
    std::cout << "Parsed Packet Length: " << parsedPkt.GetLength() << std::endl;

    // Determine command type.
    PktDef::CmdType cmd = parsedPkt.GetCmd();
    std::cout << "Parsed Command: ";
    if (cmd == PktDef::DRIVE)
        std::cout << "DRIVE" << std::endl;
    else if (cmd == PktDef::SLEEP)
        std::cout << "SLEEP" << std::endl;
    else if (cmd == PktDef::RESPONSE)
        std::cout << "RESPONSE" << std::endl;

    // If there's drive data, print it.
    char* bodyData = parsedPkt.GetBodyData();
    if (bodyData != nullptr) {
        unsigned char* driveBody = reinterpret_cast<unsigned char*>(bodyData);
        std::cout << "Drive Parameters:" << std::endl;
        std::cout << "  Direction: " << static_cast<int>(driveBody[0]) << std::endl;
        std::cout << "  Duration : " << static_cast<int>(driveBody[1]) << std::endl;
        std::cout << "  Speed    : " << static_cast<int>(driveBody[2]) << std::endl;
    }

    return 0;
}
