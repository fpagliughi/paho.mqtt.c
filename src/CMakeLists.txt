#*******************************************************************************
#  Copyright (c) 2015, 2022 logi.cals GmbH and others
#  Copyright (c) 2023 Frank Pagliughi
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
#     Ian Craggs (IBM Corp.) - merge master
#     Ian Craggs - update for MQTTV5 support
#*******************************************************************************/

## compilation/linkage settings

configure_file(VersionInfo.h.in
  ${CMAKE_BINARY_DIR}/VersionInfo.h
  @ONLY
)

set(COMMON_SRC
  MQTTTime.c
  MQTTProtocolClient.c
  Clients.c
  utf-8.c
  MQTTPacket.c
  MQTTPacketOut.c
  Messages.c
  Tree.c
  Socket.c
  Log.c
  MQTTPersistence.c
  Thread.c
  MQTTProtocolOut.c
  MQTTPersistenceDefault.c
  SocketBuffer.c
  LinkedList.c
  MQTTProperties.c
  MQTTReasonCodes.c
  Base64.c
  SHA1.c
  WebSocket.c
  Proxy.c
)

if(NOT PAHO_HIGH_PERFORMANCE)
  set(COMMON_SRC ${COMMON_SRC}
    StackTrace.c
    Heap.c
  )
endif()

if(WIN32)
    set(LIBS_SYSTEM ws2_32 crypt32 RpcRT4 Iphlpapi)
elseif(UNIX)
    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(LIBS_SYSTEM c dl pthread rt)
        # anl is only available with glibc so check if it is found before using
        # it or build will fail on uclibc or musl
        find_library(LIB_ANL anl)
        if(LIB_ANL)
            set(LIBS_SYSTEM "${LIBS_SYSTEM}" anl)
        endif()
        add_definitions(-D_GNU_SOURCE -fvisibility=hidden)
    elseif(CMAKE_SYSTEM_NAME MATCHES "Android")
        set(LIBS_SYSTEM c dl)
    elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        SET(LIBS_SYSTEM compat pthread)
    else()
        set(LIBS_SYSTEM c pthread)
    endif()
endif()

if(PAHO_BUILD_SHARED)
    # common compilation for libpaho-mqtt3c and libpaho-mqtt3a
    add_library(common_obj OBJECT ${COMMON_SRC})

    set_target_properties(common_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)

    target_compile_definitions(common_obj PRIVATE PAHO_MQTT_EXPORTS=1)
    if(WIN32)
      target_compile_definitions(common_obj PRIVATE _CRT_SECURE_NO_DEPRECATE WIN32_LEAN_AND_MEAN)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
      target_compile_definitions(common_obj PRIVATE OSX)
    endif()

    add_executable(MQTTVersion MQTTVersion.c)
    target_compile_definitions(MQTTVersion PRIVATE PAHO_MQTT_IMPORTS=1)
endif()

if(PAHO_BUILD_STATIC)
  add_library(common_obj_static OBJECT ${COMMON_SRC})

  set_target_properties(common_obj_static PROPERTIES POSITION_INDEPENDENT_CODE ON)

  target_compile_definitions(common_obj_static PRIVATE PAHO_MQTT_STATIC=1)
  if(WIN32)
    target_compile_definitions(common_obj_static PRIVATE _CRT_SECURE_NO_DEPRECATE WIN32_LEAN_AND_MEAN)
  elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_compile_definitions(common_obj_static PRIVATE OSX)
  endif()
endif()

