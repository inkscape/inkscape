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
#include "../../icon-size.h"
#include "../../prefs-utils.h"

namespace Inkscape {
namespace UI {
namespace Widget {

static const int PANEL_SETTING_SIZE = 0;
static const int PANEL_SETTING_MODE = 1;
static const int PANEL_SETTING_WRAP = 2;
static const int PANEL_SETTING_NEXTFREE = 3;

/**
 *    Construct a Panel
 *
 *    \param label Label.
 */

Panel::Panel() :
    _prefs_path(NULL),
    _menuDesired(false),
    _tempArrow( Gtk::ARROW_LEFT, Gtk::SHADOW_ETCHED_OUT ),
    menu(0),
    _fillable(0)
{
    init();
}

Panel::Panel( Glib::ustring const &label, gchar const* prefs_path, bool menuDesired ) :
    _prefs_path(prefs_path),
    _menuDesired(menuDesired),
    label(label),
    _tempArrow( Gtk::ARROW_LEFT, Gtk::SHADOW_ETCHED_OUT ),
    menu(0),
    _fillable(0)
{
    init();
}

Panel::~Panel()
{
    delete menu;
}

void Panel::_popper(GdkEventButton* event)
{
    if ( (event->type == GDK_BUTTON_PRESS) && (event->button == 3 || event->button == 1) ) {
        if (menu) {
            menu->popup( event->button, event->time );
        }
    }
}

void Panel::init()
{
    Glib::ustring tmp("<");
    tabTitle.set_label(this->label);

    guint panel_size = 0;
    if (_prefs_path) {
        panel_size = prefs_get_int_attribute_limited( _prefs_path, "panel_size", 1, 0, 10 );
    }

    guint panel_mode = 0;
    if (_prefs_path) {
        panel_mode = prefs_get_int_attribute_limited( _prefs_path, "panel_mode", 1, 0, 10 );
    }

    guint panel_wrap = 0;
    if (_prefs_path) {
        panel_wrap = prefs_get_int_attribute_limited( _prefs_path, "panel_wrap", 0, 0, 1 );
    }

    menu = new Gtk::Menu();
    {
        const char *things[] = {
            N_("tiny"),
            N_("small"),
            N_("medium"),
            N_("large"),
            N_("huge")
        };
        Gtk::RadioMenuItem::Group groupOne;
        for ( unsigned int i = 0; i < G_N_ELEMENTS(things); i++ ) {
            Glib::ustring foo(gettext(things[i]));
            Gtk::RadioMenuItem* single = manage(new Gtk::RadioMenuItem(groupOne, foo));
            menu->append(*single);
            if ( i == panel_size ) {
                single->set_active(true);
            }
            single->signal_activate().connect( sigc::bind<int, int>( sigc::mem_fun(*this, &Panel::bounceCall), PANEL_SETTING_SIZE, i) );
       }
    }
    menu->append( *manage(new Gtk::SeparatorMenuItem()) );
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

    menu->append( *one );
    nonHorizontal.push_back( one );
    menu->append( *two );
    nonHorizontal.push_back( two );
    Gtk::MenuItem* sep = manage( new Gtk::SeparatorMenuItem());
    menu->append( *sep );
    nonHorizontal.push_back( sep );
    one->signal_activate().connect( sigc::bind<int, int>( sigc::mem_fun(*this, &Panel::bounceCall), PANEL_SETTING_MODE, 0) );
    two->signal_activate().connect( sigc::bind<int, int>( sigc::mem_fun(*this, &Panel::bounceCall), PANEL_SETTING_MODE, 1) );

    {
        Glib::ustring wrapLab(_("Wrap"));
        Gtk::CheckMenuItem *check = manage(new Gtk::CheckMenuItem(wrapLab));
        check->set_active( panel_wrap );
        menu->append( *check );
        nonVertical.push_back(check);

        check->signal_toggled().connect( sigc::bind<Gtk::CheckMenuItem*>(sigc::mem_fun(*this, &Panel::_wrapToggled), check) );

        sep = manage( new Gtk::SeparatorMenuItem());
        menu->append( *sep );
        nonVertical.push_back( sep );
    }

    menu->show_all_children();
    for ( std::vector<Gtk::Widget*>::iterator iter = nonVertical.begin(); iter != nonVertical.end(); ++iter ) {
        (*iter)->hide();
    }

    //closeButton.set_label("X");

    topBar.pack_start(tabTitle);

    //topBar.pack_end(closeButton, false, false);


    if ( _menuDesired ) {
        topBar.pack_end(menuPopper, false, false);
        Gtk::Frame* outliner = manage(new Gtk::Frame());
        outliner->set_shadow_type( Gtk::SHADOW_ETCHED_IN );
        outliner->add( _tempArrow );
        menuPopper.add( *outliner );
        menuPopper.signal_button_press_event().connect_notify( sigc::mem_fun(*this, &Panel::_popper) );
    }

    pack_start( topBar, false, false );

    Gtk::HBox* boxy = manage( new Gtk::HBox() );

    boxy->pack_start( contents, true, true );
    boxy->pack_start( rightBar, false, true );

    pack_start( *boxy, true, true );

    show_all_children();

    bounceCall( PANEL_SETTING_SIZE, panel_size );
    bounceCall( PANEL_SETTING_MODE, panel_mode );
    bounceCall( PANEL_SETTING_WRAP, panel_wrap );
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
        switch ( _anchor )
        {
            case Gtk::ANCHOR_NORTH:
            case Gtk::ANCHOR_SOUTH:
            {
                if ( _menuDesired ) {
                    menuPopper.reference();
                    topBar.remove(menuPopper);
                    rightBar.pack_start(menuPopper, false, false);
                    menuPopper.unreference();

                    for ( std::vector<Gtk::Widget*>::iterator iter = nonHorizontal.begin(); iter != nonHorizontal.end(); ++iter ) {
                        (*iter)->hide();
                    }
                    for ( std::vector<Gtk::Widget*>::iterator iter = nonVertical.begin(); iter != nonVertical.end(); ++iter ) {
                        (*iter)->show();
                    }
                }
                // Ensure we are not in "list" mode
                bounceCall( PANEL_SETTING_MODE, 1 );
                topBar.remove(tabTitle);
            }
            break;

            default:
            {
                if ( _menuDesired ) {
                    for ( std::vector<Gtk::Widget*>::iterator iter = nonHorizontal.begin(); iter != nonHorizontal.end(); ++iter ) {
                        (*iter)->show();
                    }
                    for ( std::vector<Gtk::Widget*>::iterator iter = nonVertical.begin(); iter != nonVertical.end(); ++iter ) {
                        (*iter)->hide();
                    }
                }
            }
        }
    }
}

void Panel::_regItem( Gtk::MenuItem* item, int group, int id )
{
    menu->append( *item );
    item->signal_activate().connect( sigc::bind<int, int>( sigc::mem_fun(*this, &Panel::bounceCall), group + PANEL_SETTING_NEXTFREE, id) );
    item->show();
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
    guint panel_wrap = 0;
    if (_prefs_path) {
        panel_wrap = prefs_get_int_attribute_limited( _prefs_path, "panel_wrap", 0, 0, 1 );
    }
    bounceCall( PANEL_SETTING_SIZE, panel_size );
    bounceCall( PANEL_SETTING_MODE, panel_mode );
    bounceCall( PANEL_SETTING_WRAP, panel_wrap );
}

void Panel::bounceCall(int i, int j)
{
    menu->set_active(0);
    switch ( i ) {
    case PANEL_SETTING_SIZE:
        if (_prefs_path) {
            prefs_set_int_attribute( _prefs_path, "panel_size", j );
        }
        if ( _fillable ) {
            ViewType currType = _fillable->getPreviewType();
            switch ( j ) {
            case 0:
            {
                _fillable->setStyle(Inkscape::ICON_SIZE_DECORATION, currType);
            }
            break;
            case 1:
            {
                _fillable->setStyle(Inkscape::ICON_SIZE_MENU, currType);
            }
            break;
            case 2:
            {
                _fillable->setStyle(Inkscape::ICON_SIZE_SMALL_TOOLBAR, currType);
            }
            break;
            case 3:
            {
                _fillable->setStyle(Inkscape::ICON_SIZE_BUTTON, currType);
            }
            break;
            case 4:
            {
                _fillable->setStyle(Inkscape::ICON_SIZE_DIALOG, currType);
            }
            break;
            default:
                ;
            }
        }
        break;
    case PANEL_SETTING_MODE:
        if (_prefs_path) {
            prefs_set_int_attribute (_prefs_path, "panel_mode", j);
        }
        if ( _fillable ) {
            Inkscape::IconSize currSize = _fillable->getPreviewSize();
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
    case PANEL_SETTING_WRAP:
        if (_prefs_path) {
            prefs_set_int_attribute (_prefs_path, "panel_wrap", j ? 1 : 0);
        }
        if ( _fillable ) {
            _fillable->setWrap( j );
        }
        break;
    default:
        _handleAction( i - PANEL_SETTING_NEXTFREE, j );
    }
}


void Panel::_wrapToggled(Gtk::CheckMenuItem* toggler)
{
    if ( toggler ) {
        bounceCall( PANEL_SETTING_WRAP, toggler->get_active() ? 1 : 0 );
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
