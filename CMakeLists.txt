cmake_minimum_required(VERSION 3.5)

project(MonteCarloSimulator VERSION 1.0 LANGUAGES CXX)

# Use C++11 standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find required Qt packages, including PrintSupport
find_package(Qt5 COMPONENTS Widgets Network PrintSupport REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Add the executable
add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.cpp
    montecarlo.cpp
    qcustomplot.cpp
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Qt5::Network
    Qt5::PrintSupport
)
