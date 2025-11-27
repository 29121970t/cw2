cmake_minimum_required(VERSION 3.16)
project(DrugInfoSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets WebChannel Network)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE APP_SOURCES
	${SRC_DIR}/*.cpp
	${SRC_DIR}/*.h
)

file(GLOB_RECURSE APP_RESOURCES
	${SRC_DIR}/*.qrc
)
add_executable(${PROJECT_NAME}
	${APP_SOURCES}
	${APP_RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${SRC_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::WebEngineWidgets Qt6::WebChannel Qt6::Network)

if (WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()


