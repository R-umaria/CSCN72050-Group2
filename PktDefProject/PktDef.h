#ifndef PKTDEF_H
#define PKTDEF_H

#include <memory>
#include <cstring>   // For memcpy
#include <cstdlib>   // For malloc/free

/*
    CSCN72050 Term Project, Protocol vW25B
    Milestone #1

    The application layer protocol consists of:
      - A Header (4 bytes total):
          • PktCount: unsigned short (2 bytes)
          • Command Flags: bit-fields Drive, Status, Sleep, Ack in 1 byte with 4 bits of padding.
          • Length: unsigned char (1 byte) representing the total number of bytes in the packet.
      - Packet Body:
          • For Drive commands: Contains drive parameters (DriveBody) with:
              - Direction (1 byte): FORWARD (1), BACKWARD (2), RIGHT (3), LEFT (4)
              - Duration (1 byte): Time in seconds to execute command
              - Speed (1 byte): Duty cycle percentage between 80-100
          • For Sleep and Response commands: No body data.
      - Packet Trailer:
          • CRC: 1 byte (unsigned char) parity check calculated by counting the number of bits set to '1'
*/

// Constant definitions for drive directions and header size.
const int FORWARD = 1;
const int BACKWARD = 2;
const int RIGHT = 3;
const int LEFT = 4;
// HEADERSIZE calculated by hand: PktCount (2 bytes) + Command Flags (1 byte) + Length (1 byte) = 4 bytes.
const int HEADERSIZE = 4;

// Packet Header structure definition.
//struct Header {
//    unsigned short PktCount; // Unsigned short integer that increments with each packet transmitted.
//
//    // Command Flags: 1-byte with bit-fields for Drive, Status, Sleep, Ack and 4 bits of padding.
//    union {
//        struct {
//            unsigned Drive : 1; // Set to 1 if the command is a DRIVE command.
//            unsigned Status : 1; // Set to 1 if the command is a telemetry/status response.
//            unsigned Sleep : 1; // Set to 1 if the command is a SLEEP command.
//            unsigned Ack : 1; // Set to 1 if the packet is an acknowledgement.
//            unsigned Padding : 4; // Four bits of padding (should be set to 0).
//        };
//        unsigned char CmdFlags;  // Access all flags as a single byte.
//    };
//
//    unsigned char Length;      // Total number of bytes in the packet (Header + Body + Trailer).
//};

struct Header {
    unsigned short int PktCount; // 2 bytes
    bool Drive : 1;                  // 1 bit
    bool Status : 1;                 // 1 bit
    bool Sleep : 1;                  // 1 bit
    bool Ack : 1;                    // 1 bit
    unsigned char Padding : 4;   // Padding for alignment (4 bits)
    uint8_t Length;   // 1 byte
};

// Drive command parameter structure.
struct DriveBody {
    unsigned char Direction; // 1-byte direction: FORWARD, BACKWARD, RIGHT, or LEFT.
    unsigned char Duration;  // 1-byte duration (in seconds).
    unsigned char Speed;     // 1-byte motor speed percentage (80-100).
};

class PktDef {
public:
    // Enumerated command types.
    enum CmdType { DRIVE, SLEEP, RESPONSE };

    // --- Constructors and Destructor ---

    // Default constructor:
    //   - Initializes Header values to zero.
    //   - Data pointer is set to nullptr.
    //   - CRC is set to zero.
    PktDef();

    // Overloaded constructor:
    //   - Accepts a raw data buffer, parses it, and populates the Header, Body, and CRC.
    PktDef(char* rawBuffer);

    // Destructor:
    //   - Frees any allocated memory for Data and RawBuffer.
    ~PktDef();

    // --- Member Functions ---

    // Sets the command flag in the header based on the provided CmdType.
    void SetCmd(CmdType cmd);

    // Sets the packet body data.
    // Allocates memory for the Data field and copies the provided raw data buffer.
    void SetBodyData(char* data, int size);

    // Sets the packet counter (PktCount) in the header.
    void SetPktCount(int pktCount);

    // Returns the command type based on the header flags.
    CmdType GetCmd() const;

    // Returns true if the Ack flag in the header is set; otherwise, false.
    bool GetAck() const;

    // Returns the packet Length (total number of bytes) from the header.
    int GetLength() const;

    // Returns a pointer to the packet's body data.
    char* GetBodyData() const;

    // Returns the packet counter (PktCount) value.
    int GetPktCount() const;

    // Checks the CRC of a given raw data buffer.
    // Calculates the CRC and compares it with the CRC in the provided buffer.
    // Returns true if they match, otherwise false.
    bool CheckCRC(char* buffer, int size) const;

    // Calculates the 1-byte CRC for the packet (by counting bits set to '1')
    // and updates the packet's CRC field.
    void CalcCRC();

    // Generates a complete serialized packet in RawBuffer.
    // Copies the Header, Data, and CRC into RawBuffer.
    // Returns a pointer to the allocated RawBuffer.
    char* GenPacket();

private:
    // Private structure that defines the complete command packet.
    struct CmdPacket {
        Header header;  // Packet header.
        char* Data;     // Pointer to the packet body data.
        unsigned char CRC; // 1-byte CRC for packet validation.
    } packet;

    // A pointer to a raw data buffer that holds the serialized packet ready for transmission.
    char* RawBuffer;

    // Helper function:
    // Counts the number of bits set to '1' in a given unsigned char.
    int CountBits(unsigned char byte) const;
};

#endif // PKTDEF_H
