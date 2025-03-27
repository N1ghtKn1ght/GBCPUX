cmake_minimum_required (VERSION 3.8)

set(PROJECT_NAME GBCPUX)

project (${PROJECT_NAME})

file(GLOB_RECURSE SOURCES
    source/*.cpp
)

file(GLOB_RECURSE HEADERS
    source/*.h
)

add_executable (${PROJECT_NAME} ${HEADERS} ${SOURCES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()
