// Rishi Umaria & Tyler Phillips - Group Project - Milestone 1 - Mobile & Networked Systems

#include "PktDef.h"
#include <cstring>   // For memcpy

// Default constructor
PktDef::PktDef() {
    // Set all header information to 0
    packet.header.PktCount = 0;
    packet.header.Ack = false;
    packet.header.Sleep = false;
    packet.header.Status = false;
    packet.header.Drive = false;
    packet.header.Length = 0;
    // Set Data to nullptr
    packet.Data = nullptr;
    // Set CRC to 0
    packet.CRC = 0;
    // Set RawBuffer to nullptr
    RawBuffer = nullptr;
}

// Overloaded constructor
PktDef::PktDef(char* rawBuffer) {
    memcpy(&packet.header, rawBuffer, HEADERSIZE);  // Copy the header
   
    int bodyLength = packet.header.Length - (HEADERSIZE + 1);   // Calculate body length
    if (bodyLength > 0) {
        packet.Data = new char[bodyLength];
        memcpy(packet.Data, rawBuffer + HEADERSIZE, bodyLength);
    }
    else {
        packet.Data = nullptr;
    }
    // Set the CRC from the last byte of the raw buffer
    packet.CRC = *(reinterpret_cast<unsigned char*>(rawBuffer + packet.header.Length - 1));
    RawBuffer = nullptr;
}

// Destructor
PktDef::~PktDef() {
    if (packet.Data != nullptr) {   // Frees memory for Data
        delete[] packet.Data;
        packet.Data = nullptr;
    }
    if (RawBuffer != nullptr) {     // Frees memory for RawBuffer
        delete[] RawBuffer;
        RawBuffer = nullptr;
    }
}

// Sets command type
void PktDef::SetCmd(CmdType cmdType) {
    packet.header.Drive = false;
    packet.header.Status = false;
    packet.header.Sleep = false;
    packet.header.Ack = true;

    // Sets packet command flag based on cmdType
    if (cmdType == DRIVE)
        packet.header.Drive = true;
    else if (cmdType == SLEEP)
        packet.header.Sleep = true;
    else if (cmdType == RESPONSE)
        packet.header.Status = true;
    else
        packet.header.Ack = false;
}

// Sets body data
void PktDef::SetBodyData(char* data, int size) {
    if (packet.Data != nullptr) {
        delete[] packet.Data;
        packet.Data = nullptr;
    }
    if (size > 0) {
        packet.Data = new char[size];      // Allocates memory for body based on size
        memcpy(packet.Data, data, size);   // Copies data into Data    
    }
    // Update Length: header + body + tail (1 byte for CRC)
    packet.header.Length = HEADERSIZE + size + 1;
}

// Sets the packet counter
void PktDef::SetPktCount(int pktCount) {
    packet.header.PktCount = pktCount;
}

// Returns the command type based on the header flag
PktDef::CmdType PktDef::GetCmd() const {
    if (packet.header.Drive)
        return DRIVE;
    else if (packet.header.Sleep)
        return SLEEP;
    else if (packet.header.Status)
        return RESPONSE;
    // Default: if no command flag is set
    return RESPONSE;
}

// Returns the value of the ack flag in the header
bool PktDef::GetAck() const {
    return packet.header.Ack;
}

// Returns the packet Length stored in the header
int PktDef::GetLength() const {
    return packet.header.Length;
}

// Returns a pointer to the packet's body data
char* PktDef::GetBodyData() const {
    return packet.Data;
}

// Returns the packet count
int PktDef::GetPktCount() const {
    return packet.header.PktCount;
}

// Helper function to count the number of bits set to '1' in a byte
int PktDef::CountBits(unsigned char byte) const {
    int count = 0;
    while (byte) {
        count += (byte & 1);
        byte >>= 1;
    }
    return count;
}

// Calculates the CRC for the packet and updates the packet's CRC field
void PktDef::CalcCRC() {
    int totalBits = 0;
    // Process header bytes
    unsigned char* headerBytes = reinterpret_cast<unsigned char*>(&packet.header);
    for (int i = 0; i < HEADERSIZE; i++) {
        totalBits += CountBits(headerBytes[i]);
    }
    // Process body data if there is any
    int bodyLength = packet.header.Length - (HEADERSIZE + 1); // Subtract head and tail bytes
    if (bodyLength > 0 && packet.Data != nullptr) {
        for (int i = 0; i < bodyLength; i++) {
            totalBits += CountBits(static_cast<unsigned char>(packet.Data[i]));
        }
    }
    packet.CRC = static_cast<unsigned char>(totalBits);
}

// Validates the CRC provided matches the CRC of the packet
bool PktDef::CheckCRC(char* buffer, int size) const {
    if (size <= 0)
        return false;
    int totalBits = 0;
    // Calculate CRC for the head and the body (size - 1)
    for (int i = 0; i < size - 1; i++) {
        totalBits += CountBits(static_cast<unsigned char>(buffer[i]));
    }
    unsigned char crcCalculated = static_cast<unsigned char>(totalBits);
    unsigned char crcProvided = static_cast<unsigned char>(buffer[size - 1]);
    return (crcCalculated == crcProvided);
}

// Generates the complete serialized packet in RawBuffer
char* PktDef::GenPacket() {
    int totalSize = packet.header.Length;
    // Free any previously allocated RawBuffer.
    if (RawBuffer != nullptr) {
        delete[] RawBuffer;
        RawBuffer = nullptr;
    }
    RawBuffer = new char[totalSize];
    memcpy(RawBuffer, &packet.header, HEADERSIZE);                  // Copy header
    // Calculate body length (if any) and copy it.
    int bodyLength = totalSize - (HEADERSIZE + 1);                  // Determine body length
    if (bodyLength > 0 && packet.Data != nullptr) {
        memcpy(RawBuffer + HEADERSIZE, packet.Data, bodyLength);    // Copy body
    }

    CalcCRC();
    RawBuffer[totalSize - 1] = packet.CRC;  // Add CRC to packet
    return RawBuffer;
}
