/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the 
 * library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * Author(s):   Michael Wybrow <mjwybrow@users.sourceforge.net>
*/


#include <cstdio>
#include <cstdlib>
#include <climits>

#include "libavoid/timer.h"
#include "libavoid/debug.h"
#include "libavoid/assertions.h"

namespace Avoid {


Timer::Timer()
{
    Reset();
}


void Timer::Reset(void)
{
    for (int i = 0; i < tmCount; i++)
    {
        //tTotal[i] = 0;
        cTotal[i] = cPath[i] = 0;
        cTally[i] = cPathTally[i] = 0;
        cMax[i] = cPathMax[i] = 0;
    }
    running = false;
    count  = 0;
    type = lasttype = tmNon;
}


void Timer::Register(const TimerIndex t, const bool start)
{
    COLA_ASSERT(t != tmNon);

    if (type == tmNon)
    {
        type = t;
    }
    else
    {
        type = tmSev;
    }

    if (start)
    {
        Start();
    }
}

void Timer::Start(void)
{
    COLA_ASSERT(!running);
    cStart[type] = clock();  // CPU time
    running = true;
}


void Timer::Stop(void)
{
    COLA_ASSERT(running);
    clock_t cStop = clock();      // CPU time
    running = false;

    bigclock_t cDiff;
    if (cStop < cStart[type])
    {
        // Uh-oh, the clock value has wrapped around.
        //
        bigclock_t realStop = ((bigclock_t) cStop) + ULONG_MAX + 1;
        cDiff = realStop - cStart[type];
    }
    else
    {
        cDiff = cStop - cStart[type];
    }
    
    COLA_ASSERT(cDiff < LONG_MAX);

    if (type == tmPth)
    {
        cPath[lasttype] += cDiff;
        cPathTally[lasttype]++;
        if (((clock_t) cDiff) > cPathMax[lasttype])
        {
            cPathMax[lasttype] = (clock_t) cDiff;
        }
    }
    else
    {
        cTotal[type] += cDiff;
        cTally[type]++;
        if (((clock_t) cDiff) > cMax[type])
        {
            cMax[type] = (clock_t) cDiff;
        }
        lasttype = type;
    }

    type = tmNon;
}


void Timer::PrintAll(FILE *fp)
{
    for (unsigned int i = 0; i < tmCount; i++)
    {
        Print((TimerIndex) i, fp);
    }
}


#define toMsec(tot) ((bigclock_t) ((tot) / (((double) CLOCKS_PER_SEC) / 1000)))
#define toAvg(tot, cnt) ((((cnt) > 0) ? ((long double) (tot)) / (cnt) : 0))

void Timer::Print(const TimerIndex t, FILE *fp)
{
   bigclock_t avg = toMsec(toAvg(cTotal[t], cTally[t]));
   bigclock_t pind = toMsec(toAvg(cPath[t], cPathTally[t]));
   bigclock_t pavg = toMsec(toAvg(cPath[t], cTally[t]));
   double max = toMsec(cMax[t]); 
   double pmax = toMsec(cPathMax[t]);
   fprintf(fp, "\t%lld %d %lld %.0f %lld %d %lld %.0f %lld\n",
           cTotal[t], cTally[t], avg, max,
           cPath[t], cPathTally[t], pavg, pmax, pind);
}


}

