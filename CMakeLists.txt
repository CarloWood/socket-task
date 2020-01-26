cmake_minimum_required(VERSION 3.12...3.16)

project(socket-task
  DESCRIPTION "AIStatefulTask submodule - asynchronous (TLS) sockets support"
  LANGUAGES CXX
)

include(AICxxProject)

#==============================================================================
# BUILD PROJECT
#

# This project is an OBJECT-library, used by other git submodules and the main project.
add_library(socket-task_ObjLib OBJECT)

# Set SOURCE_DIR to the current directory.
set_property(
  TARGET socket-task_ObjLib
  PROPERTY SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

# The list of source files.
target_sources(socket-task_ObjLib
  PRIVATE
    "AIEndPoint.cxx"
    "ConnectToEndPoint.cxx"

    "AIEndPoint.h"
    "ConnectToEndPoint.h"
)

# Required include search-paths.
get_target_property(CWDS_INTERFACE_INCLUDE_DIRECTORIES AICxx::cwds INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(socket-task_ObjLib
  PUBLIC
    "${CWDS_INTERFACE_INCLUDE_DIRECTORIES}" # For sys.h and debug.h.
)

# Require support for C++17.
target_compile_features(socket-task_ObjLib
  PUBLIC cxx_std_17
)

# Set link dependencies.
target_link_libraries(socket-task_ObjLib
  PUBLIC
    AICxx::statefultask
)

# Create an ALIAS target.
add_library(AICxx::socket-task ALIAS socket-task_ObjLib)
