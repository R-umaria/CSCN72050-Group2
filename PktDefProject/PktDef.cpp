// PktDef.cpp

#include "PktDef.h"

PktDef::PktDef() {
    header.PktCount = 0;
    header.Drive = 0;
    header.Status = 0;
    header.Sleep = 0;
    header.Ack = 0;
    header.Length = 0;
    Data = nullptr;
    CRC = 0;
    RawBuffer = nullptr;
}

PktDef::PktDef(char* rawData) {
    if (rawData == nullptr) return;

    header = *(Header*)rawData; // Deserialize the header
    header.Length = *(unsigned short int*)(rawData + 3); // Extract length from rawData

    if (header.Drive) {
        driveBody = *(DriveBody*)(rawData + HEADERSIZE); // Extract DriveBody if it's a drive command
    }

    CRC = rawData[header.Length - 1]; // Extract CRC
}

PktDef::~PktDef() {
    delete[] Data;
    delete[] RawBuffer;
}

void PktDef::SetCmd(CmdType cmdType) {
    header.Drive = (cmdType == DRIVE);
    header.Sleep = (cmdType == SLEEP);
    header.Status = (cmdType == RESPONSE);
    header.Ack = false;
}

void PktDef::SetPktCount(int count) {
    header.PktCount = count;
}

void PktDef::SetBodyData(char* data, int size) {
    Data = new char[size];
    std::memcpy(Data, data, size);
    header.Length = HEADERSIZE + size + 1; // Including CRC byte
}

CmdType PktDef::GetCmd() {
    if (header.Drive) return DRIVE;
    if (header.Sleep) return SLEEP;
    if (header.Status) return RESPONSE;
    return DRIVE; // Default fallback
}

bool PktDef::GetAck() {
    return header.Ack;
}

int PktDef::GetLength() {
    return header.Length;
}

char* PktDef::GetBodyData() {
    return Data;
}

int PktDef::GetPktCount() {
    return header.PktCount;
}

bool PktDef::CheckCRC(char* buffer, int size) {
    int count = 0;
    for (int i = 0; i < size; ++i) {
        char byte = buffer[i];
        while (byte) {
            count += byte & 1;
            byte >>= 1;
        }
    }
    return (count % 2 == 0);
}

void PktDef::CalcCRC() {
    CRC = 0;
    int count = 0;
    for (int i = 0; i < header.Length - 1; ++i) {
        char byte = RawBuffer[i];
        while (byte) {
            count += byte & 1;
            byte >>= 1;
        }
    }
    CRC = (count % 2 == 0) ? 0 : 1;
}

char* PktDef::GenPacket() {
    delete[] RawBuffer; // Prevent memory leaks
    RawBuffer = new char[header.Length];

    std::memcpy(RawBuffer, &header, sizeof(Header));
    std::memcpy(RawBuffer + sizeof(Header), Data, header.Length - HEADERSIZE - 1);
    CalcCRC();
    RawBuffer[header.Length - 1] = CRC;

    return RawBuffer;
}
