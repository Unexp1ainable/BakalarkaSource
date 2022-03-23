cmake_minimum_required (VERSION 3.8)
set(PROJECT_NAME "main")

project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/**/*.h)
file(GLOB UIS ${CMAKE_CURRENT_SOURCE_DIR}/**/*.ui)
file(GLOB RCS ${CMAKE_CURRENT_SOURCE_DIR}/**/*.rc)


set(CMAKE_PREFIX_PATH "${Qt_DIR}/lib/cmake")

find_package( OpenCV REQUIRED )
find_package(QT NAMES Qt6 COMPONENTS Widgets Core Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)


add_executable(${PROJECT_NAME} main.cpp ${SOURCES} ${HEADERS} ${UIS} ${RCS})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui ${OpenCV_LIBS}) 

include_directories(${Qt_DIR}/include)
include_directories(${OpenCV_DIR}/include)
