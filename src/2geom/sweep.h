/**
 * \file
 * \brief  \todo brief description
 *
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

#ifndef __2GEOM_SWEEP_H__
#define __2GEOM_SWEEP_H__

#include <vector>
#include <2geom/d2.h>

namespace Geom {

struct Event {
    double x;
    unsigned ix;
    bool closing;
    Event(double pos, unsigned i, bool c) : x(pos), ix(i), closing(c) {}
// Lexicographic ordering by x then closing
    bool operator<(Event const &other) const {
        if(x < other.x) return true;
        if(x > other.x) return false;
        return closing < other.closing;
    }

};
std::vector<std::vector<unsigned> > sweep_bounds(std::vector<Rect>);
std::vector<std::vector<unsigned> > sweep_bounds(std::vector<Rect>, std::vector<Rect>);

std::vector<std::vector<unsigned> > fake_cull(unsigned a, unsigned b);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
