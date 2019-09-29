cmake_minimum_required(VERSION 3.14)
project(p1)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(p1
        args.c
        inf.c
        Makefile
        sample.c)
