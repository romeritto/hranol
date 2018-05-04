# CMakeList.txt : CMake project for hranol, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.1)

# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
find_package(OpenCV 3 REQUIRED HINTS $ENV{OpenCV_DIR}/lib)

if(CMAKE_VERSION VERSION_LESS "2.8.11")
  # Add OpenCV headers location to your paths
  include_directories(${OpenCV_INCLUDE_DIRS})
endif()

# Require C++17 the proper way
# However, for C++17 it only works from CMake 3.8 and this version is not
# wildely supported yet. Therefore explicit flags for compilers are used  for now.
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options("/W4" "$<$<CONFIG:RELEASE>:/O2>" "/std:c++17")
else()
    add_compile_options("-Wall" "-Wextra" "-Werror" "$<$<CONFIG:RELEASE>:-O3>" "-std=c++1z")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # libstdc++fs.a has to be linked explicitly for <filesystem> 
        LIST(APPEND Std_LIBS "libstdc++fs.a")
    endif()
endif()

# Add source to this project's executable.
add_executable (hranol "hranol.cpp" "FolderCrawler.cpp" "ImageStore.cpp" "ImageProcessor.cpp")


# Link with libraries
target_link_libraries(hranol ${OpenCV_LIBS})
target_link_libraries(hranol ${Std_LIBS})

