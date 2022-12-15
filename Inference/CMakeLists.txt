cmake_minimum_required(VERSION 3.24)
project(Inference)

set(CMAKE_CXX_STANDARD 17)

add_executable(Inference main.cpp zeroDCE.cpp zeroDCE.h zerodce_memAlloc.h qZeroDCE_Weight.h)
