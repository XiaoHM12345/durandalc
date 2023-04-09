cmake_minimum_required(VERSION 3.24)
project(durandalc)
set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS ALL)
find_package(SQLite3 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_executable(durandalc main.cpp durandalc.cpp durandalc.h durandalc_options.h durandalc_options.h durandalc_option.cpp)

target_link_libraries(durandalc ${Boost_LIBRARIES})
target_link_libraries(durandalc SQLite::SQLite3)