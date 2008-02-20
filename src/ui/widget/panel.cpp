/**
 * \brief Panel widget
 *
 * Authors:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2004 Bryce Harrington
 * Copyright (C) 2005 Jon A. Cruz
 * Copyright (C) 2007 Gustav Broberg
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>

#include <gtkmm/dialog.h> // for Gtk::RESPONSE_*
#include <gtkmm/stock.h>

#include "panel.h"
#include "icon-size.h"
#include "prefs-utils.h"
#include "desktop-handles.h"
#include "inkscape.h"
#include "dialogs/eek-preview.h"

namespace Inkscape {
namespace UI {
namespace Widget {

static const int PANEL_SETTING_SIZE = 0;
static const int PANEL_SETTING_MODE = 1;
static const int PANEL_SETTING_WRAP = 2;
static const int PANEL_SETTING_NEXTFREE = 3;


void Panel::prep() {
    GtkIconSize sizes[] = {
        static_cast<GtkIconSize>(Inkscape::ICON_SIZE_DECORATION),
        GTK_ICON_SIZE_MENU,
        GTK_ICON_SIZE_SMALL_TOOLBAR,
        GTK_ICON_SIZE_BUTTON,
        GTK_ICON_SIZE_DND, // Not used by options, but included to make the last size larger
        GTK_ICON_SIZE_DIALOG
    };
    eek_preview_set_size_mappings( G_N_ELEMENTS(sizes), sizes );
}

/**
 *    Construct a Panel
 */

Panel::Panel(Glib::ustring const &label, gchar const *prefs_path,
             int verb_num, Glib::ustring const &apply_label,
             bool menu_desired) :
    _prefs_path(prefs_path),
    _menu_desired(menu_desired),
    _desktop(SP_ACTIVE_DESKTOP),
    _label(label),
    _apply_label(apply_label),
    _verb_num(verb_num),
    _temp_arrow(Gtk::ARROW_LEFT, Gtk::SHADOW_ETCHED_OUT),
    _menu(0),
    _action_area(0),
    _fillable(0)
{
    _init();
}

Panel::~Panel()
{
    delete _menu;
}

void Panel::_popper(GdkEventButton* event)
{
    if ( (event->type == GDK_BUTTON_PRESS) && (event->button == 3 || event->button == 1) ) {
        if (_menu) {
            _menu->popup(event->button, event->time);
        }
    }
}

