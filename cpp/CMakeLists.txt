cmake_minimum_required(VERSION 3.15)
project(zenoh_ros2dds_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Boost.PFR
FetchContent_Declare(
    pfr
    GIT_REPOSITORY https://github.com/boostorg/pfr.git
    GIT_TAG        2.2.0
)
FetchContent_MakeAvailable(pfr)

# Executables
add_executable(publisher publisher.cpp)
add_executable(subscriber subscriber.cpp)
add_executable(client client.cpp)
add_executable(server server.cpp)

# All targets
set(ALL_TARGETS publisher subscriber client server)

# Link Boost.PFR
foreach(target ${ALL_TARGETS})
    target_include_directories(${target} PRIVATE ${pfr_SOURCE_DIR}/include)
endforeach()

# zenoh-c
find_package(zenohc QUIET)
if(zenohc_FOUND)
    foreach(target ${ALL_TARGETS})
        target_link_libraries(${target} zenohc::lib)
    endforeach()
else()
    find_library(ZENOH_LIB NAMES zenohc zenoh PATHS /usr/local/lib /usr/lib)
    find_path(ZENOH_INCLUDE zenoh.h PATHS /usr/local/include /usr/include)
    if(ZENOH_LIB AND ZENOH_INCLUDE)
        foreach(target ${ALL_TARGETS})
            target_include_directories(${target} PRIVATE ${ZENOH_INCLUDE})
            target_link_libraries(${target} ${ZENOH_LIB} pthread)
        endforeach()
else()
        message(FATAL_ERROR "zenoh-c not found, please install: sudo apt install libzenohc-dev")
    endif()
endif()
