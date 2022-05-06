cmake_minimum_required(VERSION 3.17)
project(Server C)

set(CMAKE_C_STANDARD 11)

add_executable(Server Server.c HandleClient.c Employee.h)