void Panel::_init()
{
    Glib::ustring tmp("<");
    _anchor = Gtk::ANCHOR_CENTER;

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

    _menu = new Gtk::Menu();
    {
        const char *things[] = {
            N_("tiny"),
            N_("small"),
            Q_("swatches|medium"),
            N_("large"),
            N_("huge")
        };
        Gtk::RadioMenuItem::Group groupOne;
        for (unsigned int i = 0; i < G_N_ELEMENTS(things); i++) {
            Glib::ustring foo(gettext(things[i]));
            Gtk::RadioMenuItem* single = manage(new Gtk::RadioMenuItem(groupOne, foo));
            _menu->append(*single);
            if (i == panel_size) {
                single->set_active(true);
            }
            single->signal_activate().connect(sigc::bind<int, int>(sigc::mem_fun(*this, &Panel::_bounceCall), PANEL_SETTING_SIZE, i));
       }
    }
    _menu->append(*manage(new Gtk::SeparatorMenuItem()));
    Gtk::RadioMenuItem::Group group;
    Glib::ustring one_label(_("List"));
    Glib::ustring two_label(_("Grid"));
    Gtk::RadioMenuItem *one = manage(new Gtk::RadioMenuItem(group, one_label));
    Gtk::RadioMenuItem *two = manage(new Gtk::RadioMenuItem(group, two_label));

    if (panel_mode == 0) {
        one->set_active(true);
    } else if (panel_mode == 1) {
        two->set_active(true);
    }

    _menu->append(*one);
    _non_horizontal.push_back(one);
    _menu->append(*two);
    _non_horizontal.push_back(two);
    Gtk::MenuItem* sep = manage(new Gtk::SeparatorMenuItem());
    _menu->append(*sep);
    _non_horizontal.push_back(sep);
    one->signal_activate().connect(sigc::bind<int, int>(sigc::mem_fun(*this, &Panel::_bounceCall), PANEL_SETTING_MODE, 0));
    two->signal_activate().connect(sigc::bind<int, int>(sigc::mem_fun(*this, &Panel::_bounceCall), PANEL_SETTING_MODE, 1));

    {
        Glib::ustring wrap_label(_("Wrap"));
        Gtk::CheckMenuItem *check = manage(new Gtk::CheckMenuItem(wrap_label));
        check->set_active(panel_wrap);
        _menu->append(*check);
        _non_vertical.push_back(check);

        check->signal_toggled().connect(sigc::bind<Gtk::CheckMenuItem*>(sigc::mem_fun(*this, &Panel::_wrapToggled), check));
    }

    {
        Glib::ustring type_label(_("Shape"));

        Glib::ustring shape_1_label(_("Tall"));
        Glib::ustring shape_2_label(_("Square"));
        Glib::ustring shape_3_label(_("Wide"));

        Gtk::MenuItem *item = manage( new Gtk::MenuItem(type_label));
        Gtk::Menu *type_menu = manage(new Gtk::Menu());
        item->set_submenu(*type_menu);
        _menu->append(*item);

        Gtk::RadioMenuItem::Group shapeGroup;

        Gtk::RadioMenuItem *shape_1 = manage(new Gtk::RadioMenuItem(shapeGroup, shape_1_label));
        Gtk::RadioMenuItem *shape_2 = manage(new Gtk::RadioMenuItem(shapeGroup, shape_2_label));
        Gtk::RadioMenuItem *shape_3 = manage(new Gtk::RadioMenuItem(shapeGroup, shape_3_label));

        type_menu->append(*shape_1);
        type_menu->append(*shape_2);
        type_menu->append(*shape_3);

        shape_2->set_active(true);
    }

    sep = manage(new Gtk::SeparatorMenuItem());
    _menu->append(*sep);

    _menu->show_all_children();
    for ( std::vector<Gtk::Widget*>::iterator iter = _non_vertical.begin(); iter != _non_vertical.end(); ++iter ) {
        (*iter)->hide();
    }

    // _close_button.set_label("X");

    if (!_label.empty()) {
        _tab_title.set_label(_label);
        _top_bar.pack_start(_tab_title);
    }

    // _top_bar.pack_end(_close_button, false, false);


    if ( _menu_desired ) {
        _top_bar.pack_end(_menu_popper, false, false);
        Gtk::Frame* outliner = manage(new Gtk::Frame());
        outliner->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
        outliner->add(_temp_arrow);
        _menu_popper.add(*outliner);
        _menu_popper.signal_button_press_event().connect_notify(sigc::mem_fun(*this, &Panel::_popper));
    }

    pack_start(_top_bar, false, false);

    Gtk::HBox* boxy = manage(new Gtk::HBox());

    boxy->pack_start(_contents, true, true);
    boxy->pack_start(_right_bar, false, true);

    pack_start(*boxy, true, true);

    signalResponse().connect(sigc::mem_fun(*this, &Panel::_handleResponse));

    signalActivateDesktop().connect(sigc::hide<0>(sigc::mem_fun(*this, &Panel::setDesktop)));

    show_all_children();

    _bounceCall(PANEL_SETTING_SIZE, panel_size);
    _bounceCall(PANEL_SETTING_MODE, panel_mode);
    _bounceCall(PANEL_SETTING_WRAP, panel_wrap);
}

void Panel::setLabel(Glib::ustring const &label)
{
    if (_label.empty() && !label.empty())
        _top_bar.pack_start(_tab_title);
    else if (!_label.empty() && label.empty())
        _top_bar.remove(_tab_title);

    _label = label;
    _tab_title.set_label(_label);
}

