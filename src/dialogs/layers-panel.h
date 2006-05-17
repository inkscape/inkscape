
#ifndef SEEN_LAYERS_PANEL_H
#define SEEN_LAYERS_PANEL_H
/*
 * A simple dialog for layer UI.
 *
 * Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2006 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/tooltips.h>

#include "ui/widget/panel.h"
//#include "ui/previewholder.h"

class SPObject;

namespace Inkscape {

class LayerManager;

namespace UI {
namespace Dialogs {


/**
 * A panel that displays layers.
 */
class LayersPanel : public Inkscape::UI::Widget::Panel
{
public:
    LayersPanel();
    virtual ~LayersPanel();

    static LayersPanel& getInstance();
    //virtual void setOrientation( Gtk::AnchorType how );

    void setDesktop( SPDesktop* desktop );

protected:
    //virtual void _handleAction( int setId, int itemId );

private:
    class ModelColumns;

    LayersPanel(LayersPanel const &); // no copy
    LayersPanel &operator=(LayersPanel const &); // no assign

    static LayersPanel* instance;

    void _styleButton( Gtk::Button& btn, SPDesktop *desktop, unsigned int code, char const* iconName, char const* fallback );
    void _fireAction( unsigned int code );

    void _toggled( Glib::ustring const& str, int targetCol );

    void _checkTreeSelection();

    void _takeAction( int val );

    void _selectLayer(SPObject *layer);
    bool _checkForSelected(const Gtk::TreePath& path, const Gtk::TreeIter& iter, SPObject* layer);

    void _layersChanged();

    SPObject* _selectedLayer();

    // Hooked to the desktop:
    sigc::connection _layerChangedConnection;

    // Hooked to the layer manager:
    sigc::connection _changedConnection;
    sigc::connection _addedConnection;
    sigc::connection _removedConnection;

    Inkscape::LayerManager* _mgr;
    SPDesktop* _desktop;
    ModelColumns* _model;
    Glib::RefPtr<Gtk::TreeStore> _store;
    std::vector<Gtk::Button*> _watching;

    Gtk::Tooltips _tips;
    Gtk::TreeView _tree;
    Gtk::HBox _buttonsRow;
};



} //namespace Dialogs
} //namespace UI
} //namespace Inkscape



#endif // SEEN_LAYERS_PANEL_H

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
