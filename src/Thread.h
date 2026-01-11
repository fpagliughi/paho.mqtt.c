/*******************************************************************************
 * Copyright (c) 2009, 2023, 2026 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    https://www.eclipse.org/legal/epl-2.0/
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial implementation
 *    Ian Craggs, Allan Stockdill-Mander - async client updates
 *    Ian Craggs - fix for bug #420851
 *    Ian Craggs - change MacOS semaphore implementation
 *    Frank Pagliughi - Consolidated semaphores and conditions into "events"
 *    Frank Pagliughi - Added portable condition variables
 *******************************************************************************/

#if !defined(THREAD_H)
#define THREAD_H

#if !defined(_WIN32)
#if defined(__GNUC__) && defined(__linux__)
#if !defined(_GNU_SOURCE)
    // for pthread_setname
	#define _GNU_SOURCE
#endif
#endif
#endif

#include "MQTTExportDeclarations.h"

#include "MQTTClient.h"

#if defined(_WIN32)
	#include <windows.h>

    #define mutex_type CRITICAL_SECTION*
	#define thread_type HANDLE
	#define thread_id_type DWORD
	#define thread_return_type DWORD
	#define thread_fn LPTHREAD_START_ROUTINE
	#define evt_type HANDLE
    #define condvar_type CONDITION_VARIABLE*
	#undef ETIMEDOUT
	#define ETIMEDOUT WSAETIMEDOUT
#else
	#include <pthread.h>

    #define mutex_type pthread_mutex_t*
	#define thread_type pthread_t
	#define thread_id_type pthread_t
	#define thread_return_type void*
	typedef thread_return_type (*thread_fn)(void*);
    #define condvar_type pthread_cond_t*
	typedef struct { pthread_cond_t cond; pthread_mutex_t mutex; int val; } evt_type_struct;
	typedef evt_type_struct *evt_type;
#endif

/* Thread functions */
LIBMQTT_API void Paho_thread_start(thread_fn, void*);
int Thread_set_name(const char* thread_name);
LIBMQTT_API thread_id_type Paho_thread_getid();

/* Mutex functions */
LIBMQTT_API mutex_type Paho_thread_create_mutex(int*);
LIBMQTT_API int Paho_thread_lock_mutex(mutex_type);
LIBMQTT_API int Paho_thread_unlock_mutex(mutex_type);
int Paho_thread_destroy_mutex(mutex_type);

/* Condition variable functions */
condvar_type Thread_condvar_create(int* rc);
int Thread_condvar_destroy(condvar_type cond);
int Thread_condvar_wait(condvar_type cond, mutex_type mutex);
int Thread_condvar_timed_wait(condvar_type cond, mutex_type mutex, uint32_t timeout_ms);
int Thread_condvar_signal(condvar_type cond);
int Thread_condvar_broadcast(condvar_type cond);

/* Event Functions */
evt_type Thread_create_evt(int*);
int Thread_signal_evt(evt_type);
int Thread_wait_evt(evt_type condvar, int timeout);
int Thread_destroy_evt(evt_type);

#endif
