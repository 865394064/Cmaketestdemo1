cmake_minimum_required(VERSION 3.15.0)
project(test1)
 
file(GLOB ALL_SOURCES *.cpp *.h)
 
add_executable(${PROJECT_NAME} ${ALL_SOURCES})