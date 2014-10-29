/*
 * A simple dialog for objects UI.
 *
 * Authors:
 *   Theodore Janeczko
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_OBJECTS_PANEL_H
#define SEEN_OBJECTS_PANEL_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if GLIBMM_DISABLE_DEPRECATED && HAVE_GLIBMM_THREADS_H
# include <glibmm/threads.h>
#endif

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
class SPGroup;
struct SPColorSelector;

namespace Inkscape {

namespace UI {
namespace Dialog {


/**
 * A panel that displays objects.
 */
class ObjectsPanel : public UI::Widget::Panel
{
public:
    ObjectsPanel();
    virtual ~ObjectsPanel();

    static ObjectsPanel& getInstance();

    void setDesktop( SPDesktop* desktop );
    void setDocument( SPDesktop* desktop, SPDocument* document);

private:
    //Internal Classes:
    class ModelColumns;
    class InternalUIBounce;
    class ObjectWatcher;

    //Connections, Watchers, Trackers:
    
    //Document root watcher
    ObjectsPanel::ObjectWatcher* _rootWatcher;
    
    //All object watchers
    std::vector<ObjectsPanel::ObjectWatcher*> _objectWatchers;
    
    //Connection for when the desktop changes
    sigc::connection desktopChangeConn;

    //Connection for when the document changes
    sigc::connection _documentChangedConnection;
    
    //Connection for when the active selection in the document changes
    sigc::connection _selectionChangedConnection;

    //Connection for when the selection in the dialog changes
    sigc::connection _selectedConnection;
    
    //Connections for when the opacity/blend/blur of the active selection in the document changes
    sigc::connection _opacityConnection;
    sigc::connection _blendConnection;
    sigc::connection _blurConnection;
    
    //Desktop tracker for grabbing the desktop changed connection
    DesktopTracker _deskTrack;
    
    //Members:
    
    //The current desktop
    SPDesktop* _desktop;
    
    //The current document
    SPDocument* _document;
    
    //Tree data model
    ModelColumns* _model;
    
    //Prevents the composite controls from updating
    bool _blockCompositeUpdate;
    
    //
    InternalUIBounce* _pending;
    
    //Whether the drag & drop was dragged into an item
    gboolean _dnd_into;
    
    //List of drag & drop source items
    std::vector<SPItem*> _dnd_source;
    
    //Drag & drop target item
    SPItem* _dnd_target;
    
    //List of items to change the highlight on
    std::vector<SPItem*> _highlight_target;

    //GUI Members:
    
    GdkEvent* _toggleEvent;
    
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
    Gtk::VBox _page;
    
    /* Composite Settings */
    Gtk::VBox       _composite_vbox;
    Gtk::VBox       _opacity_vbox;
    Gtk::HBox       _opacity_hbox;
    Gtk::Label      _opacity_label;
    Gtk::Label      _opacity_label_unit;
#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> _opacity_adjustment;
#else
    Gtk::Adjustment _opacity_adjustment;
#endif
    Gtk::HScale     _opacity_hscale;
    Inkscape::UI::Widget::SpinButton _opacity_spin_button;
    
    Inkscape::UI::Widget::SimpleFilterModifier _fe_cb;
    Gtk::VBox       _fe_vbox;
    Gtk::Alignment  _fe_alignment;
    Inkscape::UI::Widget::SimpleFilterModifier _fe_blur;
    Gtk::VBox       _blur_vbox;
    Gtk::Alignment  _blur_alignment;

    Gtk::Dialog _colorSelectorDialog;
    SPColorSelector *_colorSelector;

    
    //Methods:
    
    ObjectsPanel(ObjectsPanel const &); // no copy
    ObjectsPanel &operator=(ObjectsPanel const &); // no assign

    void _styleButton( Gtk::Button& btn, char const* iconName, char const* tooltip );
    void _fireAction( unsigned int code );
    
    Gtk::MenuItem& _addPopupItem( SPDesktop *desktop, unsigned int code, char const* iconName, char const* fallback, int id );
    
    void _setVisibleIter( const Gtk::TreeModel::iterator& iter, const bool visible );
    void _setLockedIter( const Gtk::TreeModel::iterator& iter, const bool locked );
    
    bool _handleButtonEvent(GdkEventButton *event);
    bool _handleKeyEvent(GdkEventKey *event);
    
    void _storeHighlightTarget(const Gtk::TreeModel::iterator& iter);
    void _storeDragSource(const Gtk::TreeModel::iterator& iter);
    bool _handleDragDrop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time);
    void _handleEdited(const Glib::ustring& path, const Glib::ustring& new_text);
    void _handleEditingCancelled();

    void _doTreeMove();
    void _renameObject(Gtk::TreeModel::Row row, const Glib::ustring& name);

    void _pushTreeSelectionToCurrent();
    void _selected_row_callback( const Gtk::TreeModel::iterator& iter, bool *setOpacity );
    
    void _checkTreeSelection();

    void _takeAction( int val );
    bool _executeAction();

    void _setExpanded( const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path, bool isexpanded );
    void _setCollapsed(SPGroup * group);
    
    bool _noSelection( Glib::RefPtr<Gtk::TreeModel> const & model, Gtk::TreeModel::Path const & path, bool b );
    bool _rowSelectFunction( Glib::RefPtr<Gtk::TreeModel> const & model, Gtk::TreeModel::Path const & path, bool b );

    void _compositingChanged( const Gtk::TreeModel::iterator& iter, bool *setValues );
    void _updateComposite();
    void _setCompositingValues(SPItem *item);
    
    void _updateObject(SPObject *obj, bool recurse);
    bool _checkForUpdated(const Gtk::TreeIter& iter, SPObject* obj);

    void _objectsSelected(Selection *sel);
    bool _checkForSelected(const Gtk::TreePath& path, const Gtk::TreeIter& iter, SPItem* item, bool scrollto);

    void _objectsChanged(SPObject *obj);
    void _addObject( SPObject* obj, Gtk::TreeModel::Row* parentRow );
    
    void _opacityChangedIter(const Gtk::TreeIter& iter);
    void _opacityValueChanged();
    
    void _blendChangedIter(const Gtk::TreeIter& iter, Glib::ustring blendmode);
    void _blendValueChanged();

    void _blurChangedIter(const Gtk::TreeIter& iter, double blur);
    void _blurValueChanged();

    
    void setupDialog(const Glib::ustring &title);
    
    friend void sp_highlight_picker_color_mod(SPColorSelector *csel, GObject *cp);

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
