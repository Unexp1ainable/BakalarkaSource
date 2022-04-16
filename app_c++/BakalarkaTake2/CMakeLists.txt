cmake_minimum_required (VERSION 3.8)
set(PROJECT_NAME "main")

project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(Boost_DEBUG 1)


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp *.h *.ui *.qrc)

find_package(OpenCV REQUIRED)
find_package(Qt6 COMPONENTS Widgets SvgWidgets REQUIRED)
find_package(Boost 1.79 REQUIRED)


add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::SvgWidgets ${OpenCV_LIBS} ) 

include_directories(${Qt_DIR}/include)
include_directories(${OpenCV_DIR}/include)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})