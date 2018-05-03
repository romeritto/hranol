# CMakeList.txt : CMake project for hranol, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
find_package(OpenCV 3 REQUIRED HINTS $ENV{OpenCV_DIR}/lib)

if(CMAKE_VERSION VERSION_LESS "2.8.11")
  # Add OpenCV headers location to your include paths
  include_directories(${OpenCV_INCLUDE_DIRS})
endif()

if(MSVC)
    add_compile_options("/W4" "$<$<CONFIG:RELEASE>:/O2>")
else()
    add_compile_options("-Wall" "-Wextra" "-Werror" "$<$<CONFIG:RELEASE>:-O3>")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options("-stdlib=libc++")
    else()
        # nothing special for gcc at the moment
    endif()
endif()

# Add source to this project's executable.
add_executable (hranol "hranol.cpp" "FolderCrawler.cpp" "ImageStore.cpp" "ImageProcessor.cpp")
#target_include_directories(hranol PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Link your application with OpenCV libraries
target_link_libraries(hranol ${OpenCV_LIBS})

# TODO: Add tests and install targets if needed.
 