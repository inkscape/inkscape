#ifndef INKSCAPE_SP_NAMEDVIEW_H
#define INKSCAPE_SP_NAMEDVIEW_H

/*
 * <sodipodi:namedview> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) Lauris Kaplinski 2000-2002
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_TYPE_NAMEDVIEW (sp_namedview_get_type())
#define SP_NAMEDVIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_NAMEDVIEW, SPNamedView))
#define SP_NAMEDVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_NAMEDVIEW, SPNamedViewClass))
#define SP_IS_NAMEDVIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_NAMEDVIEW))
#define SP_IS_NAMEDVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_NAMEDVIEW))

#include "helper/helper-forward.h"
#include "sp-object-group.h"
#include "libnr/nr-point.h"
#include "sp-metric.h"
#include "snap.h"

enum {
    SP_BORDER_LAYER_BOTTOM,
    SP_BORDER_LAYER_TOP
};

struct SPNamedView : public SPObjectGroup {
    unsigned int editable : 1;
    unsigned int showgrid : 1;
    unsigned int showguides : 1;
    unsigned int showborder : 1;
    unsigned int showpageshadow : 1;
    unsigned int borderlayer : 2;

    double zoom;
    double cx;
    double cy;
    gint window_width;
    gint window_height;
    gint window_x;
    gint window_y;

    SnapManager snap_manager;

    SPUnit const *gridunit;
    /* Grid data is in points regardless of unit */
    NR::Point gridorigin;
    gdouble gridspacing[2];
    gint gridempspacing;

    SPUnit const *doc_units;

    SPUnit const *gridtoleranceunit;
    gdouble gridtolerance;

    SPUnit const *guidetoleranceunit;
    gdouble guidetolerance;

    SPUnit const *objecttoleranceunit;
    gdouble objecttolerance;

    bool has_abs_tolerance;
    
    GQuark default_layer_id;
    
    double connector_spacing;

    guint32 gridcolor;
    guint32 gridempcolor;
    guint32 guidecolor;
    guint32 guidehicolor;
    guint32 bordercolor;
    guint32 pagecolor;
    guint32 pageshadow;

    GSList *guides;
    GSList *views;
    GSList *gridviews;
    gint viewcount;

    void show(SPDesktop *desktop);
    void hide(SPDesktop const *desktop);
    void activateGuides(gpointer desktop, bool active);
    gchar const *getName() const;
    guint getViewCount();
    GSList const *getViewList() const;
    SPMetric getDefaultMetric() const;
};

struct SPNamedViewClass {
    SPObjectGroupClass parent_class;
};

GType sp_namedview_get_type();

SPNamedView *sp_document_namedview(SPDocument *document, gchar const *name);

void sp_namedview_window_from_document(SPDesktop *desktop);
void sp_namedview_document_from_window(SPDesktop *desktop);

void sp_namedview_toggle_guides(SPDocument *doc, Inkscape::XML::Node *repr);
void sp_namedview_toggle_grid(SPDocument *doc, Inkscape::XML::Node *repr);

#endif /* !INKSCAPE_SP_NAMEDVIEW_H */


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
