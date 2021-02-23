cmake_minimum_required (VERSION 3.0)
project (MyAdeptnessService LANGUAGES C)

# Package support

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# Versioning

file (STRINGS ${CMAKE_SOURCE_DIR}/../VERSION ADEPTNESS_TEMPLATE_VERSION)
string (REGEX REPLACE "....$" "" VERSION_MAJOR ${ADEPTNESS_TEMPLATE_VERSION})
string (REGEX REPLACE "..$" "" VERSION_MINOR ${ADEPTNESS_TEMPLATE_VERSION})
string (REGEX REPLACE "^.." "" VERSION_MINOR ${VERSION_MINOR})
string (REGEX REPLACE "^...." "" VERSION_PATCH ${ADEPTNESS_TEMPLATE_VERSION})

# Find required libraries

find_package (LibMicroHTTP REQUIRED)
if (NOT LIBMICROHTTP_FOUND)
  message (FATAL_ERROR "microhttp library or header not found")
endif ()
find_package (CURL REQUIRED)
if (NOT CURL_FOUND)
  message (FATAL_ERROR "curl library or header not found")
endif ()
find_package (LibYAML REQUIRED)
if (NOT LIBYAML_FOUND)
  message (FATAL_ERROR "yaml library or header not found")
endif ()
find_package (LibUUID REQUIRED)
if (NOT LIBUUID_FOUND)
  message (FATAL_ERROR "UUID library or header not found")
endif ()

message (STATUS "ADEPTNESS TEMPLATE ${ADEPTNESS_TEMPLATE_VERSION} for ${CMAKE_SYSTEM_NAME}")

# Configuration variables

set (ADEPTNESS_TEMPLATE_BUILD_DEBUG OFF CACHE BOOL "Build Debug")
set (ADEPTNESS_TEMPLATE_BUILD_LCOV OFF CACHE BOOL "Build LCov")

# Configure for different target systems

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set (CPACK_GENERATOR TGZ)
endif ()
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  set (CPACK_GENERATOR TGZ)
endif ()


# Set additional C compiler flags

if (DEFINED CMAKE_C_COMPILER_ID)
  if (CMAKE_C_COMPILER_ID MATCHES GNU)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wstrict-prototypes -Wmissing-prototypes")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -pthread -pipe")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -O0")
    set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -ggdb -O0")
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")
    set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -O3")
    if (ADEPTNESS_TEMPLATE_BUILD_DEBUG)
      if (ADEPTNESS_TEMPLATE_BUILD_LCOV)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
      endif ()
    endif ()
  endif ()
endif ()

find_package(OpenSSL REQUIRED)
find_library(paho-mqttpp3 NAMES libpaho-mqttpp3.a REQUIRED)
find_library(paho-mqtt3a NAMES libpaho-mqtt3a.so REQUIRED)

add_library(pahottpp STATIC IMPORTED)
set_target_properties(pahottpp PROPERTIES IMPORTED_LOCATION ${paho-mqttpp3})

add_library(pahomqtt3a SHARED IMPORTED)
set_property(TARGET pahomqtt3a PROPERTY IMPORTED_LOCATION ${paho-mqtt3a})

# Set files to compile and libraries

file (GLOB C_FILES CanMonitor/*.c)
file (GLOB C_FILES ${C_FILES} CanMonitor/common/*.c)
file (GLOB C_FILES ${C_FILES} CanMonitor/mqtt/*.c)
file (GLOB C_FILES ${C_FILES} CanMonitor/rest_server/*.c)
file (GLOB C_FILES ${C_FILES} CanMonitor/implementation/*.c)
file (GLOB C_FILES ${C_FILES} CanMonitor/implementation/tables/*.c)

# Set header files location
file (GLOB H_FILES CanMonitor)
file (GLOB H_FILES ${H_FILES_LOC} CanMonitor/common)
file (GLOB H_FILES ${H_FILES_LOC} CanMonitor/mqtt)
file (GLOB H_FILES ${H_FILES_LOC} CanMonitor/rest_server)
file (GLOB H_FILES ${H_FILES_LOC} CanMonitor/implementation)
file (GLOB H_FILES ${H_FILES_LOC} CanMonitor/implementation/tables)


# Set libraries
set (LINK_LIBRARIES ${LIBMICROHTTP_LIBRARIES} ${CURL_LIBRARIES} ${LIBYAML_LIBRARIES} ${LIBUUID_LIBRARIES})

# Build modules

add_executable (CanMonitor ${C_FILES})
target_include_directories (CanMonitor PRIVATE H_FILES_LOC)

target_link_libraries (CanMonitor PUBLIC m ${LINK_LIBRARIES})
# target_link_libraries (CanMonitor PRIVATE mosquitto)
target_link_libraries (CanMonitor PRIVATE jsoncpp)


target_link_libraries(CanMonitor PRIVATE paho-mqtt3a paho-mqtt3as paho-mqtt3c paho-mqtt3c paho-mqtt3cs)
