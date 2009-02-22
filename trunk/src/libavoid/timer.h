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

#ifndef PROFILE_H
#define PROFILE_H

#include <ctime>

namespace Avoid {


#ifdef NOTIMERS

  #define register_timer(t) do {} while(0)
  #define regstart_timer(t) do {} while(0)
  #define start_timer() do {} while(0)
  #define stop_timer() do {} while(0)

#else
   
  #define register_timer(t) router->timers.Register(t)
  #define regstart_timer(t) router->timers.Register(t, timerStart)
  #define start_timer() router->timers.Start()
  #define stop_timer() router->timers.Stop()

#endif

typedef unsigned long long int bigclock_t;

static const int tmCount = 5;

static const int tmNon = -1;
static const int tmAdd = 0;
static const int tmDel = 1;
static const int tmMov = 2;
static const int tmPth = 3;
static const int tmSev = 4;


static const bool timerStart = true;
static const bool timerDelay = false;


class Timer
{
    public:
        Timer();
        void Register(const int t, const bool start = timerDelay);
        void Start(void);
        void Stop(void);
        void Reset(void);
        void Print(const int t);
        void PrintAll(void);

    private:
        clock_t cStart[tmCount];
        bigclock_t cTotal[tmCount];
        bigclock_t cPath[tmCount];
        int cTally[tmCount];
        int cPathTally[tmCount];
        clock_t cMax[tmCount];
        clock_t cPathMax[tmCount];

        bool running;
        long count;
        int type, lasttype;
};


}

#endif
