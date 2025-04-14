Mobile and Networked Systems Group Project - Milestone 2
=========================================================

Project Overview
----------------
This project implements the MySocket class as required for Milestone 2.
The MySocket class supports both TCP and UDP socket communications and is designed 
to be instantiated as either a client or server connection.

Key features include:
- Enumerations for SocketType (CLIENT, SERVER) and ConnectionType (TCP, UDP).
- A dynamically allocated raw buffer for data transmission.
- Methods to connect and disconnect TCP sockets (using a 3-way handshake and 4-way handshake respectively).
- Functions for sending and receiving raw data in both TCP and UDP modes.
- Getters and setters for IP address, port number, and socket type, with error prevention when a connection is active.
- Winsock initialization has been moved out of the MySocket class and performed once in main() as per best practices.

Project Structure
-----------------
The project is organized as follows:
- **MySocket.h & MySocket.cpp**: Contains the implementation of the MySocket class.
- **main.cpp**: A temporary test harness that demonstrates basic TCP server/client functionality. 
  (Note: main.cpp is only for temporary/manual testing and is not part of the final user interface.)
- **MySocketTests**: Contains MSTest unit tests with approximately 90% code coverage, validating both normal operation and error-handling paths.
- **Visual Studio Project Files**: All necessary project and solution files are included. Debug, Release, and .vs directories have been removed.

Build Instructions
------------------

Steps to build:
1. Open the provided solution (.sln) file in Visual Studio.
2. Build the solution (choose either Debug or Release configuration).
3. Run the MSTest unit tests via Test Explorer to verify proper functionality.
4. Optionally, run the temporary main.cpp executable for manual testing of the socket connection.

Execution Instructions
----------------------
To manually test the MySocket functionality:
1. Open a command prompt (or use Visual Studio’s debugger).
2. Run the built executable ("Milestone 2.exe").
3. Follow the on-screen prompts:
   - Enter "1" to run in Server mode or "2" for Client mode.
   - When running as a Client, you’ll be prompted to enter a message to send.
   
For full integration (Milestone 3), you will later run the Robot_Simulator.exe alongside your Command and Control GUI.

Additional Information
----------------------
- Winsock Initialization: The application now initializes Winsock once in main.cpp before any MySocket objects are created, and cleans up at the end.
- Error Handling: The code includes error prevention logic to avoid modifying socket parameters during an active connection. The error paths have been extensively tested.
- Future Enhancements: The main.cpp test harness is temporary and will be replaced or removed as the Command and Control GUI is implemented in Milestone 3.
- For any issues or questions, please contact the development team.

Credits
-------
Developed by:
Group-2/RISHI UMARIA && TYLER PHILLIPS

Submission
----------
This submission includes:
- All source code files (MySocket.h, MySocket.cpp, main.cpp, etc.)
- MSTest project files and unit tests.
- This README.txt file.
- Necessary Visual Studio project and solution files.
  
Please ensure that any temporary directories (Debug, Release, .vs) are not included in the final ZIP.

---------------------------------------------------------
