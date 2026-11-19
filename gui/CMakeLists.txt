cmake_minimum_required(VERSION 3.21)
project(OrderBookQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
qt_standard_project_setup()   # enables AUTOMOC / AUTOUIC / AUTORCC

qt_add_executable(orderbook
    src/main.cpp
    src/types.h
    src/theme.h
    src/IBookBackend.h
    src/LocalBackend.h
    src/LocalBackend.cpp
    src/HttpBackend.h
    src/HttpBackend.cpp
    src/BookModels.h
    src/BookModels.cpp
    src/DepthDelegate.h
    src/MainWindow.h
    src/MainWindow.cpp
)

target_link_libraries(orderbook PRIVATE Qt6::Widgets Qt6::Network)
