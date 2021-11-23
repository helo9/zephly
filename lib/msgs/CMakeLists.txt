cmake_minimum_required(VERSION 3.20)

project(msgs)

add_library(msgs INTERFACE)
target_include_directories(msgs INTERFACE .)
