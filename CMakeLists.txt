#user values
set(Zishan 1)
set(OPT "-O2")
set(USER_LINKER_FLAGS "-u _printf_float")
set(USER_COMPILER "GNU") # "Clang" or "GNU"
set(USER_DEBUG TRUE) # Or "False"


cmake_minimum_required (VERSION 3.3)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
if(POLICY CMP0012)
    cmake_policy(SET CMP0012 NEW)
endif()

project(osfi-z)
set(CMAKE_C_COMPILER_ID ${USER_COMPILER})
if(${USER_DEBUG})
  set(CMAKE_BUILD_TYPE "Debug")
else(${USER_DEBUG})
  set(CMAKE_BUILD_TYPE "Release")
endif(${USER_DEBUG})
include("CMakeSetCompiler.cmake")


add_subdirectory (hw)
message("FLLLAGS ${CMAKE_C_FLAGS}")
add_subdirectory (osfi-z)
