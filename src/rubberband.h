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
#include <boost/optional.hpp>
#include <vector>
#include <2geom/point.h>
#include <2geom/rect.h>

/* fixme: do multidocument safe */

class CtrlRect;
class SPCanvasItem;
class SPCurve;

enum {
    RUBBERBAND_MODE_RECT,
    RUBBERBAND_MODE_TOUCHPATH
};

namespace Inkscape
{

class Rubberband
{
public:

    void start(SPDesktop *desktop, Geom::Point const &p);
    void move(Geom::Point const &p);
    Geom::OptRect getRectangle() const;
    void stop();
    bool is_started();

    inline int getMode() {return _mode;}
    inline std::vector<Geom::Point> getPoints() {return _points;}

    void setMode(int mode);

    static Rubberband* get(SPDesktop *desktop);

private:

    Rubberband(SPDesktop *desktop);
    static Rubberband* _instance;
    
    SPDesktop *_desktop;
    Geom::Point _start;
    Geom::Point _end;

    std::vector<Geom::Point> _points;

    CtrlRect *_rect;
    SPCanvasItem *_touchpath;
    SPCurve *_touchpath_curve;

    void delete_canvas_items();

    bool _started;
    int _mode;
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
