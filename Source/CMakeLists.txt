
cmake_minimum_required(VERSION 2.8)
 
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()

PROJECT(Stereoscopic)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets REQUIRED QUIET)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB UI_FILES *.ui)
file(GLOB QT_WRAP *.h)
file(GLOB CXX_FILES *.cxx)

qt5_wrap_ui(UISrcs ${UI_FILES} )
add_executable(Stereoscopic ${CXX_FILES} ${UISrcs} ${QT_WRAP})
qt5_use_modules(Stereoscopic Core Gui)
target_link_libraries(Stereoscopic ${VTK_LIBRARIES})
