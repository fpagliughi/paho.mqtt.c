#*******************************************************************************
#  Copyright (c) 2015, 2020 logi.cals GmbH and others
#
#  All rights reserved. This program and the accompanying materials
#  are made available under the terms of the Eclipse Public License v2.0
#  and Eclipse Distribution License v1.0 which accompany this distribution.
#
#  The Eclipse Public License is available at
#     https://www.eclipse.org/legal/epl-2.0/
#  and the Eclipse Distribution License is available at
#    http://www.eclipse.org/org/documents/edl-v10.php.
#
#  Contributors:
#     Rainer Poisel - initial version
#     Genis Riera Perez - Add support for building debian package
#     Frank Pagliughi - some cleanup and modernization
#*******************************************************************************/

# Note: on OS X you should install XCode and the associated command-line tools

cmake_minimum_required(VERSION 3.5)
project(EclipsePahoC C)

message(STATUS "CMake version: " ${CMAKE_VERSION})
message(STATUS "CMake system name: " ${CMAKE_SYSTEM_NAME})

set(CMAKE_SCRIPTS "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

## build settings
file(READ version.major PAHO_VERSION_MAJOR)
file(READ version.minor PAHO_VERSION_MINOR)
file(READ version.patch PAHO_VERSION_PATCH)

set(CLIENT_VERSION ${PAHO_VERSION_MAJOR}.${PAHO_VERSION_MINOR}.${PAHO_VERSION_PATCH})

include(GNUInstallDirs)

string(TIMESTAMP BUILD_TIMESTAMP UTC)
message(STATUS "Timestamp is ${BUILD_TIMESTAMP}")

if(WIN32)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -DWIN32_LEAN_AND_MEAN)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  ADD_DEFINITIONS(-DOSX)
endif()

## build options
option(PAHO_WITH_SSL "Flag that defines whether to build ssl-enabled binaries too." OFF)
option(PAHO_BUILD_SHARED "Build shared library" ON)
option(PAHO_BUILD_STATIC "Build static library" OFF)
option(PAHO_BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)
option(PAHO_BUILD_SAMPLES "Build sample programs" OFF)
option(PAHO_BUILD_DEB_PACKAGE "Build debian package" OFF)
option(PAHO_ENABLE_TESTING "Build tests and run" ON)
option(PAHO_ENABLE_CPACK "Enable CPack" ON)
option(PAHO_HIGH_PERFORMANCE "Disable tracing and heap tracking" OFF)
option(PAHO_USE_SELECT "Revert to select system call instead of poll" OFF)

if(PAHO_HIGH_PERFORMANCE)
    add_definitions(-DHIGH_PERFORMANCE=1)
endif()

if(PAHO_USE_SELECT)
    add_definitions(-DUSE_SELECT=1)
endif()

if(NOT PAHO_BUILD_SHARED AND NOT PAHO_BUILD_STATIC)
    message(FATAL_ERROR "You must set either PAHO_BUILD_SHARED, PAHO_BUILD_STATIC, or both")
endif()

if(PAHO_BUILD_DEB_PACKAGE)
    set(CMAKE_INSTALL_DOCDIR share/doc/libpaho-mqtt)
    set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
    set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS_POLICY ">=")
endif()

add_subdirectory(src)

if(PAHO_BUILD_SAMPLES)
    add_subdirectory(src/samples)
endif()

if(PAHO_BUILD_DOCUMENTATION)
    add_subdirectory(doc)
endif()

if(PAHO_ENABLE_CPACK)
    ### packaging settings
    file(GLOB samples "src/samples/*.c")
    install(FILES ${samples} DESTINATION ${CMAKE_INSTALL_DOCDIR}/samples)

    set(CPACK_PACKAGE_VENDOR "Eclipse Paho")
    set(CPACK_PACKAGE_NAME "Eclipse-Paho-MQTT-C")
    install(FILES CONTRIBUTING.md epl-v20 edl-v10 README.md notice.html DESTINATION ${CMAKE_INSTALL_DOCDIR})

    if(WIN32)
        set(CPACK_GENERATOR "ZIP")
    elseif(PAHO_BUILD_DEB_PACKAGE)
        install(FILES CONTRIBUTING.md epl-v20 edl-v10 README.md notice.html DESTINATION ${CMAKE_INSTALL_DOCDIR})

        set(CPACK_GENERATOR "DEB")
        configure_file(${CMAKE_SCRIPTS}/CPackDebConfig.cmake.in
            ${CMAKE_BINARY_DIR}/CPackDebConfig.cmake 
            @ONLY
        )
        set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_BINARY_DIR}/CPackDebConfig.cmake)
    else()
        set(CPACK_GENERATOR "TGZ")
    endif()
else()
    file(GLOB samples "src/samples/*.c")
    install(FILES ${samples} DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()

set(CPACK_PACKAGE_VERSION_MAJOR ${PAHO_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PAHO_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PAHO_VERSION_PATCH})
include(CPack)

if(PAHO_ENABLE_TESTING)
    enable_testing()
    include_directories(test src)
    add_subdirectory(test)
else()
    include_directories(src)
endif()
