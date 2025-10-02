cmake_minimum_required(VERSION 3.16)
project(HelloWorld)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})

add_executable(hello_world main.cc helloworld.cc)

target_link_libraries(hello_world ${GTKMM_LIBRARIES})
target_compile_options(hello_world PUBLIC ${GTKMM_CFLAGS_OTHER})
