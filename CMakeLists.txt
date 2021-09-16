cmake_minimum_required(VERSION 3.0)
project(sport_strings CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(sport_strings
	src/main.cpp
)

target_link_libraries(sport_strings
	icuuc icuio
)

set_property(TARGET sport_strings PROPERTY CXX_STANDARD 17)
