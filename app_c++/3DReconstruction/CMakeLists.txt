cmake_minimum_required (VERSION 3.8)

project ("3DReconstruction")

include_directories($ENV{Eigen3_DIR})

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 3DReconstruction/*.cpp 3DReconstruction/*.h)


add_executable(${PROJECT_NAME} ${SOURCES})
