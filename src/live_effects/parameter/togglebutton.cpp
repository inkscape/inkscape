/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Jabiertxo Arraiza Cenoz 2014 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/widget/registered-widget.h"
#include <glibmm/i18n.h>

#include "live_effects/parameter/togglebutton.h"
#include "live_effects/effect.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "widgets/icon.h"
#include "inkscape.h"
#include "verbs.h"
#include "helper-fns.h"

namespace Inkscape {

namespace LivePathEffect {

ToggleButtonParam::ToggleButtonParam( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect, bool default_value, const Glib::ustring& inactive_label,
                      char const * _icon_active, char const * _icon_inactive, 
                      Inkscape::IconSize _icon_size)
    : Parameter(label, tip, key, wr, effect), value(default_value), defvalue(default_value),
      inactive_label(inactive_label), _icon_active(_icon_active), _icon_inactive(_icon_inactive), _icon_size(_icon_size)
{
    checkwdg = NULL;
}

ToggleButtonParam::~ToggleButtonParam()
{
    if (_toggled_connection.connected()) {
        _toggled_connection.disconnect();
    }
}

void
ToggleButtonParam::param_set_default()
{
    param_setValue(defvalue);
}

bool
ToggleButtonParam::param_readSVGValue(const gchar * strvalue)
{
    param_setValue(helperfns_read_bool(strvalue, defvalue));
    return true; // not correct: if value is unacceptable, should return false!
}

gchar *
ToggleButtonParam::param_getSVGValue() const
{
    gchar * str = g_strdup(value ? "true" : "false");
    return str;
}

Gtk::Widget *
ToggleButtonParam::param_newWidget()
{
    if (_toggled_connection.connected()) {
        _toggled_connection.disconnect();
    }

   checkwdg = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredToggleButton( param_label,
                                                         param_tooltip,
                                                         param_key,
                                                         *param_wr,
                                                         false,
                                                         param_effect->getRepr(),
                                                         param_effect->getSPDoc()) );
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget * box_button = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(box_button), false);
#else
    GtkWidget * box_button = gtk_hbox_new (false, 0);
#endif
    GtkWidget * label_button = gtk_label_new ("");
    if (!param_label.empty()) {
        if(value || inactive_label.empty()){
            gtk_label_set_text(GTK_LABEL(label_button), param_label.c_str());
        }else{
            gtk_label_set_text(GTK_LABEL(label_button), inactive_label.c_str());
        }
    }
    gtk_widget_show(label_button);
    if ( _icon_active ) {
        if(!_icon_inactive){
            _icon_inactive = _icon_active;
        }
        gtk_widget_show(box_button);
        GtkWidget *icon_button = NULL;
        if(!value){ 
            icon_button = sp_icon_new(_icon_size, _icon_inactive);
        } else {
            icon_button = sp_icon_new(_icon_size, _icon_active);
        }
        gtk_widget_show(icon_button);
        gtk_box_pack_start (GTK_BOX(box_button), icon_button, false, false, 1);
        if (!param_label.empty()) {
            gtk_box_pack_start (GTK_BOX(box_button), label_button, false, false, 1);
        }
    }else{
        gtk_box_pack_start (GTK_BOX(box_button), label_button, false, false, 1);
    }
    checkwdg->add(*Gtk::manage(Glib::wrap(box_button)));
    checkwdg->setActive(value);
    checkwdg->setProgrammatically = false;
    checkwdg->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change togglebutton parameter"));

    _toggled_connection = checkwdg->signal_toggled().connect(sigc::mem_fun(*this, &ToggleButtonParam::toggled));

    return checkwdg;
}

void
ToggleButtonParam::refresh_button()
{
    if (!_toggled_connection.connected()) {
        return;
    }

    if(!checkwdg){
        return;
    }
    Gtk::Widget * box_button = checkwdg->get_child();
    if(!box_button){
        return;
    }
    GList * childs = gtk_container_get_children(GTK_CONTAINER(box_button->gobj()));
    guint total_widgets = g_list_length (childs);
    if (!param_label.empty()) {
        if(value || inactive_label.empty()){
            gtk_label_set_text(GTK_LABEL(g_list_nth_data(childs, total_widgets-1)), param_label.c_str());
        }else{
            gtk_label_set_text(GTK_LABEL(g_list_nth_data(childs, total_widgets-1)), inactive_label.c_str());
        }
    }
    if ( _icon_active ) {
        GdkPixbuf * icon_pixbuf = NULL;
        if(!value){ 
            icon_pixbuf = sp_pixbuf_new( _icon_size, _icon_inactive );
        } else {
            icon_pixbuf = sp_pixbuf_new( _icon_size, _icon_active );
        }
        gtk_image_set_from_pixbuf (GTK_IMAGE(g_list_nth_data(childs, 0)), icon_pixbuf);
    }
}

void
ToggleButtonParam::param_setValue(bool newvalue)
{
    value = newvalue;
    refresh_button();
}

void
ToggleButtonParam::toggled() {
    _signal_toggled.emit();
}

} /* namespace LivePathEffect */

} /* namespace Inkscape */

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
