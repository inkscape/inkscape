#ifndef __RUBBERBAND_H__
#define __RUBBERBAND_H__

/**
 * \file src/rubberband.h
 * \brief Rubberbanding selector
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"
#include "libnr/nr-forward.h"
#include "libnr/nr-point.h"
#include "libnr/nr-maybe.h"

/* fixme: do multidocument safe */

class CtrlRect;

namespace Inkscape
{

class Rubberband
{
public:

    void start(SPDesktop *desktop, NR::Point const &p);
    void move(NR::Point const &p);
    NR::Maybe<NR::Rect> getRectangle() const;
    void stop();

    static Rubberband* get();

private:

    Rubberband();
    static Rubberband* _instance;
    
    SPDesktop *_desktop;
    NR::Point _start;
    NR::Point _end;
    CtrlRect *_canvas;
};

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
