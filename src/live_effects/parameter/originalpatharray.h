#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_ORIGINALPATHARRAY_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_ORIGINALPATHARRAY_H

/*
 * Inkscape::LivePathEffectParameters
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/scrolledwindow.h>

#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/path-reference.h"

#include "svg/svg.h"
#include "svg/stringstream.h"
#include "path-reference.h"
#include "sp-object.h"

namespace Inkscape {

namespace LivePathEffect {

class PathAndDirection {
public:
    PathAndDirection(SPObject *owner)
    : href(NULL),
    ref(owner),
    _pathvector(Geom::PathVector()),
    reversed(false)
    {
        
    }
    gchar *href;
    URIReference ref;
    std::vector<Geom::Path> _pathvector;
    bool reversed;
    
    sigc::connection linked_changed_connection;
    sigc::connection linked_delete_connection;
    sigc::connection linked_modified_connection;
    sigc::connection linked_transformed_connection;
};
    
class OriginalPathArrayParam : public Parameter {
public:
    class ModelColumns;
    
    OriginalPathArrayParam( const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect);

    virtual ~OriginalPathArrayParam();

    virtual Gtk::Widget * param_newWidget();
    virtual bool param_readSVGValue(const gchar * strvalue);
    virtual gchar * param_getSVGValue() const;
    virtual void param_set_default();
    
    /** Disable the canvas indicators of parent class by overriding this method */
    virtual void param_editOncanvas(SPItem * /*item*/, SPDesktop * /*dt*/) {};
    /** Disable the canvas indicators of parent class by overriding this method */
    virtual void addCanvasIndicators(SPLPEItem const* /*lpeitem*/, std::vector<Geom::PathVector> & /*hp_vec*/) {};
    
    std::vector<PathAndDirection*> _vector;
    
protected:
    bool _updateLink(const Gtk::TreeIter& iter, PathAndDirection* pd);
    bool _selectIndex(const Gtk::TreeIter& iter, int* i);
    void unlink(PathAndDirection* to);
    void remove_link(PathAndDirection* to);
    void setPathVector(SPObject *linked_obj, guint flags, PathAndDirection* to);
    
    void linked_changed(SPObject *old_obj, SPObject *new_obj, PathAndDirection* to);
    void linked_modified(SPObject *linked_obj, guint flags, PathAndDirection* to);
    void linked_transformed(Geom::Affine const *, SPItem *, PathAndDirection*) {}
    void linked_delete(SPObject *deleted, PathAndDirection* to);
    
    ModelColumns *_model;
    Glib::RefPtr<Gtk::TreeStore> _store;
    Gtk::TreeView _tree;
    Gtk::CellRendererText *_text_renderer;
    Gtk::CellRendererToggle *_toggle_renderer;
    Gtk::TreeView::Column *_name_column;
    Gtk::ScrolledWindow _scroller;
    
    void on_link_button_click();
    void on_remove_button_click();
    void on_up_button_click();
    void on_down_button_click();
    void on_reverse_toggled(const Glib::ustring& path);
    
private:
    OriginalPathArrayParam(const OriginalPathArrayParam&);
    OriginalPathArrayParam& operator=(const OriginalPathArrayParam&);
};

} //namespace LivePathEffect

} //namespace Inkscape

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
