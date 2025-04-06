// main.cpp -- is a temp file to test the PktDef class
#include <iostream>
#include "PktDef.h"

using namespace std;

int main() {
    PktDef packet;

    packet.SetPktCount(1);
    packet.SetCmd(DRIVE);

    char driveData[] = { FORWARD, 5, 90 }; // Direction, Duration, Speed
    packet.SetBodyData(driveData, sizeof(driveData));

    char* rawPacket = packet.GenPacket();
    if (packet.CheckCRC(rawPacket, packet.GetLength())) {
        cout << "Packet generated and CRC is valid." << endl;
    }
    else {
        cout << "CRC validation failed." << endl;
    }

    delete[] rawPacket;
    return 0;
}
