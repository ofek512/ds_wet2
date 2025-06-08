cmake_minimum_required(VERSION 3.10)
project(DSpotify VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files
set(SOURCES
        dspotify25b2.cpp
)

# Header files
set(HEADERS
        dspotify25b2.h
        wet2util.h
)

# Create executable
add_executable(dspotify ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(dspotify PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Enable warnings
if (MSVC)
    target_compile_options(dspotify PRIVATE /W4)
else ()
    target_compile_options(dspotify PRIVATE -Wall -Wextra -pedantic)
endif ()

# Set debug configuration for development
set(CMAKE_BUILD_TYPE Debug)