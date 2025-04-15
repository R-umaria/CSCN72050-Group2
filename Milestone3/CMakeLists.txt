cmake_minimum_required(VERSION 3.7)
project(Milestone3)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(Threads)

# Add include directories for PktDef and MySocket.
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/../Milestone1)   # Adjust according to your actual structure.
include_directories(${CMAKE_SOURCE_DIR}/../Milestone2)
include_directories(${CMAKE_SOURCE_DIR})                # Include current folder (for crow_all.h)

add_executable(Milestone3 main.cpp)
target_link_libraries(Milestone3 ${Boost_LIBRARIES} Threads::Threads)
