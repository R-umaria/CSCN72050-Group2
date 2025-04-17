// Rishi Umaria & Tyler Phillips - Group Project - Milestone 1 - Mobile & Networked Systems

#ifndef PKTDEF_H
#define PKTDEF_H

#include <memory>
#include <cstring>   //for memcpy
#include <cstdlib>   //for malloc/free

/*
    CSCN72050 Term Project, Milestone #1

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

//constant definitions for drive commands and headersize
const int FORWARD = 1;
const int BACKWARD = 2;
const int RIGHT = 3;
const int LEFT = 4;
const int HEADERSIZE = 4; //PktCount (2 bytes) + Command Flags (1 byte) + Length (1 byte) = 4 bytes.

struct Header {
    unsigned short int PktCount;     // 2 bytes
    bool Drive : 1;                  // 1 bit
    bool Status : 1;                 // 1 bit
    bool Sleep : 1;                  // 1 bit
    bool Ack : 1;                    // 1 bit
    unsigned char Padding : 4;       // Padding for alignment (4 bits)
    uint8_t Length;                  // 1 byte
};

//drive command structure
struct DriveBody {
    unsigned char Direction; // 1-byte direction: FORWARD, BACKWARD, RIGHT, or LEFT.
    unsigned char Duration;  // 1-byte duration (in seconds).
    unsigned char Speed;     // 1-byte motor speed percentage (80-100).
};

//packet structure
class PktDef {
private:
    //complete packet
    struct CmdPacket {
        Header header;      // Packet header
        char* Data;         // Pointer to the packet body data
        unsigned char CRC;  // 1-byte CRC for packet validation
    } packet;

    char* RawBuffer;  //holds completed, serialized packet for transmission

    int CountBits(unsigned char byte) const;  //counts # of bits set to '1'

public:
    enum CmdType { DRIVE, SLEEP, RESPONSE };      //command types.

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

    //checks the CRC of a given raw data buffer
    bool CheckCRC(char* buffer, int size) const;

    //calculates CRC
    void CalcCRC();

    //generates serialized packet
    char* GenPacket();
    
    //for debugging telemetry
    std::string Debug() const;
};

#endif // PKTDEF_H
