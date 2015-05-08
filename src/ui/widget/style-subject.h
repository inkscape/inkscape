/**
 * @file
 * Abstraction for different style widget operands.
 */
/*
 * Copyright (C) 2007 MenTaLguY <mental@rydia.net>
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */
#ifndef SEEN_INKSCAPE_UI_WIDGET_STYLE_SUBJECT_H
#define SEEN_INKSCAPE_UI_WIDGET_STYLE_SUBJECT_H

#include <boost/optional.hpp>
#include <2geom/rect.h>
#include "sp-item.h"
#include <stddef.h>
#include <sigc++/sigc++.h>

class SPDesktop;
class SPObject;
class SPCSSAttr;
class SPStyle;

namespace Inkscape {
class Selection;
}

namespace Inkscape {
namespace UI {
namespace Widget {

class StyleSubject {
public:
    class Selection;
    class CurrentLayer;


    StyleSubject();
    virtual ~StyleSubject();

    void setDesktop(SPDesktop *desktop);
    SPDesktop *getDesktop() const { return _desktop; }

    virtual Geom::OptRect getBounds(SPItem::BBoxType type) = 0;
    virtual int queryStyle(SPStyle *query, int property) = 0;
    virtual void setCSS(SPCSSAttr *css) = 0;
    virtual std::vector<SPObject*> list(){return std::vector<SPObject*>();};

    sigc::connection connectChanged(sigc::signal<void>::slot_type slot) {
        return _changed_signal.connect(slot);
    }

protected:
    virtual void _afterDesktopSwitch(SPDesktop */*desktop*/) {}
    void _emitChanged() { _changed_signal.emit(); }

private:
    sigc::signal<void> _changed_signal;
    SPDesktop *_desktop;
};

class StyleSubject::Selection : public StyleSubject {
public:
    Selection();
    ~Selection();

    virtual Geom::OptRect getBounds(SPItem::BBoxType type);
    virtual int queryStyle(SPStyle *query, int property);
    virtual void setCSS(SPCSSAttr *css);
    virtual std::vector<SPObject*> list();

protected:
    virtual void _afterDesktopSwitch(SPDesktop *desktop);

private:
    Inkscape::Selection *_getSelection() const;

    sigc::connection _sel_changed;
    sigc::connection _subsel_changed;
    sigc::connection _sel_modified;
};

class StyleSubject::CurrentLayer : public StyleSubject {
public:
    CurrentLayer();
    ~CurrentLayer();

    virtual Geom::OptRect getBounds(SPItem::BBoxType type);
    virtual int queryStyle(SPStyle *query, int property);
    virtual void setCSS(SPCSSAttr *css);
    virtual std::vector<SPObject*> list();

protected:
    virtual void _afterDesktopSwitch(SPDesktop *desktop);

private:
    SPObject *_getLayer() const;
    void _setLayer(SPObject *layer);
    SPObject *_getLayerSList() const;

    sigc::connection _layer_switched;
    sigc::connection _layer_release;
    sigc::connection _layer_modified;
    mutable SPObject* _element;
};

}
}
}

#endif // SEEN_INKSCAPE_UI_WIDGET_STYLE_SUBJECT_H

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