void Panel::setOrientation(Gtk::AnchorType how)
{
    if (_anchor != how) {
        _anchor = how;
        switch (_anchor) {
            case Gtk::ANCHOR_NORTH:
            case Gtk::ANCHOR_SOUTH:
            {
                if (_menu_desired) {
                    _menu_popper.reference();
                    _top_bar.remove(_menu_popper);
                    _right_bar.pack_start(_menu_popper, false, false);
                    _menu_popper.unreference();

                    for (std::vector<Gtk::Widget*>::iterator iter = _non_horizontal.begin(); iter != _non_horizontal.end(); ++iter) {
                        (*iter)->hide();
                    }
                    for (std::vector<Gtk::Widget*>::iterator iter = _non_vertical.begin(); iter != _non_vertical.end(); ++iter) {
                        (*iter)->show();
                    }
                }
                // Ensure we are not in "list" mode
                _bounceCall(PANEL_SETTING_MODE, 1);
                if (!_label.empty())
                    _top_bar.remove(_tab_title);
            }
            break;

            default:
            {
                if ( _menu_desired ) {
                    for (std::vector<Gtk::Widget*>::iterator iter = _non_horizontal.begin(); iter != _non_horizontal.end(); ++iter) {
                        (*iter)->show();
                    }
                    for (std::vector<Gtk::Widget*>::iterator iter = _non_vertical.begin(); iter != _non_vertical.end(); ++iter) {
                        (*iter)->hide();
                    }
                }
            }
        }
    }
}

void Panel::present()
{
    _signal_present.emit();
}


void Panel::restorePanelPrefs()
{
    guint panel_size = 0;
    if (_prefs_path) {
        panel_size = prefs_get_int_attribute_limited(_prefs_path, "panel_size", 1, 0, 10);
    }
    guint panel_mode = 0;
    if (_prefs_path) {
        panel_mode = prefs_get_int_attribute_limited(_prefs_path, "panel_mode", 1, 0, 10);
    }
    guint panel_wrap = 0;
    if (_prefs_path) {
        panel_wrap = prefs_get_int_attribute_limited(_prefs_path, "panel_wrap", 0, 0, 1 );
    }
    _bounceCall(PANEL_SETTING_SIZE, panel_size);
    _bounceCall(PANEL_SETTING_MODE, panel_mode);
    _bounceCall(PANEL_SETTING_WRAP, panel_wrap);
}

sigc::signal<void, int> &
Panel::signalResponse()
{
    return _signal_response;
}

sigc::signal<void> &
Panel::signalPresent()
{
    return _signal_present;
}

