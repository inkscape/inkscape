#ifndef INKSCAPE_SP_NAMEDVIEW_H
#define INKSCAPE_SP_NAMEDVIEW_H

/*
 * <sodipodi:namedview> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) Lauris Kaplinski 2000-2002
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_NAMEDVIEW(obj) (dynamic_cast<SPNamedView*>((SPObject*)obj))
#define SP_IS_NAMEDVIEW(obj) (dynamic_cast<const SPNamedView*>((SPObject*)obj) != NULL)

#include "sp-object-group.h"
#include "snap.h"
#include "document.h"
#include "util/units.h"
#include <vector>

namespace Inkscape {
    class CanvasGrid;
    namespace Util {
        class Unit;
    }
}

typedef unsigned int guint32;
typedef guint32 GQuark;

enum {
    SP_BORDER_LAYER_BOTTOM,
    SP_BORDER_LAYER_TOP
};

class SPNamedView : public SPObjectGroup {
public:
	SPNamedView();
	virtual ~SPNamedView();

    unsigned int editable : 1;
    unsigned int showguides : 1;
    unsigned int lockguides : 1;
    unsigned int pagecheckerboard : 1;
    unsigned int showborder : 1;
    unsigned int showpageshadow : 1;
    unsigned int borderlayer : 2;

    double zoom;
    double cx;
    double cy;
    int window_width;
    int window_height;
    int window_x;
    int window_y;
    int window_maximized;

    SnapManager snap_manager;
    std::vector<Inkscape::CanvasGrid *> grids;
    bool grids_visible;

    Inkscape::Util::Unit const *display_units;   // Units used for the UI (*not* the same as units of SVG coordinates)
    Inkscape::Util::Unit const *page_size_units; // Only used in "Custom size" part of Document Properties dialog 
    
    GQuark default_layer_id;

    double connector_spacing;

    guint32 guidecolor;
    guint32 guidehicolor;
    guint32 bordercolor;
    guint32 pagecolor;
    guint32 pageshadow;

    std::vector<SPGuide *> guides;
    std::vector<SPDesktop *> views;

    int viewcount;

    void show(SPDesktop *desktop);
    void hide(SPDesktop const *desktop);
    void activateGuides(void* desktop, bool active);
    char const *getName() const;
    unsigned int getViewCount();
    std::vector<SPDesktop *> const getViewList() const;
    Inkscape::Util::Unit const * getDisplayUnit() const;

    void translateGuides(Geom::Translate const &translation);
    void translateGrids(Geom::Translate const &translation);
    void scrollAllDesktops(double dx, double dy, bool is_scrolling);
    void writeNewGrid(SPDocument *document,int gridtype);
    bool getSnapGlobal() const;
    void setSnapGlobal(bool v);
    void setGuides(bool v);
    bool getGuides();

private:
    double getMarginLength(gchar const * const key,Inkscape::Util::Unit const * const margin_units,Inkscape::Util::Unit const * const return_units,double const width,double const height,bool const use_width);
    friend class SPDocument;

protected:
	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void release();
	virtual void set(unsigned int key, char const* value);

	virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);
	virtual void remove_child(Inkscape::XML::Node* child);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
};


SPNamedView *sp_document_namedview(SPDocument *document, char const *name);
SPNamedView const *sp_document_namedview(SPDocument const *document, char const *name);

void sp_namedview_window_from_document(SPDesktop *desktop);
void sp_namedview_document_from_window(SPDesktop *desktop);
void sp_namedview_update_layers_from_document (SPDesktop *desktop);

void sp_namedview_toggle_guides(SPDocument *doc, Inkscape::XML::Node *repr);
void sp_namedview_guides_toggle_lock(SPDocument *doc, Inkscape::XML::Node *repr);
void sp_namedview_show_grids(SPNamedView *namedview, bool show, bool dirty_document);
Inkscape::CanvasGrid * sp_namedview_get_first_enabled_grid(SPNamedView *namedview);


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
