cmake_minimum_required(VERSION 3.23)
project(os C)

set(CMAKE_C_STANDARD 11)

add_executable(os main.c tools/floppy.c tools/fileinfo.c)