void Panel::_bounceCall(int i, int j)
{
    _menu->set_active(0);
    switch (i) {
    case PANEL_SETTING_SIZE:
        if (_prefs_path) {
            prefs_set_int_attribute(_prefs_path, "panel_size", j);
        }
        if (_fillable) {
            ViewType curr_type = _fillable->getPreviewType();
            switch (j) {
            case 0:
            {
                _fillable->setStyle(Inkscape::ICON_SIZE_DECORATION, curr_type);
            }
            break;
            case 1:
            {
                _fillable->setStyle(Inkscape::ICON_SIZE_MENU, curr_type);
            }
            break;
            case 2:
            {
                _fillable->setStyle(Inkscape::ICON_SIZE_SMALL_TOOLBAR, curr_type);
            }
            break;
            case 3:
            {
                _fillable->setStyle(Inkscape::ICON_SIZE_BUTTON, curr_type);
            }
            break;
            case 4:
            {
                _fillable->setStyle(Inkscape::ICON_SIZE_DIALOG, curr_type);
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
        if (_fillable) {
            Inkscape::IconSize curr_size = _fillable->getPreviewSize();
            switch (j) {
            case 0:
            {
                _fillable->setStyle(curr_size, VIEW_TYPE_LIST);
            }
            break;
            case 1:
            {
                _fillable->setStyle(curr_size, VIEW_TYPE_GRID);
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
            _fillable->setWrap(j);
        }
        break;
    default:
        _handleAction(i - PANEL_SETTING_NEXTFREE, j);
    }
}


void Panel::_wrapToggled(Gtk::CheckMenuItem* toggler)
{
    if (toggler) {
        _bounceCall(PANEL_SETTING_WRAP, toggler->get_active() ? 1 : 0);
    }
}

gchar const *Panel::getPrefsPath() const
{
    return _prefs_path;
}

Glib::ustring const &Panel::getLabel() const
{
    return _label;
}

int const &Panel::getVerb() const
{
    return _verb_num;
}

Glib::ustring const &Panel::getApplyLabel() const
{
    return _apply_label;
}

void Panel::setDesktop(SPDesktop *desktop)
{
    _desktop = desktop;
}

void Panel::_setTargetFillable(PreviewFillable *target)
{
    _fillable = target;
}

void Panel::_regItem(Gtk::MenuItem* item, int group, int id)
{
    _menu->append(*item);
    item->signal_activate().connect(sigc::bind<int, int>(sigc::mem_fun(*this, &Panel::_bounceCall), group + PANEL_SETTING_NEXTFREE, id));
    item->show();
}

void Panel::_handleAction(int /*set_id*/, int /*item_id*/)
{
// for subclasses to override
}

void
Panel::_apply()
{
    g_warning("Apply button clicked for panel [Panel::_apply()]");
}

Gtk::Button *
Panel::addResponseButton(const Glib::ustring &button_text, int response_id)
{
    Gtk::Button *button = new Gtk::Button(button_text);
    _addResponseButton(button, response_id);
    return button;
}

Gtk::Button *
Panel::addResponseButton(const Gtk::StockID &stock_id, int response_id)
{
    Gtk::Button *button = new Gtk::Button(stock_id);
    _addResponseButton(button, response_id);
    return button;
}

void
Panel::_addResponseButton(Gtk::Button *button, int response_id)
{
    // Create a button box for the response buttons if it's the first button to be added
    if (!_action_area) {
        _action_area = new Gtk::HButtonBox(Gtk::BUTTONBOX_END, 6);
        _action_area->set_border_width(4);
        pack_end(*_action_area, Gtk::PACK_SHRINK, 0);
    }

    _action_area->pack_end(*button);

    if (response_id != 0) {
        // Re-emit clicked signals as response signals
        button->signal_clicked().connect(sigc::bind(_signal_response.make_slot(), response_id));
        _response_map[response_id] = button;
    }
}

void
Panel::setDefaultResponse(int response_id)
{
    ResponseMap::iterator widget_found;
    widget_found = _response_map.find(response_id);

    if (widget_found != _response_map.end()) {
        widget_found->second->activate();
        widget_found->second->property_can_default() = true;
        widget_found->second->grab_default();
    }
}

void
Panel::setResponseSensitive(int response_id, bool setting)
{
    if (_response_map[response_id])
        _response_map[response_id]->set_sensitive(setting);
}

sigc::signal<void, SPDesktop *, SPDocument *> &
Panel::signalDocumentReplaced()
{
    return _signal_document_replaced;
}

sigc::signal<void, Inkscape::Application *, SPDesktop *> &
Panel::signalActivateDesktop()
{
    return _signal_activate_desktop;
}

sigc::signal<void, Inkscape::Application *, SPDesktop *> &
Panel::signalDeactiveDesktop()
{
    return _signal_deactive_desktop;
}

void
Panel::_handleResponse(int response_id)
{
    switch (response_id) {
        case Gtk::RESPONSE_APPLY: {
            _apply();
            break;
        }
    }
}

Inkscape::Selection *Panel::_getSelection()
{
    return sp_desktop_selection(_desktop);
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
