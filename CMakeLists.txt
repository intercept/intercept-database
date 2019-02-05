cmake_minimum_required (VERSION 3.6)

#----Make changes here

#This is your project name. And also the filename of the resulting plugin.
project (intercept-database)

#This setting enables the use of the engine string type instead of converting to std::string.
#Enabling this results in better performance when handling strings to SQF commands.
option(USE_ENGINE_TYPES "USE_ENGINE_TYPES" ON)

#----Don't change anything below this line

option(USE_64BIT_BUILD "USE_64BIT_BUILD" OFF)
set(INTERCEPT_LINK_TYPE "static")

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
	set( USE_64BIT_BUILD ON)
endif()

message("GENERATOR USED: '${CMAKE_GENERATOR}'")
message("COMPILER USED: '${CMAKE_CXX_COMPILER_ID}'")

set(CMAKE_CL_64 ${USE_64BIT_BUILD})

if(USE_64BIT_BUILD)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/win64/")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/win32/")
endif()


#yaml resets c++17 so I just move it above ours so we don't have to re-set
SET(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Don't want" FORCE)
SET(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Don't want" FORCE)
SET(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Don't want" FORCE)
SET(YAML_CPP_INSTALL OFF CACHE BOOL "Don't want" FORCE)
SET(MSVC_SHARED_RT OFF CACHE BOOL "want static" FORCE)

add_subdirectory(yaml-cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON) 
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)


SET(BUILD_STATIC ON CACHE BOOL "force static lib" FORCE)
SET(STATIC_MSVCRT ON CACHE BOOL "force static lib" FORCE)

SET(WITH_UNIT_TESTS OFF CACHE BOOL "no mysql tests" FORCE)
set(WITH_SSL "OFF" CACHE STRING "Disabled for now" FORCE)
set(WITH_CURL OFF CACHE BOOL "not cuurl" FORCE)
#TODO test with wolfssl

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-std=c++1z -O2 -s -fPIC -fpermissive -static-libgcc -static-libstdc++")#-march=i686 -m32
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
	set(CMAKE_SHARED_LINKER_FLAGS "-shared -static-libgcc -static-libstdc++")
else()
	set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1 /MP /EHsc")
	set(CMAKE_CXX_FLAGS_RELEASE "/MT /Zi /O2 /Ob1 /EHsc /MP") #with debug info
	set(CMAKE_C_FLAGS_DEBUG "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1 /MP /EHsc")
	set(CMAKE_C_FLAGS_RELEASE "/MT /Zi /O2 /Ob1 /EHsc /MP") #with debug info
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/OPT:REF /DEBUG:FULL") 
endif()


add_subdirectory(mariadb-connector-c)

CONFIGURE_FILE("${CMAKE_CURRENT_BINARY_DIR}/mariadb-connector-c/include/mariadb_version.h"
               "${PROJECT_SOURCE_DIR}/mariadb-connector-c/include/mariadb_version.h")



add_subdirectory(mariadbpp)

include_directories(AFTER "${PROJECT_SOURCE_DIR}/mariadbpp/include")
include_directories(AFTER "${PROJECT_SOURCE_DIR}/mariadb-connector-c/include")
include_directories(AFTER "${PROJECT_SOURCE_DIR}/ittnotify")






add_subdirectory(src)

