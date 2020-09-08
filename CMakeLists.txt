cmake_minimum_required(VERSION 3.13)
project(ssphh-all)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

if (APPLE)
    include_directories(/usr/local/include)
    set(STD_CXX_FS c++fs)
endif()

include_directories(${PROJECT_SOURCE_DIR}/../imgui)

add_subdirectory(${PROJECT_SOURCE_DIR}/../fluxions fluxions)
add_subdirectory(ssphh)
#add_subdirectory(ssphh-tests)