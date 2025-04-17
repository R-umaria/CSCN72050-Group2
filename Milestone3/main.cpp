// Rishi Umaria & Tyler Phillips - Group Project - Milestone 3 - Mobile & Networked Systems

#define CROW_MAIN
#include "crow_all.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "../Milestone1/PktDefProject/PktDef.h"
#include "../Milestone2/Milestone2/MySocket.h"

MySocket client(CLIENT, "0.0.0.0", 0, UDP, DEFAULT_SIZE);
static int packetCount = 0;  //global packet counter

int main() {
    crow::SimpleApp app;

    //root route
    CROW_ROUTE(app, "/")([](const crow::request& req, crow::response& res) {
        std::ifstream in("../public/index.html");   //loads index.html
        if (in) {
            std::ostringstream contents;
            contents << in.rdbuf();
            in.close();
            res.set_header("Content-Type", "text/html");
            res.write(contents.str());
        } else {
            res.code = 404; //if can't find/read file
            res.write("Not Found");
        }
        res.end();
    });

    //receives connection information/sets up UDP connection
    CROW_ROUTE(app, "/connect/<string>/<int>").methods(crow::HTTPMethod::POST)  //only POST
    ([](const crow::request& req, crow::response& res, std::string ip, int port) {
        if (client.GetUDPSocket() == -1) {  //if UDP socket doesn't exist
            client = MySocket(CLIENT, ip, port, UDP, DEFAULT_SIZE);
        } else {    //otherwise, change ip and port
            client.SetIPAddr(ip);
            client.SetPort(port);
        }
        std::ifstream in("../public/command.html"); //loads commands page
        if (in) {
            std::ostringstream contents;
            contents << in.rdbuf();
            in.close();
            res.set_header("Content-Type", "text/html");
            res.write(contents.str());
        } else {    //if can't find/read file
            res.code = 404;
            res.write("Not Found");
        }
        res.end();
    });

    //sends sleep style command to robot
    CROW_ROUTE(app, "/telecommand/").methods(crow::HTTPMethod::PUT) //only PUT
    ([](const crow::request& req, crow::response& res) {
        //prep packet
        PktDef pkt;
        pkt.SetCmd(PktDef::SLEEP);
        pkt.SetPktCount(++packetCount);
        pkt.SetBodyData(nullptr, 0);

        //generate packet and send it
        char* raw = pkt.GenPacket();
        client.SendData(raw, pkt.GetLength());

        //receive response and parse it
        char buff[1024];
        int bytesReceived = client.GetData(buff);
        PktDef resp(buff);
        if (bytesReceived > 0 && resp.GetAck()) {   //if valid bytes and ack received
            res.write("Robot is now sleeping.\n");
        } else {    //if couldn't parse
            res.code = 500;
            res.write("Sleep command not acknowledged.\n");
        }
        res.end();
    });

    //sends drive style command to robot
    CROW_ROUTE(app, "/telecommand/<int>/<int>/<int>").methods(crow::HTTPMethod::PUT)    //only PUT
    ([](const crow::request& req, crow::response& res, int direction, int duration, int speed) {
        //prep packet
        PktDef pkt;
        pkt.SetCmd(PktDef::DRIVE);
        pkt.SetPktCount(++packetCount);

        //build drive body
        DriveBody body;
        body.Direction = (unsigned char)direction;
        body.Duration = (unsigned char)duration;
        body.Speed = (unsigned char)speed;

        //set body data
        char driveData[sizeof(DriveBody)];
        memcpy(driveData, &body, sizeof(DriveBody));
        pkt.SetBodyData(driveData, sizeof(DriveBody));

        //generate packet and send it
        char* raw = pkt.GenPacket();
        client.SendData(raw, pkt.GetLength());

        //receive response and parse it
        char buff[1024];
        int bytesReceived = client.GetData(buff);
        PktDef resp(buff);
        if (bytesReceived > 0 && resp.GetAck()) {   //if valid bytes and ack received
            std::string dirStr = (direction == 1) ? "FORWARD" : (direction == 2) ? "BACKWARD" : (direction == 3) ? "RIGHT" : "LEFT";
            std::ostringstream os;
            os << "Robot is now driving " << dirStr << " for " << duration << " seconds at " << speed << "% speed.\n";
            res.write(os.str());
        } else {    //if couldnt parse
            res.code = 500;
            res.write("Drive command not acknowledged.\n");
        }
        res.end();
    });

    //sends telemetry request to robot and prints response
    CROW_ROUTE(app, "/telemetry_request/").methods(crow::HTTPMethod::GET)   //only GET
    ([](const crow::request& req, crow::response& res) {
        //prep packet
        PktDef pkt;
        pkt.SetCmd(PktDef::RESPONSE);
        pkt.SetPktCount(++packetCount);
        pkt.SetBodyData(nullptr, 0);

        //generate packet and send it
        char* raw = pkt.GenPacket();
        client.SendData(raw, pkt.GetLength());
    
        //receive acknowledgement and parse it
        char buff[1024];
        int bytesReceived = client.GetData(buff);
        if (bytesReceived < 5) {    //if invalid header
            res.write("NACK received.\n");
        
        }
        else {
            bytesReceived = client.GetData(buff);   //receives telemetry data
        }

        try {
            PktDef resp(buff);
            std::ostringstream os;
            os << "Telemetry Response:\n";
            os << resp.Debug();

            char* rawBody = resp.GetBodyData();
            int bodyLen = resp.GetLength() - HEADERSIZE - 1;    //get data size
    
            if (rawBody == nullptr || bodyLen <= 0) {
                os << "\nReceived telemetry ACK, but no body data provided.\n";
                res.write(os.str());
                res.end();
                return;
            }

            unsigned char* data = reinterpret_cast<unsigned char*>(rawBody);    //convert raw data
            // for (int i = 0; i < bodyLen; ++i) {
            //     os << "Byte[" << i << "] = " << std::hex << std::showbase << static_cast<int>(data[i]) << "\n";
            // }
            
            //decode enums for better readability
            std::string cmdName;
            switch (data[5]) {
                case 0: cmdName = "DRIVE"; break;
                case 1: cmdName = "SLEEP"; break;
                case 2: cmdName = "RESPONSE"; break;
                default: cmdName = "UNKNOWN"; break;
            }

            std::string direction;
            switch (data[6]) {
                case 1: direction = "FORWARD"; break;
                case 2: direction = "BACKWARD"; break;
                case 3: direction = "RIGHT"; break;
                case 4: direction = "LEFT"; break;
                default: direction = "UNKNOWN"; break;
            }

            int lastPktCount = (data[1] << 8) | data[0];
            int currentGrade = (data[3] << 8) | data[2];
            int hitCount = static_cast<int>(data[4]);
            int lastCmdSpeed = static_cast<int>(data[8]);

            //reset to decimal formatting
            os << std::dec;

            os << "\nDecoded Telemetry:\n";
            os << " - LastPktCounter: " << lastPktCount << "\n";
            os << " - CurrentGrade: " << currentGrade << "\n";
            os << " - HitCount: " << hitCount << "\n";
            os << " - LastCmd: " << cmdName << ", ";
            os << direction << ", ";
            os << "Speed: " << lastCmdSpeed << "%\n";

            res.set_header("Content-Type", "text/plain");
            res.write(os.str());
        } catch (...) { //if couldn't parse
            res.code = 500;
            res.write("Error parsing telemetry response.\n");
        }
        res.end();
    });

    app.port(23500).multithreaded().run();
    return 0;
}
