#ifndef INKSCAPE_CANVAS_GRID_H
#define INKSCAPE_CANVAS_GRID_H

/*
 * Inkscape::CXYGrid
 *
 * Generic (and quite unintelligent) grid item for gnome canvas
 *
 * Copyright (C) Johan Engelen 2006-2007 <johan@shouraizou.nl>
 * Copyright (C) Lauris Kaplinski 2000
 *
 */

#include <cstring>
#include <string>

#include <gtkmm/box.h>
#include <gtkmm.h>

#include "display/sp-canvas.h"
#include "xml/repr.h"
#include "ui/widget/color-picker.h"
#include "ui/widget/scalar-unit.h"
#include "ui/widget/registered-widget.h"
#include "ui/widget/registry.h"
#include "xml/node-event-vector.h"
#include "snapper.h"
#include "line-snapper.h"

struct SPDesktop;
struct SPNamedView;
class SPDocument;

namespace Inkscape {


enum GridType {
    GRID_RECTANGULAR = 0,
    GRID_AXONOMETRIC = 1
};
#define GRID_MAXTYPENR 1

#define INKSCAPE_TYPE_GRID_CANVASITEM            (Inkscape::grid_canvasitem_get_type ())
#define INKSCAPE_GRID_CANVASITEM(obj)            (GTK_CHECK_CAST ((obj), INKSCAPE_TYPE_GRID_CANVASITEM, GridCanvasItem))
#define INKSCAPE_GRID_CANVASITEM_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), INKSCAPE_TYPE_GRID_CANVASITEM, GridCanvasItem))
#define INKSCAPE_IS_GRID_CANVASITEM(obj)         (GTK_CHECK_TYPE ((obj), INKSCAPE_TYPE_GRID_CANVASITEM))
#define INKSCAPE_IS_GRID_CANVASITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), INKSCAPE_TYPE_GRID_CANVASITEM))

class CanvasGrid;

/** \brief  All the variables that are tracked for a grid specific
            canvas item. */
struct GridCanvasItem : public SPCanvasItem{
    CanvasGrid *grid; // the owning grid object
};

struct GridCanvasItemClass {
    SPCanvasItemClass parent_class;
};

/* Standard Gtk function */
GtkType grid_canvasitem_get_type (void);



class CanvasGrid {
public:
    virtual ~CanvasGrid();

    // TODO: see effect.h and effect.cpp from live_effects how to link enums to SVGname to typename properly. (johan)
    const char * getName();
    const char * getSVGName();
    GridType     getGridType();
    static const char * getName(GridType type);
    static const char * getSVGName(GridType type);
    static GridType     getGridTypeFromSVGName(const char * typestr);
    static GridType     getGridTypeFromName(const char * typestr);

    static CanvasGrid* NewGrid(SPNamedView * nv, Inkscape::XML::Node * repr, SPDocument *doc, GridType gridtype);
    static void writeNewGridToRepr(Inkscape::XML::Node * repr, SPDocument * doc, GridType gridtype);

    GridCanvasItem * createCanvasItem(SPDesktop * desktop);

    virtual void Update (NR::Matrix const &affine, unsigned int flags) = 0;
    virtual void Render (SPCanvasBuf *buf) = 0;

    virtual void readRepr() = 0;
    virtual void onReprAttrChanged (Inkscape::XML::Node * /*repr*/, const gchar */*key*/, const gchar */*oldval*/, const gchar */*newval*/, bool /*is_interactive*/) = 0;

    Gtk::Widget * newWidget();

    NR::Point origin;     /**< Origin of the grid */
    guint32 color;        /**< Color for normal lines */
    guint32 empcolor;     /**< Color for emphasis lines */
    gint empspacing;      /**< Spacing between emphasis lines */
    
    SPUnit const* gridunit;
    
    Inkscape::XML::Node * repr;
    SPDocument *doc;

    Inkscape::Snapper* snapper;

    static void on_repr_attr_changed (Inkscape::XML::Node * repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive, void * data);

    bool isVisible() { return (isEnabled() &&visible); };
    bool isEnabled();

protected:
    CanvasGrid(SPNamedView * nv, Inkscape::XML::Node * in_repr, SPDocument *in_doc, GridType type);

    virtual Gtk::Widget * newSpecificWidget() = 0;

    GSList * canvasitems;  // list of created canvasitems

    SPNamedView * namedview;

    Inkscape::UI::Widget::Registry _wr;
    bool visible;

    GridType gridtype;

private:
    CanvasGrid(const CanvasGrid&);
    CanvasGrid& operator=(const CanvasGrid&);
};


class CanvasXYGrid : public CanvasGrid {
public:
    CanvasXYGrid(SPNamedView * nv, Inkscape::XML::Node * in_repr, SPDocument * in_doc);
    virtual ~CanvasXYGrid();

    void Update (NR::Matrix const &affine, unsigned int flags);
    void Render (SPCanvasBuf *buf);

    void readRepr();
    void onReprAttrChanged (Inkscape::XML::Node * repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive);

    NR::Point spacing; /**< Spacing between elements of the grid */
    bool scaled[2];    /**< Whether the grid is in scaled mode, which can
                            be different in the X or Y direction, hense two
                            variables */
    NR::Point ow;      /**< Transformed origin by the affine for the zoom */
    NR::Point sw;      /**< Transformed spacing by the affine for the zoom */

protected:
    virtual Gtk::Widget * newSpecificWidget();

private:
    CanvasXYGrid(const CanvasXYGrid&);
    CanvasXYGrid& operator=(const CanvasXYGrid&);

    void updateWidgets();

    bool render_dotted;
};



class CanvasXYGridSnapper : public LineSnapper
{
public:
    CanvasXYGridSnapper(CanvasXYGrid *grid, SnapManager const *sm, Geom::Coord const d);
    bool ThisSnapperMightSnap() const;

private:
    LineList _getSnapLines(Geom::Point const &p) const;
    void _addSnappedLine(SnappedConstraints &sc, Geom::Point const snapped_point, Geom::Coord const snapped_distance, Geom::Point const normal_to_line, const Geom::Point point_on_line) const;
    CanvasXYGrid *grid;
};

}; /* namespace Inkscape */




#endif
