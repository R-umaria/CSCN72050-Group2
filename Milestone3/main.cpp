#define CROW_MAIN
#include "crow_all.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "PktDef.h"      // Include Milestone 1 code.
#include "MySocket.h"    // Include Milestone 2 code.

// Create a global MySocket instance in UDP client mode.
// (Provide dummy initial values; these will be updated by /connect.)
MySocket mySocket(CLIENT, "127.0.0.1", 0, UDP, DEFAULT_SIZE);

int main()
{
    crow::SimpleApp app;

    // Root Route: Serve the main GUI page.
    CROW_ROUTE(app, "/")
    ([]() {
        std::ifstream in("public/index.html");
        if (!in) {
            return crow::response(404, "Index file not found");
        }
        std::ostringstream ss;
        ss << in.rdbuf();
        crow::response resp(ss.str());
        resp.set_header("Content-Type", "text/html");
        return resp;
    });

    // Connect Route: Configure the socket with robot IP and port.
    CROW_ROUTE(app, "/connect/<string>/<int>")
    .methods(crow::HTTPMethod::Post)
    ([](const crow::request& req, crow::response& res, std::string robotIP, int robotPort) {
        if (mySocket.configure(robotIP, robotPort)) {
            crow::json::wvalue response;
            response["status"] = "Connected";
            response["robotIP"] = robotIP;
            response["robotPort"] = robotPort;
            res.write(crow::json::dump(response));
        }
        else {
            res.code = 500;
            crow::json::wvalue response;
            response["status"] = "Error: Socket configuration failed";
            res.write(crow::json::dump(response));
        }
        res.end();
    });

	CROW_ROUTE(app, "/connect/<string>/<int>").methods(crow::HTTPMethod::POST)	//only POST
		([](const crow::request& req, crow::response& res, string ip, int port) {
		
		if (!robotSocket) {
			MySocket robotSocket(CLIENT, ip, port, UDP);
		}
		else {
			robotSocket->SetIPAddr(ip);
			robotSocket->SetPort(port);
		}

		ostringstream contents;
		contents << in.rdbuf();
		in.close();

		res.set_header("Content-Type", "text/html");
		res.write(contents.str());

		res.write("Not Found");
		res.end();
			});

	CROW_ROUTE(app, "/telecommand/").methods(crow::HTTPMethod::PUT)	//only PUT
		([](const crow::request& req, crow::response& res, string fileName) {
		ifstream in("../public/images/" + fileName, ifstream::in);
		if (in) {
			ostringstream contents;
			contents << in.rdbuf();
			in.close();

			res.set_header("Content-Type", "image/png");
			res.write(contents.str());
		}
		else {
			res.code = 404;
			res.write("Not Found");
		}
		res.end();
			});

	CROW_ROUTE(app, "/telemetry_request/").methods(crow::HTTPMethod::GET)	//only GET
		([](const crow::request& req, crow::response& res, string fileName) {
		ifstream in("../public/scripts/" + fileName, ifstream::in);
		if (in) {
			ostringstream contents;
			contents << in.rdbuf();
			in.close();

			res.set_header("Content-Type", "application/javascript");
			res.write(contents.str());
		}
		else {
			res.write("Not Found");
		}
		res.end();
			});

    // Telemetry Request Route: Sends a telemetry request and returns the result.
    CROW_ROUTE(app, "/telementry_request/")
    .methods(crow::HTTPMethod::Get)
    ([](const crow::request& req, crow::response& res) {
        std::string packet = PktDef::createTelemetryRequest();
        if (mySocket.sendPacket(packet)) {
            std::string telemetry = mySocket.receiveResponse();
            crow::json::wvalue response;
            response["status"] = "Telemetry received";
            response["telemetry"] = telemetry;
            res.write(crow::json::dump(response));
        }
        else {
            res.code = 500;
            crow::json::wvalue response;
            response["status"] = "Error sending telemetry request";
            res.write(crow::json::dump(response));
        }
        res.end();
    });

    // Remove or comment out any extraneous routes (example route removed).

    // Run the server on port 23500.
    app.port(23500).multithreaded().run();
    return 0;
}
