Robot Controller Web Server – README.txt
========================================

Project Overview
----------------
This project implements a simple web server (using Crow) that communicates with a robot simulator over UDP/TCP
to send commands (drive, sleep) and request telemetry data. The server serves static HTML pages for UI and
exposes REST endpoints for telecommands and telemetry.

Team
----
Group 2:
- Rishi Umaria
- Tyler Phillips

Folder Structure
----------------
robot_controller/
├─ Milestone1/  
│   └─ PktDef.cpp, PktDef.h, …  
│   (Defines packet format, CRC, serialization, deserialization)  
├─ Milestone2/  
│   └─ MySocket.cpp, MySocket.h, …  
│   (Implements TCP/UDP socket wrapper, send/receive)  
└─ Milestone3/  
    ├─ build/  
    │   (CMake-generated build directory)  
    ├─ public/  
    │   ├─ index.html        ← Landing page  
    │   └─ command.html      ← Command UI  
    ├─ main.cpp             ← Crow-based web server  
    └─ CMakeLists.txt  

Prerequisites
-------------
- Docker (with Linux container support)  
- C++ build tools inside container (g++, cmake, make)  

Building & Running in Docker
----------------------------
1. **Launch Docker container**

```docker run -ti
-p 23500:23500
-v "C:/Users/Rishi/source/repos/Mobile and Networked Systems/Robot__Simulator/RobotController/:/mnt/robot_controller"
webserver_dev_updated bash``` --this was used for development and its local

2. **Install build tools**  
```
apt-get update && \
apt-get install -y g++ cmake make```
Build & run the web server

```cd /mnt/robot_controller/Milestone3/build```
```cmake .. && make```
```./Milestone3```

Access the UI
Open your browser to:

```http://<docker-host-ip>:23500/```
Landing page: /

Connect endpoint (POST): /connect/<IP>/<port>

Sleep command (PUT): /telecommand/

Drive command (PUT): /telecommand/<direction>/<duration>/<speed>

Telemetry request (GET): /telemetry_request/

Notes
Ensure your robot simulator /robot is reachable at the IP/port you POST to /connect/.

The container maps port 23500 on the host to the Crow server inside Docker.

Static files (index.html, command.html) are served from public/.

Packet definitions and socket code live in Milestone1/ and Milestone2/ respectively.
