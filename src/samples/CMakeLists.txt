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
#     Ian Craggs - update sample names
#*******************************************************************************/

# Note: on OS X you should install XCode and the associated command-line tools

## compilation/linkage settings
include_directories(
    .
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}
)

if(WIN32)
    add_definitions(/DCMAKE_BUILD /D_CRT_SECURE_NO_DEPRECATE)
endif()

# sample files c

if(PAHO_BUILD_SHARED)
    add_executable(paho_c_pub paho_c_pub.c pubsub_opts.c)
    add_executable(paho_c_sub paho_c_sub.c pubsub_opts.c)
    add_executable(paho_cs_pub paho_cs_pub.c pubsub_opts.c)
    add_executable(paho_cs_sub paho_cs_sub.c pubsub_opts.c)

    set_target_properties(paho_c_pub paho_c_sub paho_cs_pub paho_cs_sub
        PROPERTIES COMPILE_DEFINITIONS "PAHO_MQTT_IMPORTS=1"
    )

    target_link_libraries(paho_c_pub paho-mqtt3as)
    target_link_libraries(paho_c_sub paho-mqtt3as)
    target_link_libraries(paho_cs_pub paho-mqtt3cs)
    target_link_libraries(paho_cs_sub paho-mqtt3cs)

    add_executable(MQTTAsync_subscribe MQTTAsync_subscribe.c)
    add_executable(MQTTAsync_publish MQTTAsync_publish.c)
    add_executable(MQTTAsync_publish_time MQTTAsync_publish_time.c)
    add_executable(MQTTClient_subscribe MQTTClient_subscribe.c)
    add_executable(MQTTClient_publish MQTTClient_publish.c)
    add_executable(MQTTClient_publish_async MQTTClient_publish_async.c)

    set_target_properties(
        MQTTAsync_subscribe
        MQTTAsync_publish
        MQTTAsync_publish_time
        MQTTClient_subscribe
        MQTTClient_publish
        MQTTClient_publish_async
        PROPERTIES COMPILE_DEFINITIONS "PAHO_MQTT_IMPORTS=1"
    )

    target_link_libraries(MQTTAsync_subscribe paho-mqtt3a)
    target_link_libraries(MQTTAsync_publish paho-mqtt3a)
    target_link_libraries(MQTTAsync_publish_time paho-mqtt3a)
    target_link_libraries(MQTTClient_subscribe paho-mqtt3c)
    target_link_libraries(MQTTClient_publish paho-mqtt3c)
    target_link_libraries(MQTTClient_publish_async paho-mqtt3c)

    install(
        TARGETS
            paho_c_sub
            paho_c_pub
            paho_cs_sub
            paho_cs_pub
            MQTTAsync_subscribe
            MQTTAsync_publish
            MQTTAsync_publish_time
            MQTTClient_subscribe
            MQTTClient_publish
            MQTTClient_publish_async
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

if(PAHO_BUILD_STATIC)
    add_executable(paho_c_pub_static paho_c_pub.c pubsub_opts.c)
    add_executable(paho_c_sub_static paho_c_sub.c pubsub_opts.c)
    add_executable(paho_cs_pub_static paho_cs_pub.c pubsub_opts.c)
    add_executable(paho_cs_sub_static paho_cs_sub.c pubsub_opts.c)

    target_link_libraries(paho_c_pub_static paho-mqtt3as-static)
    target_link_libraries(paho_c_sub_static paho-mqtt3as-static)
    target_link_libraries(paho_cs_pub_static paho-mqtt3cs-static)
    target_link_libraries(paho_cs_sub_static paho-mqtt3cs-static)

    install(
        TARGETS
            paho_c_sub_static
            paho_c_pub_static
            paho_cs_sub_static
            paho_cs_pub_static
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

