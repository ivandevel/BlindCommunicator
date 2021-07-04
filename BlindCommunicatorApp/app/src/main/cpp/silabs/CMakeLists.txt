# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set ( LKFLIB_SOURCES lkf-lib.cpp)
add_library( lkf-lib SHARED ${LKFLIB_SOURCES} )

find_library( log-lib log )

target_link_libraries( lkf-lib ${log-lib} )