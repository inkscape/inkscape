/**
 * @file
 * SVG <hatch> implementation
 */
/*
 * Authors:
 *   Tomasz Boczkowski <penginsbacon@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2014 Tomasz Boczkowski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_SP_HATCH_H
#define SEEN_SP_HATCH_H

#include <list>
#include <stddef.h>
#include <glibmm/ustring.h>
#include <sigc++/connection.h>

#include "svg/svg-length.h"
#include "svg/svg-angle.h"
#include "sp-paint-server.h"
#include "uri-references.h"

class SPHatchReference;
class SPHatchPath;
class SPItem;

namespace Inkscape {

class Drawing;
class DrawingPattern;

namespace XML {

class Node;

}
}

class SPHatch : public SPPaintServer {
public:
    enum HatchUnits {
        UNITS_USERSPACEONUSE,
        UNITS_OBJECTBOUNDINGBOX
    };

    class RenderInfo {
    public:
        RenderInfo();
        ~RenderInfo();

        Geom::Affine child_transform;
        Geom::Affine pattern_to_user_transform;
        Geom::Rect tile_rect;

        int overflow_steps;
        Geom::Affine overflow_step_transform;
        Geom::Affine overflow_initial_transform;
    };

    SPHatch();
    virtual ~SPHatch();

    // Reference (href)
    Glib::ustring href;
    SPHatchReference *ref;

    gdouble x() const;
    gdouble y() const;
    gdouble pitch() const;
    gdouble rotate() const;
    HatchUnits hatchUnits() const;
    HatchUnits hatchContentUnits() const;
    Geom::Affine const &hatchTransform() const;
    SPHatch *rootHatch(); //TODO: const

    std::vector<SPHatchPath *> hatchPaths();
    std::vector<SPHatchPath const *> hatchPaths() const;

    bool isValid() const;

    Inkscape::DrawingPattern *show(Inkscape::Drawing &drawing, unsigned int key, Geom::OptRect bbox);
    void hide(unsigned int key);
    virtual cairo_pattern_t* pattern_new(cairo_t *ct, Geom::OptRect const &bbox, double opacity);

    RenderInfo calculateRenderInfo(unsigned key) const;
    Geom::Interval bounds() const;
    void setBBox(unsigned int key, Geom::OptRect const &bbox);

protected:
    virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
    virtual void release();
    virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);
    virtual void set(unsigned int key, const gchar* value);
    virtual void update(SPCtx* ctx, unsigned int flags);
    virtual void modified(unsigned int flags);

private:
    class View {
    public:
        View(Inkscape::DrawingPattern *arenaitem, int key);
        //Do not delete arenaitem in destructor.

        ~View();

        Inkscape::DrawingPattern *arenaitem;
        Geom::OptRect bbox;
        unsigned int key;
    };

    typedef std::vector<SPHatchPath *>::iterator ChildIterator;
    typedef std::vector<SPHatchPath const *>::const_iterator ConstChildIterator;
    typedef std::list<View>::iterator ViewIterator;
    typedef std::list<View>::const_iterator ConstViewIterator;

    static bool _hasHatchPatchChildren(SPHatch const* hatch);

    void _updateView(View &view);
    RenderInfo _calculateRenderInfo(View const &view) const;
    Geom::OptInterval _calculateStripExtents(Geom::OptRect const &bbox) const;


    /**
     * Gets called when the hatch is reattached to another <hatch>
     */
    void _onRefChanged(SPObject *old_ref, SPObject *ref);

    /**
     * Gets called when the referenced <hatch> is changed
     */
    void _onRefModified(SPObject *ref, guint flags);

    // patternUnits and patternContentUnits attribute
    HatchUnits _hatchUnits : 1;
    bool _hatchUnits_set : 1;
    HatchUnits _hatchContentUnits : 1;
    bool _hatchContentUnits_set : 1;

    // hatchTransform attribute
    Geom::Affine _hatchTransform;
    bool _hatchTransform_set : 1;

    // Strip
    SVGLength _x;
    SVGLength _y;
    SVGLength _pitch;
    SVGAngle _rotate;

    sigc::connection _modified_connection;

    std::list<View> _display;
};


class SPHatchReference : public Inkscape::URIReference {
public:
    SPHatchReference (SPObject *obj)
        : URIReference(obj)
    {}

    SPHatch *getObject() const {
        return reinterpret_cast<SPHatch *>(URIReference::getObject());
    }

protected:
    virtual bool _acceptObject(SPObject *obj) const {
        return dynamic_cast<SPHatch *>(obj) != NULL && URIReference::_acceptObject(obj);
    }
};

#endif // SEEN_SP_HATCH_H

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
