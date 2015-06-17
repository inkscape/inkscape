/**
 * @file
 * SVG <hatchPath> implementation
 */
/*
 * Author:
 *   Tomasz Boczkowski <penginsbacon@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2014 Tomasz Boczkowski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <string>
#include <2geom/path.h>
#include <2geom/transforms.h>

#include "svg/svg.h"
#include "display/cairo-utils.h"
#include "display/curve.h"
#include "display/drawing-context.h"
#include "display/drawing-surface.h"
#include "display/drawing.h"
#include "display/drawing-shape.h"
#include "helper/geom.h"
#include "attributes.h"
#include "document-private.h"
#include "uri.h"
#include "style.h"
#include "sp-hatch-path.h"
#include "svg/css-ostringstream.h"
#include "xml/repr.h"

SPHatchPath::SPHatchPath()
    : offset(),
      _display(),
      _curve(NULL),
      _continuous(false)
{
    offset.unset();
}

SPHatchPath::~SPHatchPath()
{
}

void SPHatchPath::setCurve(SPCurve *new_curve, bool owner)
{
    if (_curve) {
        _curve = _curve->unref();
    }

    if (new_curve) {
        if (owner) {
            _curve = new_curve->ref();
        } else {
            _curve = new_curve->copy();
        }
    }

    requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void SPHatchPath::build(SPDocument* doc, Inkscape::XML::Node* repr)
{
    SPObject::build(doc, repr);

    readAttr("d");
    readAttr("offset");
    readAttr( "style" );

    style->fill.setNone();
}

void SPHatchPath::release()
{
    for (ViewIterator iter = _display.begin(); iter != _display.end(); ++iter) {
        delete iter->arenaitem;
        iter->arenaitem = NULL;
    }

    SPObject::release();
}

void SPHatchPath::set(unsigned int key, const gchar* value)
{
    switch (key) {
    case SP_ATTR_D:
        if (value) {
            Geom::PathVector pv;
            _readHatchPathVector(value, pv, _continuous);
            SPCurve *curve = new SPCurve(pv);

            if (curve) {
                setCurve(curve, true);
                curve->unref();
            }
        } else {
            setCurve(NULL, true);
        }

        requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    case SP_ATTR_OFFSET:
        offset.readOrUnset(value);
        requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    default:
        if (SP_ATTRIBUTE_IS_CSS(key)) {
            style->readFromObject( this );
            requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        } else {
            SPObject::set(key, value);
        }
        break;
    }
}


void SPHatchPath::update(SPCtx* ctx, unsigned int flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        flags &= ~SP_OBJECT_USER_MODIFIED_FLAG_B;
    }

    if (flags & (SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        if (style->stroke_width.unit == SP_CSS_UNIT_PERCENT) {
            //TODO: Check specification

            SPItemCtx *ictx = static_cast<SPItemCtx *>(ctx);
            double const aw = (ictx) ? 1.0 / ictx->i2vp.descrim() : 1.0;
            style->stroke_width.computed = style->stroke_width.value * aw;

            for (ViewIterator iter = _display.begin(); iter != _display.end(); ++iter) {
                iter->arenaitem->setStyle(style);
            }
        }
    }

    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG)) {
        for (ViewIterator iter = _display.begin(); iter != _display.end(); ++iter) {
            _updateView(*iter);
        }
    }
}

bool SPHatchPath::isValid() const
{
    if (_curve && (_repeatLength() <= 0)) {
        return false;
    } else {
        return true;
    }
}

Inkscape::DrawingItem *SPHatchPath::show(Inkscape::Drawing &drawing, unsigned int key, Geom::OptInterval extents)
{
    Inkscape::DrawingShape *s = new Inkscape::DrawingShape(drawing);
    _display.push_front(View(s, key));
    _display.front().extents = extents;

    _updateView(_display.front());

    return s;
}

void SPHatchPath::hide(unsigned int key)
{
    for (ViewIterator iter = _display.begin(); iter != _display.end(); ++iter) {
        if (iter->key == key) {
            delete iter->arenaitem;
            _display.erase(iter);
            return;
        }
    }

    g_assert_not_reached();
}

void SPHatchPath::setStripExtents(unsigned int key, Geom::OptInterval const &extents)
{
    for (ViewIterator iter = _display.begin(); iter != _display.end(); ++iter) {
        if (iter->key == key) {
            iter->extents = extents;
            break;
        }
    }
}

Geom::Interval SPHatchPath::bounds() const
{
    Geom::OptRect bbox;
    Geom::Interval result;

    Geom::Affine transform = Geom::Translate(offset.computed, 0);
    if (!_curve) {
        SPCurve test_curve;
        test_curve.moveto(Geom::Point(0, 0));
        test_curve.moveto(Geom::Point(0, 1));
        bbox = bounds_exact_transformed(test_curve.get_pathvector(), transform);
    } else {
        bbox = bounds_exact_transformed(_curve->get_pathvector(), transform);
    }

    gdouble stroke_width = style->stroke_width.computed;
    result.setMin(bbox->left() - stroke_width / 2);
    result.setMax(bbox->right() + stroke_width / 2);
    return result;
}

SPCurve *SPHatchPath::calculateRenderCurve(unsigned key) const
{
    for (ConstViewIterator iter = _display.begin(); iter != _display.end(); ++iter) {
        if (iter->key == key) {
            return _calculateRenderCurve(*iter);
        }
    }
    g_assert_not_reached();
    return NULL;
}

gdouble SPHatchPath::_repeatLength() const
{
    gdouble val = 0;

    if (_curve && _curve->last_point()) {
        val = _curve->last_point()->y();
    }

    return val;
}

void SPHatchPath::_updateView(View &view)
{
    SPCurve *calculated_curve = _calculateRenderCurve(view);

    Geom::Affine offset_transform = Geom::Translate(offset.computed, 0);
    view.arenaitem->setTransform(offset_transform);
    style->fill.setNone();
    view.arenaitem->setStyle(style);
    view.arenaitem->setPath(calculated_curve);

    calculated_curve->unref();
}

SPCurve *SPHatchPath::_calculateRenderCurve(View const &view) const
{
    SPCurve *calculated_curve = new SPCurve;

    if (!view.extents) {
        return calculated_curve;
    }

    if (!_curve) {
        calculated_curve->moveto(0, view.extents->min());
        calculated_curve->lineto(0, view.extents->max());
        //TODO: if hatch has a dasharray defined, adjust line ends
    } else {
        gdouble repeatLength = _repeatLength();
        if (repeatLength > 0) {
            gdouble initial_y = floor(view.extents->min() / repeatLength) * repeatLength;
            int segment_cnt = ceil((view.extents->extent()) / repeatLength) + 1;

            SPCurve *segment =_curve->copy();
            segment->transform(Geom::Translate(0, initial_y));

            Geom::Affine step_transform = Geom::Translate(0, repeatLength);
            for (int i = 0; i < segment_cnt; ++i) {
                if (_continuous) {
                    calculated_curve->append_continuous(segment, 0.0625);
                } else {
                    calculated_curve->append(segment, false);
                }
                segment->transform(step_transform);
            }

            segment->unref();
        }
    }
    return calculated_curve;
}


void SPHatchPath::_readHatchPathVector(char const *str, Geom::PathVector &pathv, bool &continous_join)
{
    if (!str) {
        return;
    }

    pathv = sp_svg_read_pathv(str);

    if (!pathv.empty()) {
        continous_join = false;
    } else {
        Glib::ustring str2 = Glib::ustring::compose("M0,0 %1", str);
        pathv = sp_svg_read_pathv(str2.c_str());
        if (pathv.empty()) {
            return;
        }

        gdouble last_point_x = pathv.back().finalPoint().x();
        Inkscape::CSSOStringStream stream;
        stream << last_point_x;
        Glib::ustring str3 = Glib::ustring::compose("M%1,0 %2", stream.str(), str);
        Geom::PathVector pathv3 = sp_svg_read_pathv(str3.c_str());

        //Path can be composed of relative commands only. In this case final point
        //coordinates would depend on first point position. If this happens, fall
        //back to using 0,0 as first path point
        if (pathv3.back().finalPoint().y() == pathv.back().finalPoint().y()) {
            pathv = pathv3;
        }
        continous_join = true;
    }
}

SPHatchPath::View::View(Inkscape::DrawingShape *arenaitem, int key)
    : arenaitem(arenaitem),
      extents(),
      key(key)
{
}

SPHatchPath::View::~View()
{
    // remember, do not delete arenaitem here
    arenaitem = NULL;
}


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
