/*
 * A simple dialog for layer UI.
 *
 * Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2006,2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_LAYERS_PANEL_H
#define SEEN_LAYERS_PANEL_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/box.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/scrolledwindow.h>
#include "ui/widget/spinbutton.h"
#include "ui/widget/panel.h"
#include "ui/widget/object-composite-settings.h"
#include "desktop-tracker.h"
#include "ui/widget/style-subject.h"

class SPObject;

namespace Inkscape {

class LayerManager;

namespace UI {
namespace Dialog {


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

    bool _handleButtonEvent(GdkEventButton *event);
    bool _handleKeyEvent(GdkEventKey *event);
    bool _handleDragDrop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time);
    void _handleEdited(const Glib::ustring& path, const Glib::ustring& new_text);
    void _handleEditingCancelled();

    void _doTreeMove();
    void _renameLayer(Gtk::TreeModel::Row row, const Glib::ustring& name);

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

    DesktopTracker deskTrack;
    int _maxNestDepth;
    SPDesktop* _desktop;
    ModelColumns* _model;
    InternalUIBounce* _pending;
    gboolean _dnd_into;
    SPItem* _dnd_source;
    SPItem* _dnd_target;
    GdkEvent* _toggleEvent;

    Glib::RefPtr<Gtk::TreeStore> _store;
    std::vector<Gtk::Widget*> _watching;
    std::vector<Gtk::Widget*> _watchingNonTop;
    std::vector<Gtk::Widget*> _watchingNonBottom;

    Gtk::TreeView _tree;
    Gtk::CellRendererText *_text_renderer;
    Gtk::TreeView::Column *_name_column;
#if WITH_GTKMM_3_0
    Gtk::Box _buttonsRow;
    Gtk::Box _buttonsPrimary;
    Gtk::Box _buttonsSecondary;
#else
    Gtk::HBox _buttonsRow;
    Gtk::HBox _buttonsPrimary;
    Gtk::HBox _buttonsSecondary;
#endif
    Gtk::ScrolledWindow _scroller;
    Gtk::Menu _popupMenu;
    Inkscape::UI::Widget::SpinButton _spinBtn;
    Gtk::VBox _layersPage;

    UI::Widget::StyleSubject::CurrentLayer _subject;
    UI::Widget::ObjectCompositeSettings _compositeSettings;
    sigc::connection desktopChangeConn;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
