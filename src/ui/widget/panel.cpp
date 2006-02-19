/**
 * \brief Panel widget
 *
 * Authors:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 * Copyright (C) 2005 Jon A. Cruz
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>

#include "panel.h"
#include "../../prefs-utils.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 *    Construct a Panel
 *
 *    \param label Label.
 */

Panel::Panel(const gchar *prefs_path) :
    _prefs_path(NULL),
    _fillable(0)
{
    _prefs_path = prefs_path;
    init();
}

Panel::Panel() :
    _prefs_path(NULL),
    _fillable(0)
{
    init();
}

Panel::Panel(Glib::ustring const &label) :
    _prefs_path(NULL),
    _fillable(0)
{
    this->label = label;
    init();
}

Panel::~Panel()
{
}

void Panel::init()
{
    Glib::ustring tmp("<");
    tabTitle.set_label(this->label);

    guint panel_size = 0;
    if (_prefs_path) {
        panel_size = prefs_get_int_attribute_limited (_prefs_path, "panel_size", 1, 0, 10);
    }

    guint panel_mode = 0;
    if (_prefs_path) {
        panel_mode = prefs_get_int_attribute_limited (_prefs_path, "panel_mode", 1, 0, 10);
    }

    tabButton.set_menu(menu);
    Gtk::MenuItem* dummy = manage(new Gtk::MenuItem(tmp));
    menu.append( *dummy );
    menu.append( *manage(new Gtk::SeparatorMenuItem()) );
    {
        const char *things[] = {
            N_("small"),
            N_("medium"),
            N_("large"),
            N_("huge")
        };
        Gtk::RadioMenuItem::Group groupOne;
        for ( unsigned int i = 0; i < G_N_ELEMENTS(things); i++ ) {
            Glib::ustring foo(gettext(things[i]));
            Gtk::RadioMenuItem* single = manage(new Gtk::RadioMenuItem(groupOne, foo));
            menu.append(*single);
            if ( i == panel_size ) {
                single->set_active(true);
            }
            single->signal_activate().connect( sigc::bind<int, int>( sigc::mem_fun(*this, &Panel::bounceCall), 0, i) );
       }
    }
    menu.append( *manage(new Gtk::SeparatorMenuItem()) );
    Gtk::RadioMenuItem::Group group;
    Glib::ustring oneLab(_("List"));
    Glib::ustring twoLab(_("Grid"));
    Gtk::RadioMenuItem *one = manage(new Gtk::RadioMenuItem(group, oneLab));
    Gtk::RadioMenuItem *two = manage(new Gtk::RadioMenuItem(group, twoLab));

    if (panel_mode == 0) {
        one->set_active(true);
    } else if (panel_mode == 1) {
        two->set_active(true);
    }

    menu.append( *one );
    menu.append( *two );
    menu.append( *manage(new Gtk::SeparatorMenuItem()) );
    one->signal_activate().connect( sigc::bind<int, int>( sigc::mem_fun(*this, &Panel::bounceCall), 1, 0) );
    two->signal_activate().connect( sigc::bind<int, int>( sigc::mem_fun(*this, &Panel::bounceCall), 1, 1) );

    closeButton.set_label("X");

    topBar.pack_start(tabTitle);

    topBar.pack_end(closeButton, false, false);
    topBar.pack_end(tabButton, false, false);

    pack_start( topBar, false, false );

    show_all_children();

    bounceCall (0, panel_size);
    bounceCall (1, panel_mode);
}

void Panel::setLabel(Glib::ustring const &label)
{
    this->label = label;
    tabTitle.set_label(this->label);
}

void Panel::setOrientation( Gtk::AnchorType how )
{
    if ( _anchor != how )
    {
        _anchor = how;
    }
}

void Panel::_regItem( Gtk::MenuItem* item, int group, int id )
{
    menu.append( *item );
    item->signal_activate().connect( sigc::bind<int, int>( sigc::mem_fun(*this, &Panel::bounceCall), group + 2, id) );
}

void Panel::restorePanelPrefs()
{
    guint panel_size = 0;
    if (_prefs_path) {
        panel_size = prefs_get_int_attribute_limited (_prefs_path, "panel_size", 1, 0, 10);
    }
    guint panel_mode = 0;
    if (_prefs_path) {
        panel_mode = prefs_get_int_attribute_limited (_prefs_path, "panel_mode", 1, 0, 10);
    }
    bounceCall (0, panel_size);
    bounceCall (1, panel_mode);
}

void Panel::bounceCall(int i, int j)
{
    menu.set_active(0);
    switch ( i ) {
    case 0:
        if (_prefs_path) prefs_set_int_attribute (_prefs_path, "panel_size", j);
        if ( _fillable ) {
            ViewType currType = _fillable->getPreviewType();
            switch ( j ) {
            case 0:
            {
                _fillable->setStyle(Gtk::ICON_SIZE_MENU, currType);
            }
            break;
            case 1:
            {
                _fillable->setStyle(Gtk::ICON_SIZE_SMALL_TOOLBAR, currType);
            }
            break;
            case 2:
            {
                _fillable->setStyle(Gtk::ICON_SIZE_BUTTON, currType);
            }
            break;
            case 3:
            {
                _fillable->setStyle(Gtk::ICON_SIZE_DIALOG, currType);
            }
            break;
            default:
                ;
            }
        }
        break;
    case 1:
        if (_prefs_path) prefs_set_int_attribute (_prefs_path, "panel_mode", j);
        if ( _fillable ) {
            Gtk::BuiltinIconSize currSize = _fillable->getPreviewSize();
            switch ( j ) {
            case 0:
            {
                _fillable->setStyle(currSize, VIEW_TYPE_LIST);
            }
            break;
            case 1:
            {
                _fillable->setStyle(currSize, VIEW_TYPE_GRID);
            }
            break;
            default:
                break;
            }
        }
        break;
    default:
        _handleAction( i - 2, j );
    }
}





Glib::ustring const &Panel::getLabel() const
{
    return label;
}

void Panel::_setTargetFillable( PreviewFillable *target )
{
    _fillable = target;
}

void Panel::_handleAction( int setId, int itemId )
{
// for subclasses to override
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

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
