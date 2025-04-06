#include "PktDef.h"
#include <cstring>   // For memcpy

// Default constructor:
// - Initializes the header fields to zero,
// - Sets the Data pointer to nullptr,
// - Sets the CRC to zero,
// - And initializes the RawBuffer pointer to nullptr.
PktDef::PktDef() {
    packet.header.PktCount = 0;
    // Initialize the command flags to zero (Drive, Status, Sleep, Ack, and Padding)
    //packet.header.CmdFlags = 0;
    packet.header.Ack = false;
    packet.header.Sleep = false;
    packet.header.Status = false;
    packet.header.Drive = false;
    // Set default Length to header (4 bytes) + trailer (1 byte) when there is no body.
    packet.header.Length = HEADERSIZE + 1;
    packet.Data = nullptr;
    packet.CRC = 0;
    RawBuffer = nullptr;
}

// Overloaded constructor:
// - Parses the rawBuffer to populate the header, body, and CRC fields.
PktDef::PktDef(char* rawBuffer) {
    // Copy the header (first HEADERSIZE bytes)
    memcpy(&packet.header, rawBuffer, HEADERSIZE);
    // Calculate body length: total packet length minus header and trailer (CRC)
    int bodyLength = packet.header.Length - (HEADERSIZE + 1);
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

// Destructor:
// - Frees any dynamically allocated memory for Data and RawBuffer.
PktDef::~PktDef() {
    if (packet.Data != nullptr) {
        delete[] packet.Data;
        packet.Data = nullptr;
    }
    if (RawBuffer != nullptr) {
        delete[] RawBuffer;
        RawBuffer = nullptr;
    }
}

// SetCmd: Sets the command flag in the header based on the provided CmdType.
// Clears any previously set flags.
//void PktDef::SetCmd(CmdType cmd) {
//    // Clear all command flag bits (padding will remain 0)
//    packet.header.CmdFlags = 0;
//    switch (cmd) {
//    case DRIVE:
//        packet.header.Drive = 1;
//        break;
//    case SLEEP:
//        packet.header.Sleep = 1;
//        break;
//    case RESPONSE:
//        packet.header.Status = 1;
//        break;
//    }
//    //
//    
//    //
//}

void PktDef::SetCmd(CmdType cmdType) {
    packet.header.Drive = false;
    packet.header.Status = false;
    packet.header.Sleep = false;
    packet.header.Ack = true;

    if (cmdType == DRIVE)
        packet.header.Drive = true;
    else if (cmdType == SLEEP)
        packet.header.Sleep = true;
    else if (cmdType == RESPONSE)
        packet.header.Status = true;
    else
        packet.header.Ack = false;
}

// SetBodyData: Allocates memory for the packet body (Data) and copies the provided data.
// Updates the header's Length field (Header + Body + Trailer).
void PktDef::SetBodyData(char* data, int size) {
    if (packet.Data != nullptr) {
        delete[] packet.Data;
        packet.Data = nullptr;
    }
    if (size > 0) {
        packet.Data = new char[size];
        memcpy(packet.Data, data, size);
    }
    // Update Length: header (HEADERSIZE) + body (size) + trailer (1 byte for CRC)
    packet.header.Length = HEADERSIZE + size + 1;
}

// SetPktCount: Sets the packet counter in the header.
void PktDef::SetPktCount(int pktCount) {
    packet.header.PktCount = static_cast<unsigned short>(pktCount);
}

// GetCmd: Returns the command type based on the header flags.
PktDef::CmdType PktDef::GetCmd() const {
    if (packet.header.Drive)
        return DRIVE;
    else if (packet.header.Sleep)
        return SLEEP;
    else if (packet.header.Status)
        return RESPONSE;
    // Default: if no command flag is set, return DRIVE.
    return DRIVE;
}

// GetAck: Returns true if the Ack flag in the header is set.
bool PktDef::GetAck() const {
    return (packet.header.Ack == 1);
}

// GetLength: Returns the packet Length as stored in the header.
int PktDef::GetLength() const {
    return packet.header.Length;
}

// GetBodyData: Returns a pointer to the packet's body data.
char* PktDef::GetBodyData() const {
    return packet.Data;
}

// GetPktCount: Returns the packet counter value.
int PktDef::GetPktCount() const {
    return packet.header.PktCount;
}

// CountBits: Helper function to count the number of bits set to '1' in a byte.
int PktDef::CountBits(unsigned char byte) const {
    int count = 0;
    while (byte) {
        count += (byte & 1);
        byte >>= 1;
    }
    return count;
}

// CalcCRC: Calculates the 1-byte CRC for the packet by counting the number of bits set to '1'
// in the header and body (if present) and updates the packet's CRC field.
void PktDef::CalcCRC() {
    int totalBits = 0;
    // Process header bytes
    unsigned char* headerBytes = reinterpret_cast<unsigned char*>(&packet.header);
    for (int i = 0; i < HEADERSIZE; i++) {
        totalBits += CountBits(headerBytes[i]);
    }
    // Process body data if present:
    int bodyLength = packet.header.Length - (HEADERSIZE + 1); // subtract header and trailer bytes
    if (bodyLength > 0 && packet.Data != nullptr) {
        for (int i = 0; i < bodyLength; i++) {
            totalBits += CountBits(static_cast<unsigned char>(packet.Data[i]));
        }
    }
    packet.CRC = static_cast<unsigned char>(totalBits);
}

// CheckCRC: Verifies the integrity of a given raw data buffer.
// Calculates the CRC (by counting bits set in each byte, excluding the last byte),
// and returns true if it matches the CRC in the buffer.
bool PktDef::CheckCRC(char* buffer, int size) const {
    if (size <= 0)
        return false;
    int totalBits = 0;
    // Calculate CRC from the first (size - 1) bytes.
    for (int i = 0; i < size - 1; i++) {
        totalBits += CountBits(static_cast<unsigned char>(buffer[i]));
    }
    unsigned char crcCalculated = static_cast<unsigned char>(totalBits);
    unsigned char crcProvided = static_cast<unsigned char>(buffer[size - 1]);
    return (crcCalculated == crcProvided);
}

// GenPacket: Generates the complete serialized packet in RawBuffer.
// Copies the header, body data (if any), and CRC into the allocated RawBuffer,
// then returns a pointer to it.
char* PktDef::GenPacket() {
    int totalSize = packet.header.Length;
    // Free any previously allocated RawBuffer.
    if (RawBuffer != nullptr) {
        delete[] RawBuffer;
        RawBuffer = nullptr;
    }
    RawBuffer = new char[totalSize];
    // Copy header (HEADERSIZE bytes)
    memcpy(RawBuffer, &packet.header, HEADERSIZE);
    // Calculate body length (if any) and copy it.
    int bodyLength = totalSize - (HEADERSIZE + 1); // excluding header and CRC
    if (bodyLength > 0 && packet.Data != nullptr) {
        memcpy(RawBuffer + HEADERSIZE, packet.Data, bodyLength);
    }
    // Compute and append the CRC.
    CalcCRC();
    RawBuffer[totalSize - 1] = packet.CRC;
    return RawBuffer;
}
