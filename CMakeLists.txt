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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON) 
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)


SET(BUILD_STATIC ON CACHE BOOL "force static lib" FORCE)
SET(STATIC_MSVCRT ON CACHE BOOL "force static lib" FORCE)

SET(HAVE_SHARED_PTR ON CACHE BOOL "shut up CDK" FORCE)
SET(HAVE_SYSTEM_ERROR ON CACHE BOOL "shut up CDK" FORCE)
SET(HAVE_CODECVT_UTF8 ON CACHE BOOL "shut up CDK" FORCE)

set(WITH_SSL "${CMAKE_SOURCE_DIR}/wolfssl" CACHE STRING
   "Use the same setting as used when building connector" FORCE
)

add_subdirectory(mysql-connector-cpp)

include_directories(AFTER "${PROJECT_SOURCE_DIR}/mysql-connector-cpp/include")
add_subdirectory(src)

