cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0005 OLD)
endif(COMMAND cmake_policy)

project("xviweb")
set(PROJECT_VERSION "0.1.0")

message(STATUS "Using gcc warning options")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wshadow -DPROJECT_VERSION='\"${PROJECT_VERSION}\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wshadow -DPROJECT_VERSION='\"${PROJECT_VERSION}\"'")

include_directories(include)
subdirs(src)
