/*
 * A simple dialog for tags UI.
 *
 * Authors:
 *   Theodore Janeczko
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_TAGS_PANEL_H
#define SEEN_TAGS_PANEL_H

#include <gtkmm/box.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/dialog.h>
#include "ui/widget/spinbutton.h"
#include "ui/widget/panel.h"
#include "ui/widget/object-composite-settings.h"
#include "desktop-tracker.h"
#include "ui/widget/style-subject.h"
#include "selection.h"
#include "ui/widget/filter-effect-chooser.h"

class SPObject;
class SPTag;
struct SPColorSelector;

namespace Inkscape {

namespace UI {
namespace Dialog {


/**
 * A panel that displays layers.
 */
class TagsPanel : public UI::Widget::Panel
{
public:
    TagsPanel();
    virtual ~TagsPanel();

    //virtual void setOrientation( Gtk::AnchorType how );

    static TagsPanel& getInstance();

    void setDesktop( SPDesktop* desktop );
    void setDocument( SPDesktop* desktop, SPDocument* document);

protected:
    //virtual void _handleAction( int setId, int itemId );
    friend void sp_highlight_picker_color_mod(SPColorSelector *csel, GObject *cp);
private:
    class ModelColumns;
    class InternalUIBounce;
    class ObjectWatcher;

    TagsPanel(TagsPanel const &); // no copy
    TagsPanel &operator=(TagsPanel const &); // no assign

    void _styleButton( Gtk::Button& btn, char const* iconName, char const* tooltip );
    void _fireAction( unsigned int code );
    Gtk::MenuItem& _addPopupItem( SPDesktop *desktop, unsigned int code, char const* iconName, char const* fallback, int id );
    
    bool _handleButtonEvent(GdkEventButton *event);
    bool _handleKeyEvent(GdkEventKey *event);
    
    void _storeDragSource(const Gtk::TreeModel::iterator& iter);
    bool _handleDragDrop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time);
    void _handleEdited(const Glib::ustring& path, const Glib::ustring& new_text);
    void _handleEditingCancelled();

    void _doTreeMove();
    void _renameObject(Gtk::TreeModel::Row row, const Glib::ustring& name);

    void _pushTreeSelectionToCurrent();
    void _selected_row_callback( const Gtk::TreeModel::iterator& iter );
    void _select_tag( SPTag * tag );
    
    void _checkTreeSelection();

    void _takeAction( int val );
    bool _executeAction();

    void _setExpanded( const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path, bool isexpanded );
    
    bool _noSelection( Glib::RefPtr<Gtk::TreeModel> const & model, Gtk::TreeModel::Path const & path, bool b );
    bool _rowSelectFunction( Glib::RefPtr<Gtk::TreeModel> const & model, Gtk::TreeModel::Path const & path, bool b );

    void _updateObject(SPObject *obj);
    bool _checkForUpdated(const Gtk::TreePath &path, const Gtk::TreeIter& iter, SPObject* obj);

    void _objectsSelected(Selection *sel);
    bool _checkForSelected(const Gtk::TreePath& path, const Gtk::TreeIter& iter, SPObject* layer);

    void _objectsChanged(SPObject *root);
    void _addObject( SPDocument* doc, SPObject* obj, Gtk::TreeModel::Row* parentRow );
    
    void _checkForDeleted(const Gtk::TreeIter& iter, std::vector<SPObject *>* todelete);

//    std::vector<sigc::connection> groupConnections;
    TagsPanel::ObjectWatcher* _rootWatcher;
    std::vector<TagsPanel::ObjectWatcher*> _objectWatchers;

    // Hooked to the layer manager:
    sigc::connection _documentChangedConnection;
    sigc::connection _selectionChangedConnection;
    
    sigc::connection _changedConnection;
    sigc::connection _addedConnection;
    sigc::connection _removedConnection;

    // Internal
    sigc::connection _selectedConnection;
    sigc::connection _expandedConnection;
    sigc::connection _collapsedConnection;
    
    DesktopTracker deskTrack;
    SPDesktop* _desktop;
    SPDocument* _document;
    ModelColumns* _model;
    InternalUIBounce* _pending;
    gboolean _dnd_into;
    std::vector<SPTag*> _dnd_source;
    SPObject* _dnd_target;
    
    GdkEvent* _toggleEvent;
    bool down_at_add;
    
    Gtk::TreeModel::Path _defer_target;

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

    sigc::connection desktopChangeConn;
    
};



} //namespace Dialogs
} //namespace UI
} //namespace Inkscape



#endif // SEEN_OBJECTS_PANEL_H

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
