#define CROW_MAIN

#include "crow_all.h"
#include <iostream>
using namespace std;

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

	CROW_ROUTE(app, "/connect/<string>/<int>")	//only POST
		([](const crow::request& req, crow::response& res, string fileName) {
		ifstream in("../public/" + fileName, ifstream::in);
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

	CROW_ROUTE(app, "/telecommand/")	//only PUT
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

	CROW_ROUTE(app, "/telemetry_request/")	//only GET
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