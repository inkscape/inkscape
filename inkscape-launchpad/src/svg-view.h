#ifndef SEEN_SP_SVG_VIEW_H
#define SEEN_SP_SVG_VIEW_H
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/view/view.h"

struct SPCanvasGroup;
struct SPCanvasItem;

/**
 * Generic SVG view.
 */
class SPSVGView : public Inkscape::UI::View::View {
public:	
    unsigned int    _dkey;
    SPCanvasGroup  *_parent;
    SPCanvasItem   *_drawing;
    double          _hscale;     ///< horizontal scale
    double          _vscale;     ///< vertical scale
    bool            _rescale;    ///< whether to rescale automatically
    bool            _keepaspect;
    double          _width;
    double          _height;


    /**
     * Constructs new SPSVGView object and returns pointer to it.
     */
    SPSVGView(SPCanvasGroup* parent);

    virtual ~SPSVGView();
        
    /**
     * Rescales SPSVGView to given proportions.
     */
    void setScale(double hscale, double vscale);
    
    /**
     * Rescales SPSVGView and keeps aspect ratio.
     */
    void setRescale(bool rescale, bool keepaspect, double width, double height);

    /**
     * Helper function that sets rescale ratio and emits resize event.
     */
    void doRescale(bool event);

    /**
     * Callback connected with set_document signal.
     */
    virtual void setDocument(SPDocument *document);

    virtual void mouseover();

    virtual void mouseout();

    virtual bool shutdown() { return true; }

private:
    virtual void onPositionSet(double, double) {}
    virtual void onResized(double, double) {}
    virtual void onRedrawRequested() {}
    virtual void onStatusMessage(Inkscape::MessageType /*type*/, gchar const */*message*/) {}
    virtual void onDocumentURISet(gchar const* /*uri*/) {}

    /**
     * Callback connected with document_resized signal.
     */
    virtual void onDocumentResized(double, double);
};

#endif // SEEN_SP_SVG_VIEW_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
