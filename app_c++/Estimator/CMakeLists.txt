cmake_minimum_required (VERSION 3.8)
set(PROJECT_NAME "Estimator")

project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS source/*.cpp source/*.h source/*.ui source/*.qrc)

find_package(OpenCV REQUIRED)
find_package(Boost 1.79 REQUIRED)


add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS} ) 

include_directories(${OpenCV_DIR}/include)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})