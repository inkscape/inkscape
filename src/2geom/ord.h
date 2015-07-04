/** @file
 * @brief Comparator template
 *//*
 * Authors:
 *      ? <?@?.?>
 * 
 * Copyright ?-?  authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 */

#ifndef LIB2GEOM_SEEN_ORD_H
#define LIB2GEOM_SEEN_ORD_H

namespace {

enum Cmp {
  LESS_THAN=-1,
  GREATER_THAN=1,
  EQUAL_TO=0
};

static inline Cmp operator-(Cmp x) {
  switch(x) {
  case LESS_THAN:
    return GREATER_THAN;
  case GREATER_THAN:
    return LESS_THAN;
  case EQUAL_TO:
    return EQUAL_TO;
  }
}

template <typename T1, typename T2>
inline Cmp cmp(T1 const &a, T2 const &b) {
  if ( a < b ) {
    return LESS_THAN;
  } else if ( b < a ) {
    return GREATER_THAN;
  } else {
    return EQUAL_TO;
  }
}

}

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
