// Rishi Umaria & Tyler Phillips - Group Project - Milestone 1 - Mobile & Networked Systems

#ifndef PKTDEF_H
#define PKTDEF_H

#include <memory>
#include <cstring>   // For memcpy
#include <cstdlib>   // For malloc/free

/*
    CSCN72050 Term Project, Protocol vW25B
    Milestone #1

    Application layer protocol:
      - Header (4 bytes total):
          - PktCount: unsigned short int (2 bytes)
          - Command Flags: boolean bit-fields Drive, Status, Sleep, Ack in 1 byte with 4 bits of padding.
          - Length: uint8_t (1 byte) representing the total number of bytes in the packet.
      - Body:
          - For Drive commands: Contains drive parameters (DriveBody) with:
              - Direction (1 byte): FORWARD (1), BACKWARD (2), RIGHT (3), LEFT (4)
              - Duration (1 byte): Time in seconds to execute command
              - Speed (1 byte): Duty cycle percentage between 80-100
          - For Sleep and Response commands: No body data.
      - Tail:
          - CRC: 1 byte (unsigned char) parity check calculated by counting the number of bits set to '1'
*/

// Constant definitions for drive directions and header size.
const int FORWARD = 1;
const int BACKWARD = 2;
const int RIGHT = 3;
const int LEFT = 4;
const int HEADERSIZE = 4; // PktCount (2 bytes) + Command Flags (1 byte) + Length (1 byte) = 4 bytes.

struct Header {
    unsigned short int PktCount;     // 2 bytes
    bool Drive : 1;                  // 1 bit
    bool Status : 1;                 // 1 bit
    bool Sleep : 1;                  // 1 bit
    bool Ack : 1;                    // 1 bit
    unsigned char Padding : 4;       // Padding for alignment (4 bits)
    uint8_t Length;                  // 1 byte
};

// Drive Command Structure
struct DriveBody {
    unsigned char Direction; // 1-byte direction: FORWARD, BACKWARD, RIGHT, or LEFT.
    unsigned char Duration;  // 1-byte duration (in seconds).
    unsigned char Speed;     // 1-byte motor speed percentage (80-100).
};

class PktDef {
private:
    // Complete Packet Structure
    struct CmdPacket {
        Header header;      // Packet header.
        char* Data;         // Pointer to the packet body data.
        unsigned char CRC;  // 1-byte CRC for packet validation.
    } packet;

    char* RawBuffer;  // Holds completed, serialized packet for transmission

    int CountBits(unsigned char byte) const;  // Counts # of bits set to '1'

public:
    enum CmdType { DRIVE, SLEEP, RESPONSE };      // Enumerated command types.

    // --- Constructors and Destructor ---
    PktDef();
    PktDef(char* rawBuffer);
    ~PktDef();

    // --- Setters ---
    void SetCmd(CmdType cmd);
    void SetBodyData(char* data, int size);
    void SetPktCount(int pktCount);

    // --- Getters ---
    CmdType GetCmd() const;
    bool GetAck() const;
    int GetLength() const;
    char* GetBodyData() const;
    int GetPktCount() const;

    // Checks the CRC of a given raw data buffer.
    bool CheckCRC(char* buffer, int size) const;

    // Calculates the 1-byte CRC for the packet (by counting bits set to '1')
    void CalcCRC();

    // Generates a complete serialized packet in RawBuffer.
    char* GenPacket();


    //updates from milestone3
    // 
    // Generates and returns a complete serialized packet as a std::string.
    // For drive commands, body data is constructed from a DriveBody.
    static std::string createDriveCommand(int direction, int speed, int duration);
    // Creates a packet for a sleep command (no body data).
    static std::string createSleepCommand();
    // Creates a packet to request telemetry data (using the RESPONSE flag).
    static std::string createTelemetryRequest();
};

#endif // PKTDEF_H
