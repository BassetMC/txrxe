cmake_minimum_required(VERSION 3.16)

project(txrxe2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(consumer
    consumer/main.cpp
    core/txrx.cpp
    core/txrx.h
)

add_executable(producer
    producer/main.cpp
    core/txrx.cpp
    core/txrx.h
)

include(GNUInstallDirs)
install(TARGETS consumer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS producer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)