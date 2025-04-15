#define CROW_MAIN
#include "crow_all.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "../Milestone1/PktDefProject/PktDef.h"      // Milestone 1: Packet definitions.
#include "../Milestone2/Milestone2/MySocket.h"          // Milestone 2: Socket communication.

MySocket client(CLIENT, "0.0.0.0", 0, UDP, DEFAULT_SIZE);

int main()
{
	crow::SimpleApp app;

	CROW_ROUTE(app, "/")
		([](const crow::request& req, crow::response& res) {
		std::ifstream in("../public/index.html", std::ifstream::in);
		if (in) {
			std::ostringstream contents;
			contents << in.rdbuf();
			in.close();

			res.set_header("Content-Type", "text/html");
			res.write(contents.str());
		}
		else {
			res.write("Not Found");
		}
		res.end();
			});

	//receives connection information/sets up UDP connection
	CROW_ROUTE(app, "/connect/<string>/<int>").methods(crow::HTTPMethod::POST)	//only POST
		([](const crow::request& req, crow::response& res, std::string ip, int port) {

		//set internal parameters to be used by UDP/IP communications
		if (client.GetUDPSocket() == -1) {  // Check if the underlying socket file descriptor is -1 (invalid)
			// Re-create the client object
			client = MySocket(CLIENT, ip, port, UDP, DEFAULT_SIZE);
		}
		else {
			client.SetIPAddr(ip);
			client.SetPort(port);
		}
		std::cout << "Successfuly created UDP client socket" << std::endl;
		//needs to send back a response to the html browser
		res.set_header("Content-Type", "text/html");

		std::ostringstream os;
		os << "Socket connected, IP: " << ip << " Port: " << port << std::endl;
		res.code = 200;
		res.write(os.str());

		res.end();
			});

	//sends sleep style command to robot
	CROW_ROUTE(app, "/telecommand/").methods(crow::HTTPMethod::PUT)	//only PUT
		([](const crow::request& req, crow::response& res) {

		//construct packet
		PktDef pkt;
		pkt.SetCmd(PktDef::SLEEP);
		pkt.SetPktCount(pkt.GetPktCount() + 1);

		char* raw = pkt.GenPacket();

		client.SendData(raw, pkt.GetLength());
		//receive acknowledgement
		char buff[1024];
		int bytesReceived = client.GetData(buff);
		if (bytesReceived > 0) {
			std::string response(buff, bytesReceived);
			if (response == "ACK") {
				res.set_header("Content-Type", "text/plain");
				res.write("Robot is now sleeping.\n");
			}
			else if (response == "NACK") {
				res.code = 500;
				res.write("Packet not acknowledged.\n");
			}
		}
		else {
			res.code = 500;
			res.write("No response received.\n");
		}

		res.end();
			});

	//sends drive style command to robot
	CROW_ROUTE(app, "/telecommand/<int>/<int>/<int>").methods(crow::HTTPMethod::PUT)	//only PUT
		([](const crow::request& req, crow::response& res, int direction, int duration, int speed) {

		PktDef pkt;
		pkt.SetCmd(PktDef::DRIVE);
		pkt.SetPktCount(pkt.GetPktCount() + 1);

		//build drive body
		DriveBody body;
		body.Direction = direction;
		body.Duration = duration;
		body.Speed = speed;

		char driveData[sizeof(DriveBody)];
		std::memcpy(driveData, &body, sizeof(DriveBody));
		pkt.SetBodyData(driveData, sizeof(DriveBody));

		char* raw = pkt.GenPacket();

		client.SendData(raw, pkt.GetLength());
		//receive acknowledgement
		char buff[1024];
		int bytesReceived = client.GetData(buff);
		if (bytesReceived > 0) {
			std::string response(buff, bytesReceived);
			if (response == "ACK") {
				res.set_header("Content-Type", "text/plain");

				std::string dir;
				if (direction == 1) {
					dir = "FORWARD";
				}
				else if (direction == 2) {
					dir = "BACKWARD";
				}
				else if (direction == 3) {
					dir = "RIGHT";
				}
				else if (direction == 4) {
					dir = "LEFT";
				}
				std::ostringstream os;
				os << "Robot is now driving " << dir << " for " << duration << " seconds at " << speed << " speed.\n";

				res.write(os.str());
			}
			else if (response == "NACK") {
				res.code = 500;
				res.write("Packet not acknowledged.\n");
			}
		}
		else {
			res.code = 500;
			res.write("No response received.\n");
		}
		res.end();
			});

	//sends request for housekeeping telemetry
	CROW_ROUTE(app, "/telemetry_request/").methods(crow::HTTPMethod::GET)	//only GET
		([](const crow::request& req, crow::response& res) {

		PktDef pkt;
		pkt.SetCmd(PktDef::RESPONSE);
		pkt.SetPktCount(pkt.GetPktCount() + 1);

		char* raw = pkt.GenPacket();

		//receive acknowledgement
		char buff[1024];
		int bytesReceived = client.GetData(buff);
		if (bytesReceived > 0) {
			std::string response(buff, bytesReceived);
			if (response == "ACK") {
				res.set_header("Content-Type", "text/plain");
				res.write("Robot is now sleeping.\n");
			}
			else if (response == "NACK") {
				res.code = 500;
				res.write("Packet not acknowledged.\n");
			}
		}
		else {
			res.code = 500;
			res.write("No response received.\n");
		}

		res.end();
			});

	app.port(23500).multithreaded().run();
	return 0;
}