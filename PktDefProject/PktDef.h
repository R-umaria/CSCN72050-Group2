// PktDef.h
#ifndef PktDef_H
#define PktDef_H

#include <iostream>
#include <cstring>

enum CmdType { DRIVE, SLEEP, RESPONSE };

// Constants
const int FORWARD = 1;
const int BACKWARD = 2;
const int LEFT = 3;
const int RIGHT = 4;
const int HEADERSIZE = 7; // Calculation: PktCount (2 bytes) + Flags (1 byte) + Length (2 bytes) + Padding (4 bits) = 7 bytes

class PktDef {
private:
    struct Header {
        unsigned short int PktCount; // 2 bytes
        bool Drive;                  // 1 bit
        bool Status;                 // 1 bit
        bool Sleep;                  // 1 bit
        bool Ack;                    // 1 bit
        unsigned char Padding : 4;   // Padding for alignment (4 bits)
        unsigned short int Length;   // 2 bytes
    };

    struct DriveBody {
        unsigned char Direction;     // 1 byte
        unsigned char Duration;      // 1 byte
        unsigned char Speed;         // 1 byte
    };

    Header header;
    DriveBody driveBody;
    char* Data;
    unsigned char CRC;
    char* RawBuffer;

public:
    // Constructors and Destructor
    PktDef();
    PktDef(char*);
    ~PktDef();

    // Setters
    void SetCmd(CmdType);
    void SetBodyData(char*, int);
    void SetPktCount(int);

    // Getters
    CmdType GetCmd();
    bool GetAck();
    int GetLength();
    char* GetBodyData();
    int GetPktCount();

    // CRC Functions
    bool CheckCRC(char*, int);
    void CalcCRC();

    // Generate Packet
    char* GenPacket();
};

#endif
