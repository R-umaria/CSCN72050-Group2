cmake_minimum_required(VERSION 3.7)
project(Milestone3)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(Threads)

# Add include directories for Boost, and for Milestone1/Milestone2 headers.
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/../Milestone1)   # For PktDef headers.
include_directories(${CMAKE_SOURCE_DIR}/../Milestone2)   # For MySocket headers.
include_directories(${CMAKE_SOURCE_DIR})                # For crow_all.h and local headers.

# Create the executable and include the additional source files.
add_executable(Milestone3 
    main.cpp
    ../Milestone1/PktDefProject/PktDef.cpp
    ../Milestone2/Milestone2/MySocket.cpp
)
target_link_libraries(Milestone3 ${Boost_LIBRARIES} Threads::Threads)
