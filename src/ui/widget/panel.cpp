/*
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

#include <gtkmm/dialog.h> // for Gtk::RESPONSE_*
#include <gtkmm/menu.h>
#include <gtkmm/stock.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/radiomenuitem.h>
#include <gtkmm/separatormenuitem.h>

#include <glibmm/i18n.h>

#include <gtk/gtk.h>

#include "panel.h"
#include "icon-size.h"
#include "preferences.h"
#include "desktop.h"

#include "inkscape.h"
#include "widgets/eek-preview.h"
#include "ui/previewfillable.h"

namespace Inkscape {
namespace UI {
namespace Widget {

static const int PANEL_SETTING_SIZE = 0;
static const int PANEL_SETTING_MODE = 1;
static const int PANEL_SETTING_SHAPE = 2;
static const int PANEL_SETTING_WRAP = 3;
static const int PANEL_SETTING_BORDER = 4;
static const int PANEL_SETTING_NEXTFREE = 5;


void Panel::prep() {
    GtkIconSize sizes[] = {
        Inkscape::getRegisteredIconSize(Inkscape::ICON_SIZE_DECORATION),
        GTK_ICON_SIZE_MENU,
        GTK_ICON_SIZE_SMALL_TOOLBAR,
        GTK_ICON_SIZE_BUTTON,
        GTK_ICON_SIZE_DND, // Not used by options, but included to make the last size larger
        GTK_ICON_SIZE_DIALOG
    };
    eek_preview_set_size_mappings( G_N_ELEMENTS(sizes), sizes );
}

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
    set_name( "InkscapePanel" );
#if WITH_GTKMM_3_0
    set_orientation( Gtk::ORIENTATION_VERTICAL );
#endif
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
    _anchor = SP_ANCHOR_CENTER;

    guint panel_size = 0, panel_mode = 0, panel_ratio = 100, panel_border = 0;
    bool panel_wrap = 0;
    if (!_prefs_path.empty()) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        panel_wrap = prefs->getBool(_prefs_path + "/panel_wrap");
        panel_size = prefs->getIntLimited(_prefs_path + "/panel_size", 1, 0, PREVIEW_SIZE_HUGE);
        panel_mode = prefs->getIntLimited(_prefs_path + "/panel_mode", 1, 0, 10);
        panel_ratio = prefs->getIntLimited(_prefs_path + "/panel_ratio", 100, 0, 500 );
        panel_border = prefs->getIntLimited(_prefs_path + "/panel_border", BORDER_NONE, 0, 2 );
    }

    _menu = new Gtk::Menu();

    {
        Gtk::RadioMenuItem::Group group;
        Glib::ustring one_label(_("List"));
        Glib::ustring two_label(_("Grid"));
        Gtk::RadioMenuItem *one = Gtk::manage(new Gtk::RadioMenuItem(group, one_label));
        Gtk::RadioMenuItem *two = Gtk::manage(new Gtk::RadioMenuItem(group, two_label));

        if (panel_mode == 0) {
            one->set_active(true);
        } else if (panel_mode == 1) {
            two->set_active(true);
        }

        _menu->append(*one);
        _non_horizontal.push_back(one);
        _menu->append(*two);
        _non_horizontal.push_back(two);
        Gtk::MenuItem* sep = Gtk::manage(new Gtk::SeparatorMenuItem());
        _menu->append(*sep);
        _non_horizontal.push_back(sep);
        one->signal_activate().connect(sigc::bind<int, int>(sigc::mem_fun(*this, &Panel::_bounceCall), PANEL_SETTING_MODE, 0));
        two->signal_activate().connect(sigc::bind<int, int>(sigc::mem_fun(*this, &Panel::_bounceCall), PANEL_SETTING_MODE, 1));
    }

    {
        Glib::ustring heightItemLabel(C_("Swatches", "Size"));

        //TRANSLATORS: Indicates size of colour swatches
        const gchar *heightLabels[] = {
            NC_("Swatches height", "Tiny"),
            NC_("Swatches height", "Small"),
            NC_("Swatches height", "Medium"),
            NC_("Swatches height", "Large"),
            NC_("Swatches height", "Huge")
        };

        Gtk::MenuItem *sizeItem = Gtk::manage(new Gtk::MenuItem(heightItemLabel));
        Gtk::Menu *sizeMenu = Gtk::manage(new Gtk::Menu());
        sizeItem->set_submenu(*sizeMenu);

        Gtk::RadioMenuItem::Group heightGroup;
        for (unsigned int i = 0; i < G_N_ELEMENTS(heightLabels); i++) {
            Glib::ustring _label(g_dpgettext2(NULL, "Swatches height", heightLabels[i]));
            Gtk::RadioMenuItem* _item = Gtk::manage(new Gtk::RadioMenuItem(heightGroup, _label));
            sizeMenu->append(*_item);
            if (i == panel_size) {
                _item->set_active(true);
            }
            _item->signal_activate().connect(sigc::bind<int, int>(sigc::mem_fun(*this, &Panel::_bounceCall), PANEL_SETTING_SIZE, i));
       }

       _menu->append(*sizeItem);
    }

    {
        Glib::ustring widthItemLabel(C_("Swatches", "Width"));

        //TRANSLATORS: Indicates width of colour swatches
        const gchar *widthLabels[] = {
            NC_("Swatches width", "Narrower"),
            NC_("Swatches width", "Narrow"),
            NC_("Swatches width", "Medium"),
            NC_("Swatches width", "Wide"),
            NC_("Swatches width", "Wider")
        };

        Gtk::MenuItem *item = Gtk::manage( new Gtk::MenuItem(widthItemLabel));
        Gtk::Menu *type_menu = Gtk::manage(new Gtk::Menu());
        item->set_submenu(*type_menu);
        _menu->append(*item);

        Gtk::RadioMenuItem::Group widthGroup;

        guint values[] = {0, 25, 50, 100, 200, 400};
        guint hot_index = 3;
        for ( guint i = 0; i < G_N_ELEMENTS(widthLabels); ++i ) {
            // Assume all values are in increasing order
            if ( values[i] <= panel_ratio ) {
                hot_index = i;
            }
        }
        for ( guint i = 0; i < G_N_ELEMENTS(widthLabels); ++i ) {
            Glib::ustring _label(g_dpgettext2(NULL, "Swatches width", widthLabels[i]));
            Gtk::RadioMenuItem *_item = Gtk::manage(new Gtk::RadioMenuItem(widthGroup, _label));
            type_menu->append(*_item);
            if ( i <= hot_index ) {
                _item->set_active(true);
            }
            _item->signal_activate().connect(sigc::bind<int, int>(sigc::mem_fun(*this, &Panel::_bounceCall), PANEL_SETTING_SHAPE, values[i]));
        }
    }

    {
        Glib::ustring widthItemLabel(C_("Swatches", "Border"));

        //TRANSLATORS: Indicates border of colour swatches
        const gchar *widthLabels[] = {
            NC_("Swatches border", "None"),
            NC_("Swatches border", "Solid"),
            NC_("Swatches border", "Wide"),
        };

        Gtk::MenuItem *item = Gtk::manage( new Gtk::MenuItem(widthItemLabel));
        Gtk::Menu *type_menu = Gtk::manage(new Gtk::Menu());
        item->set_submenu(*type_menu);
        _menu->append(*item);

        Gtk::RadioMenuItem::Group widthGroup;

        guint values[] = {0, 1, 2};
        guint hot_index = 0;
        for ( guint i = 0; i < G_N_ELEMENTS(widthLabels); ++i ) {
            // Assume all values are in increasing order
            if ( values[i] <= panel_border ) {
                hot_index = i;
            }
        }
        for ( guint i = 0; i < G_N_ELEMENTS(widthLabels); ++i ) {
            Glib::ustring _label(g_dpgettext2(NULL, "Swatches border", widthLabels[i]));
            Gtk::RadioMenuItem *_item = Gtk::manage(new Gtk::RadioMenuItem(widthGroup, _label));
            type_menu->append(*_item);
            if ( i <= hot_index ) {
                _item->set_active(true);
            }
            _item->signal_activate().connect(sigc::bind<int, int>(sigc::mem_fun(*this, &Panel::_bounceCall), PANEL_SETTING_BORDER, values[i]));
        }
    }

    {
        //TRANSLATORS: "Wrap" indicates how colour swatches are displayed
        Glib::ustring wrap_label(C_("Swatches","Wrap"));
        Gtk::CheckMenuItem *check = Gtk::manage(new Gtk::CheckMenuItem(wrap_label));
        check->set_active(panel_wrap);
        _menu->append(*check);
        _non_vertical.push_back(check);

        check->signal_toggled().connect(sigc::bind<Gtk::CheckMenuItem*>(sigc::mem_fun(*this, &Panel::_wrapToggled), check));
    }

    Gtk::SeparatorMenuItem *sep;
    sep = Gtk::manage(new Gtk::SeparatorMenuItem());
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
        gint width = 0;
        gint height = 0;

        if ( gtk_icon_size_lookup( Inkscape::getRegisteredIconSize(Inkscape::ICON_SIZE_DECORATION), &width, &height ) ) {
            _temp_arrow.set_size_request(width, height);
        }

        _menu_popper.add(_temp_arrow);
        _menu_popper.signal_button_press_event().connect_notify(sigc::mem_fun(*this, &Panel::_popper));
    }

    pack_start(_top_bar, false, false);

    Gtk::HBox* boxy = Gtk::manage(new Gtk::HBox());
    boxy->set_name( "PanelBoxY" );
    _contents.set_name( "PanelContents" );
    _right_bar.set_name( "PanelRightBar" );
    _top_bar.set_name( "PanelTopBar" );
    boxy->pack_start(_contents, true, true);
    boxy->pack_start(_right_bar, false, true);

    pack_start(*boxy, true, true);

    signalResponse().connect(sigc::mem_fun(*this, &Panel::_handleResponse));

    signalActivateDesktop().connect(sigc::mem_fun(*this, &Panel::setDesktop));

    show_all_children();

    _bounceCall(PANEL_SETTING_SIZE, panel_size);
    _bounceCall(PANEL_SETTING_MODE, panel_mode);
    _bounceCall(PANEL_SETTING_SHAPE, panel_ratio);
    _bounceCall(PANEL_SETTING_WRAP, panel_wrap);
    _bounceCall(PANEL_SETTING_BORDER, panel_border);
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

void Panel::setOrientation(SPAnchorType how)
{
    if (_anchor != how) {
        _anchor = how;
        switch (_anchor) {
            case SP_ANCHOR_NORTH:
            case SP_ANCHOR_SOUTH:
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
    guint panel_size = 0, panel_mode = 0, panel_ratio = 100, panel_border = 0;
    bool panel_wrap = 0;
    if (!_prefs_path.empty()) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        panel_wrap = prefs->getBool(_prefs_path + "/panel_wrap");
        panel_size = prefs->getIntLimited(_prefs_path + "/panel_size", 1, 0, PREVIEW_SIZE_HUGE);
        panel_mode = prefs->getIntLimited(_prefs_path + "/panel_mode", 1, 0, 10);
        panel_ratio = prefs->getIntLimited(_prefs_path + "/panel_ratio", 000, 0, 500 );
        panel_border = prefs->getIntLimited(_prefs_path + "/panel_border", BORDER_NONE, 0, 2 );
    }
    _bounceCall(PANEL_SETTING_SIZE, panel_size);
    _bounceCall(PANEL_SETTING_MODE, panel_mode);
    _bounceCall(PANEL_SETTING_SHAPE, panel_ratio);
    _bounceCall(PANEL_SETTING_WRAP, panel_wrap);
    _bounceCall(PANEL_SETTING_BORDER, panel_border);
}

sigc::signal<void, int> &Panel::signalResponse()
{
    return _signal_response;
}

sigc::signal<void> &Panel::signalPresent()
{
    return _signal_present;
}

void Panel::_bounceCall(int i, int j)
{
    _menu->set_active(0);
    switch (i) {
    case PANEL_SETTING_SIZE:
        if (!_prefs_path.empty()) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            prefs->setInt(_prefs_path + "/panel_size", j);
        }
        if (_fillable) {
            ViewType curr_type = _fillable->getPreviewType();
            guint curr_ratio = _fillable->getPreviewRatio();
            ::BorderStyle curr_border = _fillable->getPreviewBorder();

            switch (j) {
            case 0:
            {
                _fillable->setStyle(::PREVIEW_SIZE_TINY, curr_type, curr_ratio, curr_border);
            }
            break;
            case 1:
            {
                _fillable->setStyle(::PREVIEW_SIZE_SMALL, curr_type, curr_ratio, curr_border);
            }
            break;
            case 2:
            {
                _fillable->setStyle(::PREVIEW_SIZE_MEDIUM, curr_type, curr_ratio, curr_border);
            }
            break;
            case 3:
            {
                _fillable->setStyle(::PREVIEW_SIZE_BIG, curr_type, curr_ratio, curr_border);
            }
            break;
            case 4:
            {
                _fillable->setStyle(::PREVIEW_SIZE_HUGE, curr_type, curr_ratio, curr_border);
            }
            break;
            default:
                ;
            }
        }
        break;
    case PANEL_SETTING_MODE:
        if (!_prefs_path.empty()) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            prefs->setInt(_prefs_path + "/panel_mode", j);
        }
        if (_fillable) {
            ::PreviewSize curr_size = _fillable->getPreviewSize();
            guint curr_ratio = _fillable->getPreviewRatio();
            ::BorderStyle curr_border = _fillable->getPreviewBorder();
            switch (j) {
            case 0:
            {
                _fillable->setStyle(curr_size, VIEW_TYPE_LIST, curr_ratio, curr_border);
            }
            break;
            case 1:
            {
                _fillable->setStyle(curr_size, VIEW_TYPE_GRID, curr_ratio, curr_border);
            }
            break;
            default:
                break;
            }
        }
        break;
    case PANEL_SETTING_SHAPE:
        if (!_prefs_path.empty()) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            prefs->setInt(_prefs_path + "/panel_ratio", j);
        }
        if ( _fillable ) {
            ViewType curr_type = _fillable->getPreviewType();
            ::PreviewSize curr_size = _fillable->getPreviewSize();
            ::BorderStyle curr_border = _fillable->getPreviewBorder();

            _fillable->setStyle(curr_size, curr_type, j, curr_border);
        }
        break;
    case PANEL_SETTING_BORDER:
        if (!_prefs_path.empty()) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            prefs->setInt(_prefs_path + "/panel_border", j);
        }
        if ( _fillable ) {
            ::PreviewSize curr_size = _fillable->getPreviewSize();
            ViewType curr_type = _fillable->getPreviewType();
            guint curr_ratio = _fillable->getPreviewRatio();

            switch (j) {
            case 0:
            {
                _fillable->setStyle(curr_size, curr_type, curr_ratio, BORDER_NONE);
            }
            break;
            case 1:
            {
                _fillable->setStyle(curr_size, curr_type, curr_ratio, BORDER_SOLID);
            }
            break;
            case 2:
            {
                _fillable->setStyle(curr_size, curr_type, curr_ratio, BORDER_WIDE);
            }
            break;
            default:
                break;
            }
        }
        break;
    case PANEL_SETTING_WRAP:
        if (!_prefs_path.empty()) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            prefs->setBool(_prefs_path + "/panel_wrap", j);
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
    return _prefs_path.data();
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

void Panel::_apply()
{
    g_warning("Apply button clicked for panel [Panel::_apply()]");
}

Gtk::Button *Panel::addResponseButton(const Glib::ustring &button_text, int response_id, bool pack_start)
{
    Gtk::Button *button = new Gtk::Button(button_text);
    _addResponseButton(button, response_id, pack_start);
    return button;
}

Gtk::Button *Panel::addResponseButton(const Gtk::StockID &stock_id, int response_id, bool pack_start)
{
    Gtk::Button *button = new Gtk::Button(stock_id);
    _addResponseButton(button, response_id, pack_start);
    return button;
}

void Panel::_addResponseButton(Gtk::Button *button, int response_id, bool pack_start)
{
    // Create a button box for the response buttons if it's the first button to be added
    if (!_action_area) {
#if WITH_GTKMM_3_0
        _action_area = new Gtk::ButtonBox();
        _action_area->set_layout(Gtk::BUTTONBOX_END);
        _action_area->set_spacing(6);
#else
        _action_area = new Gtk::HButtonBox(Gtk::BUTTONBOX_END, 6);
#endif
        _action_area->set_border_width(4);
        pack_end(*_action_area, Gtk::PACK_SHRINK, 0);
    }

    _action_area->pack_end(*button);

    if (pack_start) {
        _action_area->set_child_secondary( *button , true);
    }

    if (response_id != 0) {
        // Re-emit clicked signals as response signals
        button->signal_clicked().connect(sigc::bind(_signal_response.make_slot(), response_id));
        _response_map[response_id] = button;
    }
}

void Panel::setDefaultResponse(int response_id)
{
    ResponseMap::iterator widget_found;
    widget_found = _response_map.find(response_id);

    if (widget_found != _response_map.end()) {
        widget_found->second->activate();
        widget_found->second->property_can_default() = true;
        widget_found->second->grab_default();
    }
}

void Panel::setResponseSensitive(int response_id, bool setting)
{
    if (_response_map[response_id])
        _response_map[response_id]->set_sensitive(setting);
}

sigc::signal<void, SPDesktop *, SPDocument *> &
Panel::signalDocumentReplaced()
{
    return _signal_document_replaced;
}

sigc::signal<void, SPDesktop *> &
Panel::signalActivateDesktop()
{
    return _signal_activate_desktop;
}

sigc::signal<void, SPDesktop *> &
Panel::signalDeactiveDesktop()
{
    return _signal_deactive_desktop;
}

void Panel::_handleResponse(int response_id)
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
    return _desktop->getSelection();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
