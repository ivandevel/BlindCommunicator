cmake_minimum_required(VERSION 3.4.1)

set ( I2CLIB_SOURCES I2C.c I2C.h)
add_library( I2C SHARED ${I2CLIB_SOURCES} )

find_library( log-lib log )

target_link_libraries( I2C ${log-lib} )