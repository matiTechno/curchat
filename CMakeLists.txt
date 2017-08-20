cmake_minimum_required(VERSION 3.1)
project(curchat)

# TODO:
# * clean, remove string coupling?

# OPTIONS
#===========
set(DEFAULT_PORT 6969)
set(DEFAULT_HOST 127.0.0.1)
option(DEV_BUILD "compile with debug symbols and no optimization" OFF)
option(BUILD_SERVER "build server executable" ON)
option(BUILD_CLIENT "build client executable"  ON)
#===========

find_package(Threads REQUIRED)

if(${BUILD_CLIENT})
    find_package(PkgConfig REQUIRED)
    pkg_search_module(NCURSES REQUIRED ncurses)
endif()

if(EXISTS "/usr/include/asio.hpp")

elseif(EXISTS "/usr/boost/asio.hpp")
    include_directories("usr/include/boost")

elseif(EXISTS "usr/local/include/asio.hpp")
    include_directories("usr/local/include")

elseif(EXISTS "${CMAKE_SOURCE_DIR}/src/asio/asio/include/asio.hpp")
    execute_process(COMMAND git pull
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src/asio")
    include_directories("${CMAKE_SOURCE_DIR}/src/asio/asio/include")

else()
    execute_process(COMMAND git clone https://github.com/chriskohlhoff/asio
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src")
    include_directories("${CMAKE_SOURCE_DIR}/src/asio/asio/include")
endif()

set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wextra -pedantic")
if(DEV_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=native")
endif()

add_definitions(-DASIO_STANDALONE
    "-DDEFAULT_PORT=\"${DEFAULT_PORT}\""
    "-DDEFAULT_HOST=\"${DEFAULT_HOST}\"")
include_directories(src)

if(${BUILD_CLIENT})
    add_executable(curchat
        src/client.cpp
        src/tcp_client.cpp
        src/tcp_client.hpp
        src/curses_client.cpp
        src/curses_client.hpp
        src/msg_pool.hpp
        # obj file duplication
        src/message.cpp
        # added for qtcreator
        readme.txt
        .gitignore
        )
    target_compile_options(curchat PRIVATE ${NCURSES_CFLAGS})
    target_link_libraries(curchat ${NCURSES_LDFLAGS} Threads::Threads)
    install(TARGETS curchat DESTINATION bin)
    file(WRITE "$ENV{HOME}/.curchatrc" "$ENV{USER}")
endif()

if(${BUILD_SERVER})
    add_executable(curserver
        src/server.cpp
        src/tcp_server.cpp
        src/tcp_server.hpp
        src/message.cpp
        src/message.hpp
        )
    target_link_libraries(curserver Threads::Threads)
    install(TARGETS curserver DESTINATION bin)
endif()
