#define CROW_MAIN
#include "crow_all.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "../Milestone1/PktDefProject/PktDef.h"      // Milestone 1: PktDef header.
#include "../Milestone2/Milestone2/MySocket.h"          // Milestone 2: MySocket header.

// Create a global MySocket instance in UDP client mode.
// Dummy initial values; these will be updated by the /connect route.
MySocket mySocket(CLIENT, "127.0.0.1", 0, UDP, DEFAULT_SIZE);

int main()
{
    crow::SimpleApp app;

    // Root Route: Serve the main GUI page from "public/index.html".
    CROW_ROUTE(app, "/")
    ([]() {
        std::ifstream in("../public/index.html");
        if (!in) {
            return crow::response(404, "Index file not found");
        }
        std::ostringstream ss;
        ss << in.rdbuf();
        crow::response resp(ss.str());
        resp.set_header("Content-Type", "text/html");
        return resp;
    });

    // Connect Route: Configures the UDP socket with robot IP and port.
    CROW_ROUTE(app, "/connect/<string>/<int>")
    .methods(crow::HTTPMethod::Post)
    ([](const crow::request& req, crow::response& res, std::string robotIP, int robotPort) {
        if (mySocket.configure(robotIP, robotPort)) {
            crow::json::wvalue response;
            response["status"] = "Connected";
            response["robotIP"] = robotIP;
            response["robotPort"] = robotPort;
            res.write(response.dump());
        } else {
            res.code = 500;
            crow::json::wvalue response;
            response["status"] = "Error: Socket configuration failed";
            res.write(response.dump());
        }
        res.end();
    });

    // Telecommand Route: Accepts a PUT request with a JSON payload.
    // Expect payload like:
    // { "command": "drive", "direction": 1, "speed": 90, "duration": 10 }
    // or for sleep:
    // { "command": "sleep" }
    CROW_ROUTE(app, "/telecommand/")
    .methods(crow::HTTPMethod::Put)
    ([](const crow::request& req, crow::response& res) {
        auto body = crow::json::load(req.body);
        if (!body) {
            res.code = 400;
            res.write("Invalid JSON payload");
            res.end();
            return;
        }
        std::string command = body["command"].s();
        std::string packet;
        if (command == "drive") {
            int direction = body["direction"].i();
            int speed = body["speed"].i();
            int duration = body["duration"].i();
            packet = PktDef::createDriveCommand(direction, speed, duration);
        }
        else if (command == "sleep") {
            packet = PktDef::createSleepCommand();
        }
        else {
            res.code = 400;
            res.write("Invalid command specified");
            res.end();
            return;
        }
        if (mySocket.sendPacket(packet)) {
            std::string ack = mySocket.receiveResponse();
            crow::json::wvalue response;
            response["status"] = "Command sent";
            response["ack"] = ack;
            res.write(response.dump());
        } else {
            res.code = 500;
            crow::json::wvalue response;
            response["status"] = "Error sending command";
            res.write(response.dump());
        }
        res.end();
    });

    // Telemetry Request Route: Accepts GET requests.
    // Sends a telemetry request packet and returns telemetry data as JSON.
    CROW_ROUTE(app, "/telementry_request/")
    .methods(crow::HTTPMethod::Get)
    ([](const crow::request& req, crow::response& res) {
        std::string packet = PktDef::createTelemetryRequest();
        if (mySocket.sendPacket(packet)) {
            std::string telemetry = mySocket.receiveResponse();
            crow::json::wvalue response;
            response["status"] = "Telemetry received";
            response["telemetry"] = telemetry;
            res.write(response.dump());
        } else {
            res.code = 500;
            crow::json::wvalue response;
            response["status"] = "Error sending telemetry request";
            res.write(response.dump());
        }
        res.end();
    });

    // Run the webserver on port 23500.
    app.port(23500).multithreaded().run();
    return 0;
}
