#ifndef SEEN_GRADIENT_SELECTOR_H
#define SEEN_GRADIENT_SELECTOR_H

/*
 * Gradient vector and position widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>

#include <vector>
#include "sp-gradient-spread.h"
#include "sp-gradient-units.h"

class SPDocument;
class SPGradient;

namespace Gtk {
class CellRendererPixbuf;
class CellRendererText;
class ScrolledWindow;
class TreeView;
}

#define SP_TYPE_GRADIENT_SELECTOR (sp_gradient_selector_get_type ())
#define SP_GRADIENT_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_GRADIENT_SELECTOR, SPGradientSelector))
#define SP_GRADIENT_SELECTOR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), SP_TYPE_GRADIENT_SELECTOR, SPGradientSelectorClass))
#define SP_IS_GRADIENT_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_GRADIENT_SELECTOR))
#define SP_IS_GRADIENT_SELECTOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), SP_TYPE_GRADIENT_SELECTOR))



struct SPGradientSelector {
#if GTK_CHECK_VERSION(3,0,0)
    GtkBox  vbox;
#else
    GtkVBox vbox;
#endif

    enum SelectorMode {
        MODE_LINEAR,
        MODE_RADIAL,
#ifdef WITH_MESH
        MODE_MESH,
#endif
        MODE_SWATCH
    };

    SelectorMode mode;

    SPGradientUnits gradientUnits;
    SPGradientSpread gradientSpread;

    /* Vector selector */
    GtkWidget *vectors;

    /* Tree */
    bool _checkForSelected(const Gtk::TreePath& path, const Gtk::TreeIter& iter, SPGradient *vector);
    void onTreeSelection();
    void onGradientRename( const Glib::ustring& path_string, const Glib::ustring& new_text);
    void onTreeNameColClick();
    void onTreeColorColClick();
    void onTreeCountColClick();

    Gtk::TreeView     *treeview;
    Gtk::ScrolledWindow *scrolled_window;
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
      public:
        ModelColumns()
        {
            add(name);
            add(refcount);
            add(color);
            add(data);
            add(pixbuf);
        }
        virtual ~ModelColumns() {}

        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<unsigned long> color;
        Gtk::TreeModelColumn<gint> refcount;
        Gtk::TreeModelColumn<SPGradient*> data;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > pixbuf;

    };

    ModelColumns *columns;
    Glib::RefPtr<Gtk::ListStore> store;
    Gtk::CellRendererPixbuf* icon_renderer;
    Gtk::CellRendererText* text_renderer;

    /* Editing buttons */
    GtkWidget *edit;
    GtkWidget *add;
    GtkWidget *del;
    GtkWidget *merge;

    /* Position widget */
    GtkWidget *position;

    bool safelyInit;
    bool blocked;

    std::vector<GtkWidget*> nonsolid;
    std::vector<GtkWidget*> swatch_widgets;

    void setMode(SelectorMode mode);
    void setUnits(SPGradientUnits units);
    void setSpread(SPGradientSpread spread);
    void setVector(SPDocument *doc, SPGradient *vector);
    void selectGradientInTree(SPGradient *vector);

    SPGradientUnits getUnits();
    SPGradientSpread getSpread();
    SPGradient *getVector();
};

struct SPGradientSelectorClass {
#if GTK_CHECK_VERSION(3,0,0)
    GtkBoxClass parent_class;
#else
    GtkVBoxClass parent_class;
#endif

    void (* grabbed) (SPGradientSelector *sel);
    void (* dragged) (SPGradientSelector *sel);
    void (* released) (SPGradientSelector *sel);
    void (* changed) (SPGradientSelector *sel);
};

GType sp_gradient_selector_get_type(void);

GtkWidget *sp_gradient_selector_new (void);

void sp_gradient_selector_set_bbox (SPGradientSelector *sel, gdouble x0, gdouble y0, gdouble x1, gdouble y1);

#endif // SEEN_GRADIENT_SELECTOR_H


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
