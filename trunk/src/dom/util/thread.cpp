/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2006 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * Thread wrapper.  This provides a platform-independent thread
 * class for IO and testing.
 *
 */

#include "thread.h"
#include <stdio.h>
#include <string.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace util
{


#ifdef __WIN32__
#include <windows.h>

static DWORD WINAPI WinThreadFunction(LPVOID context)
{
    Thread *thread = (Thread *)context;
    thread->execute();
    return 0;
}


void Thread::start()
{
    DWORD dwThreadId;
    HANDLE hThread = CreateThread(NULL, 0, WinThreadFunction,
               (LPVOID)this,  0,  &dwThreadId);
    //Make sure the thread is started before 'this' is deallocated
    while (!started)
        sleep(10);
    CloseHandle(hThread);
}

void Thread::sleep(unsigned long millis)
{
    Sleep(millis);
}



#else /* UNIX */
#include <pthread.h>

void *PthreadThreadFunction(void *context)
{
    Thread *thread = (Thread *)context;
    thread->execute();
    return NULL;
}


void Thread::start()
{
    pthread_t thread;

    int ret = pthread_create(&thread, NULL,
            PthreadThreadFunction, (void *)this);
    if (ret != 0)
        printf("Thread::start: thread creation failed: %s\n", strerror(ret));

    //Make sure the thread is started before 'this' is deallocated
    while (!started)
        sleep(10);

}

void Thread::sleep(unsigned long millis)
{
    timespec requested;
    requested.tv_sec = millis / 1000;
    requested.tv_nsec = (millis % 1000 ) * 1000000L;
    nanosleep(&requested, NULL);
}

#endif /* __WIN32__ */

}  //namespace util
}  //namespace dom
}  //namespace w3c
}  //namespace org



//#########################################################################
//# E N D    O F    F I L E
//#########################################################################


