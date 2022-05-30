#*******************************************************************************
#  Copyright (c) 2015, 2020, 2022 logi.cals GmbH and others
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
#     Frank Pagliughi - Consolidated operations into loops over lists
#*******************************************************************************/

# Note: on OS X you should install XCode and the associated command-line tools

# These compile in 'pubsub_opts.c', and are built for shared and static
set(ASYNC_PUBSUB paho_c_pub paho_c_sub)
set(SYNC_PUBSUB paho_cs_pub paho_cs_sub)

if(PAHO_BUILD_SHARED)
    foreach(EXE ${ASYNC_PUBSUB} ${SYNC_PUBSUB})
        add_executable(${EXE} ${EXE}.c pubsub_opts.c)
        target_compile_definitions(${EXE} PRIVATE
            PAHO_MQTT_IMPORTS=1
            $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_DEPRECATE>
        )
    endforeach()

    set(SHARED_ASYNC 
        MQTTAsync_subscribe 
        MQTTAsync_publish 
        MQTTAsync_publish_time
    )
    set(SHARED_SYNC 
        MQTTClient_subscribe
        MQTTClient_publish
        MQTTClient_publish_async
    )

    foreach(EXE ${SHARED_ASYNC} ${SHARED_SYNC})
        add_executable(${EXE} ${EXE}.c)
        target_compile_definitions(${EXE} PRIVATE
            PAHO_MQTT_IMPORTS=1
            $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_DEPRECATE>
        )
    endforeach()

    list(APPEND SHARED_ASYNC ${ASYNC_PUBSUB})
    list(APPEND SHARED_SYNC ${SYNC_PUBSUB})

    foreach(EXE ${SHARED_ASYNC})
        target_link_libraries(${EXE} paho-mqtt3as)
    endforeach()

    foreach(EXE ${SHARED_SYNC})
        target_link_libraries(${EXE} paho-mqtt3cs)
    endforeach()
endif()

if(PAHO_BUILD_STATIC)
    foreach(EXE ${ASYNC_PUBSUB} ${SYNC_PUBSUB})
        add_executable(${EXE}_static ${EXE}.c pubsub_opts.c)
        target_compile_definitions(${EXE} PRIVATE
            $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_DEPRECATE>
        )
    endforeach()

    list(APPEND STATIC_ASYNC ${ASYNC_PUBSUB})
    list(TRANSFORM STATIC_ASYNC APPEND _static)

    list(APPEND STATIC_SYNC ${SYNC_PUBSUB})
    list(TRANSFORM STATIC_SYNC APPEND _static)

    foreach(EXE ${STATIC_ASYNC})
        target_link_libraries(${EXE} paho-mqtt3as-static)
    endforeach()

    foreach(EXE ${STATIC_SYNC})
        target_link_libraries(${EXE} paho-mqtt3cs-static)
    endforeach()
endif()

install(TARGETS ${SHARED_ASYNC} ${SHARED_SYNC} ${STATIC_ASYNC} ${STATIC_SYNC}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

