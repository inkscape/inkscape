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

#ifndef SEEN_LAYERS_PANEL_H
#define SEEN_LAYERS_PANEL_H

#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/scale.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/notebook.h>

//#include "ui/previewholder.h"
#include "ui/widget/panel.h"
#include "ui/widget/object-composite-settings.h"

class SPObject;

namespace Inkscape {

class LayerManager;

namespace UI {
namespace Dialogs {


/**
 * A panel that displays layers.
 */
class LayersPanel : public UI::Widget::Panel
{
public:
    LayersPanel();
    virtual ~LayersPanel();

    //virtual void setOrientation( Gtk::AnchorType how );
    
    static LayersPanel& getInstance();

    void setDesktop( SPDesktop* desktop );

protected:
    //virtual void _handleAction( int setId, int itemId );

private:
    class ModelColumns;
    class InternalUIBounce;

    LayersPanel(LayersPanel const &); // no copy
    LayersPanel &operator=(LayersPanel const &); // no assign

    void _styleButton( Gtk::Button& btn, SPDesktop *desktop, unsigned int code, char const* iconName, char const* fallback );
    void _fireAction( unsigned int code );
    Gtk::MenuItem& _addPopupItem( SPDesktop *desktop, unsigned int code, char const* iconName, char const* fallback, int id );

    void _preToggle( GdkEvent const *event );
    void _toggled( Glib::ustring const& str, int targetCol );

    void _handleButtonEvent(GdkEventButton* evt);
    void _handleRowChange( Gtk::TreeModel::Path const& path, Gtk::TreeModel::iterator const& iter );

    void _pushTreeSelectionToCurrent();
    void _checkTreeSelection();

    void _takeAction( int val );
    bool _executeAction();

    bool _rowSelectFunction( Glib::RefPtr<Gtk::TreeModel> const & model, Gtk::TreeModel::Path const & path, bool b );

    void _updateLayer(SPObject *layer);
    bool _checkForUpdated(const Gtk::TreePath &path, const Gtk::TreeIter& iter, SPObject* layer);

    void _selectLayer(SPObject *layer);
    bool _checkForSelected(const Gtk::TreePath& path, const Gtk::TreeIter& iter, SPObject* layer);

    void _layersChanged();
    void _addLayer( SPDocument* doc, SPObject* layer, Gtk::TreeModel::Row* parentRow, SPObject* target, int level );

    SPObject* _selectedLayer();

    // Hooked to the layer manager:
    sigc::connection _layerChangedConnection;
    sigc::connection _layerUpdatedConnection;
    sigc::connection _changedConnection;
    sigc::connection _addedConnection;
    sigc::connection _removedConnection;

    // Internal
    sigc::connection _selectedConnection;

    int _maxNestDepth;
    Inkscape::LayerManager* _mgr;
    SPDesktop* _desktop;
    ModelColumns* _model;
    InternalUIBounce* _pending;
    GdkEvent* _toggleEvent;
    Glib::RefPtr<Gtk::TreeStore> _store;
    std::vector<Gtk::Widget*> _watching;
    std::vector<Gtk::Widget*> _watchingNonTop;
    std::vector<Gtk::Widget*> _watchingNonBottom;

    Gtk::Tooltips _tips;
    Gtk::TreeView _tree;
    Gtk::HButtonBox _buttonsRow;
    Gtk::ScrolledWindow _scroller;
    Gtk::Menu _popupMenu;
    Gtk::SpinButton _spinBtn;
    Gtk::Notebook _notebook;
    Gtk::VBox _layersPage;

    UI::Widget::StyleSubject::CurrentLayer _subject;
    UI::Widget::ObjectCompositeSettings _compositeSettings;
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
