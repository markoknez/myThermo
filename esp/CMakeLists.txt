cmake_minimum_required(VERSION 3.6)
project(termostat)

set(CONFIG_VERSION 1.0.0)

set(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/builds)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS_DEBUG "-DCLION")

include_directories(include src/include src/include/driver src/include/sll src/include/weather_icons)
file(GLOB_RECURSE SOURCE_C_FILES "src/*.c")
file(GLOB_RECURSE SOURCE_CPP_FILES "src/*.cpp")

add_executable(termostat ${SOURCE_C_FILES} ${SOURCE_CPP_FILES})

target_link_libraries(termostat lib/libdriver.a lib/libgcc.a lib/liblwip.a lib/libphy.a lib/libupgrade.a lib/libmain.a)