if(PAHO_BUILD_SHARED)
  add_library(paho-mqtt3c SHARED
    $<TARGET_OBJECTS:common_obj>
    MQTTClient.c
  )
  add_library(paho-mqtt3a SHARED
    $<TARGET_OBJECTS:common_obj>
    MQTTAsync.c
    MQTTAsyncUtils.c
  )

  foreach(TARGET paho-mqtt3c paho-mqtt3a)
    target_link_libraries(${TARGET} PRIVATE ${LIBS_SYSTEM})

    target_include_directories(${TARGET}
      PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      PRIVATE
        ${CMAKE_BINARY_DIR}
    )
    set_target_properties(${TARGET} PROPERTIES
      VERSION ${CLIENT_VERSION}
      SOVERSION ${PAHO_VERSION_MAJOR}
    )
    target_compile_definitions(${TARGET} PRIVATE PAHO_MQTT_EXPORTS)
    if(WIN32)
      target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_DEPRECATE WIN32_LEAN_AND_MEAN)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
      target_compile_definitions(${TARGET} PRIVATE OSX)
    endif()
  endforeach()

  target_link_libraries(MQTTVersion paho-mqtt3a paho-mqtt3c)

  if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(MQTTCLIENT_ENTRY_POINT _MQTTClient_init)
    set(MQTTASYNC_ENTRY_POINT _MQTTAsync_init)
  elseif(NOT WIN32)
    set(MQTTCLIENT_ENTRY_POINT MQTTClient_init)
		set(MQTTASYNC_ENTRY_POINT MQTTAsync_init)
	endif()

  if(NOT WIN32)
    set_target_properties(paho-mqtt3c
      PROPERTIES
        LINK_FLAGS "-Wl,-init,${MQTTCLIENT_ENTRY_POINT}"
    )
    set_target_properties(paho-mqtt3a
      PROPERTIES
        LINK_FLAGS "-Wl,-init,${MQTTASYNC_ENTRY_POINT}"
    )
  endif()

  install(TARGETS paho-mqtt3c paho-mqtt3a
    EXPORT eclipse-paho-mqtt-cTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  install(TARGETS MQTTVersion
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
endif()

if(PAHO_BUILD_STATIC)
  add_library(paho-mqtt3c-static STATIC
    $<TARGET_OBJECTS:common_obj_static>
    MQTTClient.c
  )
  add_library(paho-mqtt3a-static STATIC
    $<TARGET_OBJECTS:common_obj_static>
    MQTTAsync.c
    MQTTAsyncUtils.c
  )

  foreach(TARGET paho-mqtt3c-static paho-mqtt3a-static)
    target_link_libraries(${TARGET} PRIVATE ${LIBS_SYSTEM})

    target_include_directories(${TARGET}
      PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      PRIVATE
        ${CMAKE_BINARY_DIR}
    )
    set_target_properties(${TARGET}
      PROPERTIES
        VERSION ${CLIENT_VERSION}
        SOVERSION ${PAHO_VERSION_MAJOR}
    )
    target_compile_definitions(${TARGET} PRIVATE PAHO_MQTT_STATIC)
    if(WIN32)
      target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_DEPRECATE WIN32_LEAN_AND_MEAN)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
      target_compile_definitions(${TARGET} PRIVATE OSX)
    endif()
  endforeach()

  if(NOT WIN32)
    set_target_properties(paho-mqtt3c-static PROPERTIES OUTPUT_NAME paho-mqtt3c)
    set_target_properties(paho-mqtt3a-static PROPERTIES OUTPUT_NAME paho-mqtt3a)
  endif()

  if(NOT PAHO_BUILD_SHARED)
    install(TARGETS paho-mqtt3c-static paho-mqtt3a-static
      EXPORT eclipse-paho-mqtt-cTargets
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    else()
      install(TARGETS paho-mqtt3c-static paho-mqtt3a-static
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      )
    endif()
endif()

install(
  FILES 
    MQTTAsync.h
    MQTTClient.h
    MQTTClientPersistence.h
    MQTTProperties.h
    MQTTReasonCodes.h
    MQTTSubscribeOpts.h
    MQTTExportDeclarations.h
  DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}
)

if(PAHO_WITH_SSL)
  set(OPENSSL_ROOT_DIR "" CACHE PATH "Directory containing OpenSSL libraries and includes")
  find_package(OpenSSL REQUIRED)

  if(PAHO_BUILD_SHARED)
    ## common compilation for libpaho-mqtt3cs and libpaho-mqtt3as
    ## Note: SSL libraries must be recompiled due ifdefs
    add_library(common_ssl_obj OBJECT ${COMMON_SRC})
    target_include_directories(common_ssl_obj PUBLIC ${OPENSSL_INCLUDE_DIR})

    set_property(TARGET common_ssl_obj PROPERTY	POSITION_INDEPENDENT_CODE ON)
    target_compile_definitions(common_ssl_obj PRIVATE OPENSSL PAHO_MQTT_EXPORTS)
    if(WIN32)
      target_compile_definitions(common_ssl_obj PRIVATE _CRT_SECURE_NO_DEPRECATE WIN32_LEAN_AND_MEAN)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
      target_compile_definitions(common_ssl_obj PRIVATE OSX)
    endif()
    
    add_library(paho-mqtt3cs SHARED
      $<TARGET_OBJECTS:common_ssl_obj>
      MQTTClient.c
      SSLSocket.c
    )
    add_library(paho-mqtt3as SHARED
      $<TARGET_OBJECTS:common_ssl_obj>
      MQTTAsync.c
      MQTTAsyncUtils.c
      SSLSocket.c
    )
    
    foreach(TARGET paho-mqtt3cs paho-mqtt3as)
      target_include_directories(${TARGET}
        PUBLIC
          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PRIVATE
          ${CMAKE_BINARY_DIR}
      )
      target_link_libraries(${TARGET}
        PUBLIC
        OpenSSL::SSL OpenSSL::Crypto ${LIBS_SYSTEM}
      )
      set_target_properties(${TARGET}
        PROPERTIES
          VERSION ${CLIENT_VERSION}
          SOVERSION ${PAHO_VERSION_MAJOR}
      )

      target_compile_definitions(${TARGET} PRIVATE OPENSSL PAHO_MQTT_EXPORTS)
      if(WIN32)
        target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_DEPRECATE WIN32_LEAN_AND_MEAN)
      elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        target_compile_definitions(${TARGET} PRIVATE OSX)
      endif()
    endforeach()

    if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
      set(MQTTCLIENT_ENTRY_POINT _MQTTClient_init)
      set(MQTTASYNC_ENTRY_POINT _MQTTAsync_init)
    elseif(NOT WIN32)
      set(MQTTCLIENT_ENTRY_POINT MQTTClient_init)
      set(MQTTASYNC_ENTRY_POINT MQTTAsync_init)
    endif()

    if(NOT WIN32)
      set_target_properties(
        paho-mqtt3cs PROPERTIES
        LINK_FLAGS "-Wl,-init,${MQTTCLIENT_ENTRY_POINT}")
      set_target_properties(
        paho-mqtt3as PROPERTIES
        LINK_FLAGS "-Wl,-init,${MQTTASYNC_ENTRY_POINT}")
    endif()

    install(TARGETS paho-mqtt3cs paho-mqtt3as
      EXPORT eclipse-paho-mqtt-cTargets
      ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION  ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR}
    )
  endif()

  if(PAHO_BUILD_STATIC)
    ## common compilation for libpaho-mqtt3cs and libpaho-mqtt3as
    ## Note: SSL libraries must be recompiled due ifdefs
    add_library(common_ssl_obj_static OBJECT ${COMMON_SRC})
    target_include_directories(common_ssl_obj_static PUBLIC ${OPENSSL_INCLUDE_DIR})

    set_target_properties(common_ssl_obj_static PROPERTIES POSITION_INDEPENDENT_CODE ON)

    target_compile_definitions(common_ssl_obj_static PRIVATE OPENSSL PAHO_MQTT_STATIC)
    if(WIN32)
      target_compile_definitions(common_ssl_obj_static PRIVATE _CRT_SECURE_NO_DEPRECATE WIN32_LEAN_AND_MEAN)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
      target_compile_definitions(common_ssl_obj_static PRIVATE OSX)
    endif()

    add_library(paho-mqtt3cs-static STATIC
      $<TARGET_OBJECTS:common_ssl_obj_static>
      MQTTClient.c
      SSLSocket.c
    )
    add_library(paho-mqtt3as-static STATIC
      $<TARGET_OBJECTS:common_ssl_obj_static>
      MQTTAsync.c
      MQTTAsyncUtils.c
      SSLSocket.c
    )

    foreach(TARGET paho-mqtt3cs-static paho-mqtt3as-static)
      target_include_directories(${TARGET}
        PUBLIC
          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PRIVATE
          ${CMAKE_BINARY_DIR}
      )
      target_link_libraries(${TARGET}
        PUBLIC
          OpenSSL::SSL OpenSSL::Crypto ${LIBS_SYSTEM}
      )

      set_target_properties(${TARGET} PROPERTIES
        VERSION ${CLIENT_VERSION}
        SOVERSION ${PAHO_VERSION_MAJOR}
      )

      target_compile_definitions(${TARGET} PRIVATE OPENSSL=1 PAHO_MQTT_STATIC=1)
      if(WIN32)
        target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_DEPRECATE WIN32_LEAN_AND_MEAN)
      elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        target_compile_definitions(${TARGET} PRIVATE OSX)
      endif()
    endforeach()

    if(NOT WIN32)
      set_target_properties(paho-mqtt3cs-static PROPERTIES OUTPUT_NAME paho-mqtt3cs)
      set_target_properties(paho-mqtt3as-static PROPERTIES OUTPUT_NAME paho-mqtt3as)
    endif()

    if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
      set(MQTTCLIENT_ENTRY_POINT _MQTTClient_init)
      set(MQTTASYNC_ENTRY_POINT _MQTTAsync_init)
    elseif(NOT WIN32)
      set(MQTTCLIENT_ENTRY_POINT MQTTClient_init)
      set(MQTTASYNC_ENTRY_POINT MQTTAsync_init)
    endif()

    if(NOT WIN32)
      set_target_properties(
        paho-mqtt3cs-static PROPERTIES
        LINK_FLAGS "-Wl,-init,${MQTTCLIENT_ENTRY_POINT}"
      )
      set_target_properties(
        paho-mqtt3as-static PROPERTIES
        LINK_FLAGS "-Wl,-init,${MQTTASYNC_ENTRY_POINT}"
      )
    endif()

    if(NOT PAHO_BUILD_SHARED)
      install(TARGETS paho-mqtt3cs-static paho-mqtt3as-static
        EXPORT eclipse-paho-mqtt-cTargets
        ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION  ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR}
      )
      else()
        install(TARGETS paho-mqtt3cs-static paho-mqtt3as-static
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
      endif()
    endif()
endif()

install(EXPORT eclipse-paho-mqtt-cTargets
  FILE eclipse-paho-mqtt-cConfig.cmake
  NAMESPACE eclipse-paho-mqtt-c::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/eclipse-paho-mqtt-c
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file("eclipse-paho-mqtt-cConfigVersion.cmake"
  VERSION ${CLIENT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/eclipse-paho-mqtt-cConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/eclipse-paho-mqtt-c
)

# Base64 test
add_executable(Base64Test EXCLUDE_FROM_ALL Base64.c Base64.h)
target_compile_definitions( Base64Test PUBLIC "-DBASE64_TEST")

if(PAHO_WITH_SSL)
  add_executable(Base64TestOpenSSL EXCLUDE_FROM_ALL Base64.c Base64.h)

  target_link_libraries( Base64TestOpenSSL OpenSSL::SSL OpenSSL::Crypto)
	target_compile_definitions( Base64TestOpenSSL PUBLIC "-DBASE64_TEST -DOPENSSL=1" )
endif()

# SHA1 test
add_executable(Sha1Test EXCLUDE_FROM_ALL SHA1.c SHA1.h)
target_compile_definitions( Sha1Test PUBLIC "-DSHA1_TEST" )

if(PAHO_WITH_SSL)
  add_executable(Sha1TestOpenSSL EXCLUDE_FROM_ALL SHA1.c SHA1.h)

  target_link_libraries(Sha1TestOpenSSL OpenSSL::SSL OpenSSL::Crypto)
  target_compile_definitions(Sha1TestOpenSSL PUBLIC "-DSHA1_TEST -DOPENSSL=1")
endif()
