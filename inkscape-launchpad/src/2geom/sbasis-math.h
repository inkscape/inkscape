/** @file
 * @brief some std functions to work with (pw)s-basis
 *//*
 *  Authors:
 *   Jean-Francois Barraud
 *
 * Copyright (C) 2006-2007 authors
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
 */

//this a first try to define sqrt, cos, sin, etc...
//TODO: define a truncated compose(sb,sb, order) and extend it to pw<sb>.
//TODO: in all these functions, compute 'order' according to 'tol'.
//TODO: use template to define the pw version automatically from the sb version?

#ifndef LIB2GEOM_SEEN_SBASIS_MATH_H
#define LIB2GEOM_SEEN_SBASIS_MATH_H


#include <2geom/sbasis.h>
#include <2geom/piecewise.h>
#include <2geom/d2.h>

namespace Geom{
//-|x|---------------------------------------------------------------
Piecewise<SBasis> abs(          SBasis const &f);
Piecewise<SBasis> abs(Piecewise<SBasis>const &f);

//- max(f,g), min(f,g) ----------------------------------------------
Piecewise<SBasis> max(          SBasis  const &f,           SBasis  const &g);
Piecewise<SBasis> max(Piecewise<SBasis> const &f,           SBasis  const &g);
Piecewise<SBasis> max(          SBasis  const &f, Piecewise<SBasis> const &g);
Piecewise<SBasis> max(Piecewise<SBasis> const &f, Piecewise<SBasis> const &g);
Piecewise<SBasis> min(          SBasis  const &f,           SBasis  const &g);
Piecewise<SBasis> min(Piecewise<SBasis> const &f,           SBasis  const &g);
Piecewise<SBasis> min(          SBasis  const &f, Piecewise<SBasis> const &g);
Piecewise<SBasis> min(Piecewise<SBasis> const &f, Piecewise<SBasis> const &g);

//-sign(x)---------------------------------------------------------------
Piecewise<SBasis> signSb(          SBasis const &f);
Piecewise<SBasis> signSb(Piecewise<SBasis>const &f);

//-Sqrt---------------------------------------------------------------
Piecewise<SBasis> sqrt(          SBasis const &f, double tol=1e-3, int order=3);
Piecewise<SBasis> sqrt(Piecewise<SBasis>const &f, double tol=1e-3, int order=3);

//-sin/cos--------------------------------------------------------------
Piecewise<SBasis> cos(          SBasis  const &f, double tol=1e-3, int order=3);
Piecewise<SBasis> cos(Piecewise<SBasis> const &f, double tol=1e-3, int order=3);
Piecewise<SBasis> sin(          SBasis  const &f, double tol=1e-3, int order=3);
Piecewise<SBasis> sin(Piecewise<SBasis> const &f, double tol=1e-3, int order=3);
//-Log---------------------------------------------------------------
Piecewise<SBasis> log(          SBasis const &f, double tol=1e-3, int order=3);
Piecewise<SBasis> log(Piecewise<SBasis>const &f, double tol=1e-3, int order=3);

//--1/x------------------------------------------------------------
//TODO: change this...
Piecewise<SBasis> reciprocalOnDomain(Interval range, double tol=1e-3);
Piecewise<SBasis> reciprocal(          SBasis const &f, double tol=1e-3, int order=3);
Piecewise<SBasis> reciprocal(Piecewise<SBasis>const &f, double tol=1e-3, int order=3);

//--interpolate------------------------------------------------------------
Piecewise<SBasis> interpolate( std::vector<double> times, std::vector<double> values, unsigned smoothness = 1);
}

#endif //SEEN_GEOM_PW_SB_CALCULUS_H

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
