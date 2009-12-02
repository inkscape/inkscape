/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2009  Monash University
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

#ifndef AVOID_ASSERTIONS_H
#define AVOID_ASSERTIONS_H

#ifdef NDEBUG 

  #define COLA_ASSERT(expr)  static_cast<void>(0)

#else // Not NDEBUG

  #if defined(USE_ASSERT_EXCEPTIONS)

    #include "libvpsc/assertions.h"

  #else

    #include <cassert>
    #define COLA_ASSERT(expr)  assert(expr)

  #endif

#endif


#endif // AVOID_ASSERTIONS_H

