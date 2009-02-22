/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2004-2006  Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
*/

#include <cstdio>
#include <cstdlib>
#include <cassert>
using std::abort;
#include <climits>

#include "libavoid/timer.h"

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


void Timer::Register(const int t, const bool start)
{
    assert(t != tmNon);

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
    if (running)
    {
        fprintf(stderr, "ERROR: Timer already running in Timer::Start()\n");
        abort();
    }
    cStart[type] = clock();  // CPU time
    running = true;
}


void Timer::Stop(void)
{
    if (!running)
    {
        fprintf(stderr, "ERROR: Timer not running in Timer::Stop()\n");
        abort();
    }
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
    
    if (cDiff > LONG_MAX)
    {
        fprintf(stderr, "Error: cDiff overflow in Timer:Stop()\n");
        abort();
    }

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


void Timer::PrintAll(void)
{
    for (int i = 0; i < tmCount; i++)
    {
        Print(i);
    }
}


#define toMsec(tot) ((bigclock_t) ((tot) / (((double) CLOCKS_PER_SEC) / 1000)))
#define toAvg(tot, cnt) ((((cnt) > 0) ? ((long double) (tot)) / (cnt) : 0))

void Timer::Print(const int t)
{
   bigclock_t avg = toMsec(toAvg(cTotal[t], cTally[t]));
   bigclock_t pind = toMsec(toAvg(cPath[t], cPathTally[t]));
   bigclock_t pavg = toMsec(toAvg(cPath[t], cTally[t]));
   double max = toMsec(cMax[t]); 
   double pmax = toMsec(cPathMax[t]);
   printf("\t%lld %d %lld %.0f %lld %d %lld %.0f %lld\n",
           cTotal[t], cTally[t], avg, max,
           cPath[t], cPathTally[t], pavg, pmax, pind);
}


}

