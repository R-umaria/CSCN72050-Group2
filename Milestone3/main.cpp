#define CROW_MAIN

#include "crow_all.h"
#include <iostream>
using namespace std;

MySocket client(CLIENT, "127.0.0.1", 0, UDP, DEFAULT_SIZE);

int main()
{
	crow::SimpleApp app;

	CROW_ROUTE(app, "/")
	([](const crow::request& req, crow::response& res) {
		ifstream in("../public/index.html", ifstream::in);
		if (in) {
			ostringstream contents;
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
		([](const crow::request& req, crow::response& res, string ip, int port) {
		
		//set internal parameters to be used by UDP/IP communications
		if (client == INVALID_SOCKET) {
			MySocket client(CLIENT, ip, port, UDP, DEFAULT_SIZE);
		}
		else {
			client.SetIPAddr(ip);
			client.SetPort(port);
		}

		//needs to send back a response to the html browser
		res.set_header("Content-Type", "text/html");

		ostringstream os;
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
		pkt.SetCmd(SLEEP);
		pkt.SetPktCount(pkt.GetPktCount() + 1);

		//return error if invalid
		res.code = 500;
		res.write("Unable to create packet");
		res.end();

		char* raw = pkt.GenPacket();

		res.end();
			});

	//sends drive style command to robot
	CROW_ROUTE(app, "/telecommand/<int>/<int>/<int>").methods(crow::HTTPMethod::PUT)	//only PUT
		([](const crow::request& req, crow::response& res, int direction, int speed, int duration) {
		
		PktDef pkt;
		pkt.SetCmd(DRIVE);
		pkt.SetPktCount(pkt.GetPktCount() + 1);

		//build drive body
		DriveBody body;
		body.Direction = direction;
		body.Duration = duration;
		body.Speed = speed;

		char driveData[sizeof(DriveBody)];
		std::memcpy(driveData, &body, sizeof(DriveBody));
		pkt.SetBodyData(driveData, sizeof(DriveBody));

		//return error if invalid

		char* raw = pkt.GenPacket();

		res.end();
			});

	CROW_ROUTE(app, "/telemetry_request/").methods(crow::HTTPMethod::GET)	//only GET
		([](const crow::request& req, crow::response& res, string fileName) {
		//sends request for housekeeping telemetry

		PktDef pkt;
		pkt.SetCmd(RESPONSE);
		pkt.SetPktCount(pkt.GetPktCount() + 1);

		char* raw = pkt.GenPacket();

		res.end();
			});

	
	//example
	CROW_ROUTE(app, "/add-brick/<int>/<string>").methods(crow::HTTPMethod::Post,
		crow::HTTPMethod::Get, crow::HTTPMethod::Put)
		([](const crow::request& req, crow::response& res, int quantity, std::string product) {
		std::ostringstream contents;

		std::ifstream in("../cart.txt");
		if (!isFileEmpty(in)) {
			contents << in.rdbuf();
			in.close();
		}

		contents << quantity << "," << product << "\n";
		std::ofstream newCart("../cart.txt");
		if (newCart) {
			newCart << contents.str();
			newCart.close();
			res.write("Product added to cart.");
		}
		else {
			res.code = 500;
			res.write("Error: Unable to update cart.");
			res.end();
		}
		res.end();
			});

		app.port(23500).multithreaded().run();
	return 0;
}