/*
 * A widget for controlling object compositing (filter, opacity, etc.)
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2004--2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "desktop-handles.h"
#include "desktop-style.h"
#include "document.h"
#include "filter-chemistry.h"
#include "inkscape.h"
#include "inkscape-stock.h"
#include "selection.h"
#include "style.h"
#include "svg/css-ostringstream.h"
#include "verbs.h"
#include "xml/repr.h"
#include "widgets/icon.h"
#include "ui/widget/object-composite-settings.h"
#include "display/sp-canvas.h"

namespace Inkscape {
namespace UI {
namespace Widget {

void ObjectCompositeSettings::on_selection_changed(
  Inkscape::Application *inkscape,
  Inkscape::Selection *selection,
  ObjectCompositeSettings *w
) {
    w->selectionChanged(inkscape, selection);
}

void ObjectCompositeSettings::on_selection_modified(
  Inkscape::Application *inkscape,
  Inkscape::Selection *selection,
  guint /*flags*/,
  ObjectCompositeSettings *w
) {
    w->selectionChanged(inkscape, selection);
}

ObjectCompositeSettings::ObjectCompositeSettings()
: _fe_vbox(false, 0),
  _fe_alignment(1, 1, 1, 1),
  _opacity_vbox(false, 0),
  _opacity_label_box(false, 0),
  _opacity_label(_("Opacity, %"), 0.0, 1.0, true),
  _opacity_adjustment(100.0, 0.0, 100.0, 1.0, 1.0, 0.0),
  _opacity_hscale(_opacity_adjustment),
  _opacity_spin_button(_opacity_adjustment, 0.01, 1),
  _blocked(false)
{
    // Filter Effects
    pack_start(_fe_vbox, false, false, 2);
    _fe_alignment.set_padding(0, 0, 4, 0);
    _fe_alignment.add(_fe_cb);
    _fe_vbox.pack_start(_fe_alignment, false, false, 0);
    _fe_cb.signal_blend_blur_changed().connect(sigc::mem_fun(*this, &ObjectCompositeSettings::_blendBlurValueChanged));

    // Opacity
    pack_start(_opacity_vbox, false, false, 2);
    _opacity_label_box.pack_start(_opacity_label, false, false, 4);
    _opacity_vbox.pack_start(_opacity_label_box, false, false, 0);
    _opacity_vbox.pack_start(_opacity_hbox, false, false, 0);
    _opacity_hbox.pack_start(_opacity_hscale, true, true, 4);
    _opacity_hbox.pack_start(_opacity_spin_button, false, false, 0);
    _opacity_hscale.set_draw_value(false);
    _opacity_adjustment.signal_value_changed().connect(sigc::mem_fun(*this, &ObjectCompositeSettings::_opacityValueChanged));

    _sel_changed = g_signal_connect ( G_OBJECT (INKSCAPE), "change_selection", G_CALLBACK (on_selection_changed), this );
    _subsel_changed = g_signal_connect ( G_OBJECT (INKSCAPE), "change_subselection", G_CALLBACK (on_selection_changed), this );
    _sel_modified = g_signal_connect ( G_OBJECT (INKSCAPE), "modify_selection", G_CALLBACK (on_selection_modified), this );
    _desktop_activated = g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (on_selection_changed), this );

    selectionChanged(INKSCAPE, sp_desktop_selection(SP_ACTIVE_DESKTOP));

    show_all_children();
}

ObjectCompositeSettings::~ObjectCompositeSettings() {
    g_signal_handler_disconnect(G_OBJECT(INKSCAPE), _sel_changed);
    g_signal_handler_disconnect(G_OBJECT(INKSCAPE), _subsel_changed);
    g_signal_handler_disconnect(G_OBJECT(INKSCAPE), _sel_modified);
    g_signal_handler_disconnect(G_OBJECT(INKSCAPE), _desktop_activated);
}

void
ObjectCompositeSettings::_blendBlurValueChanged()
{
    if (_blocked)
        return;
    _blocked = true;

    //get desktop
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) {
        return;
    }

    // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed; here it results in crash 1580903
    sp_canvas_force_full_redraw_after_interruptions(sp_desktop_canvas(desktop), 0);

    //get current selection
    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    NR::Maybe<NR::Rect> bbox = selection->bounds();
    if (!bbox) {
        return;
    }
    //get list of selected items
    GSList const *items = selection->itemList();
    //get current document
    SPDocument *document = sp_desktop_document (desktop);

    double perimeter = bbox->extent(NR::X) + bbox->extent(NR::Y);

    const Glib::ustring blendmode = _fe_cb.get_blend_mode();
    double radius = _fe_cb.get_blur_value() * perimeter / 400;

    SPFilter *filter = 0;
    const bool remfilter = (blendmode == "normal" && radius == 0) || (blendmode == "filter" && !filter);

    if(blendmode != "filter" || filter) {
        //apply created filter to every selected item
        for (GSList const *i = items; i != NULL; i = i->next) {
            SPItem * item = SP_ITEM(i->data);
            SPStyle *style = SP_OBJECT_STYLE(item);
            g_assert(style != NULL);


            if(remfilter) {
                remove_filter (item, false);
            }
            else {
                if(blendmode != "filter")
                    filter = new_filter_simple_from_item(document, item, blendmode.c_str(), radius);
                sp_style_set_property_url (SP_OBJECT(item), "filter", SP_OBJECT(filter), false);
            }

            //request update
            SP_OBJECT(item)->requestDisplayUpdate(( SP_OBJECT_MODIFIED_FLAG |
                                                    SP_OBJECT_STYLE_MODIFIED_FLAG ));
        }
    }

    sp_document_maybe_done (sp_desktop_document (desktop), "fillstroke:blur", SP_VERB_DIALOG_FILL_STROKE,  _("Change blur"));

    // resume interruptibility
    sp_canvas_end_forced_full_redraws(sp_desktop_canvas(desktop));

    _blocked = false;
}

void
ObjectCompositeSettings::_opacityValueChanged()
{
    if (_blocked)
        return;
    _blocked = true;

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed; here it results in crash 1580903
    // UPDATE: crash fixed in GTK+ 2.10.7 (bug 374378), remove this as soon as it's reasonably common
    // (though this only fixes the crash, not the multiple change events)
    sp_canvas_force_full_redraw_after_interruptions(sp_desktop_canvas(desktop), 0);

    SPCSSAttr *css = sp_repr_css_attr_new ();

    Inkscape::CSSOStringStream os;
    os << CLAMP (_opacity_adjustment.get_value() / 100, 0.0, 1.0);
    sp_repr_css_set_property (css, "opacity", os.str().c_str());

    sp_desktop_set_style (desktop, css);

    sp_repr_css_attr_unref (css);

    sp_document_maybe_done (sp_desktop_document (desktop), "fillstroke:opacity", SP_VERB_DIALOG_FILL_STROKE,
                            _("Change opacity"));

    // resume interruptibility
    sp_canvas_end_forced_full_redraws(sp_desktop_canvas(desktop));

    _blocked = false;
}

void
ObjectCompositeSettings::selectionChanged(Inkscape::Application */*inkscape*/,
                                          Inkscape::Selection */*selection*/)
{
    if (_blocked)
        return;
    _blocked = true;

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    // create temporary style
    SPStyle *query = sp_style_new (sp_desktop_document(desktop));
    // query style from desktop into it. This returns a result flag and fills query with the style of subselection, if any, or selection
    int result = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_MASTEROPACITY);

    switch (result) {
        case QUERY_STYLE_NOTHING:
            _opacity_hbox.set_sensitive(false);
            // gtk_widget_set_sensitive (opa, FALSE);
            break;
        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_AVERAGED: // TODO: treat this slightly differently
        case QUERY_STYLE_MULTIPLE_SAME:
            _opacity_hbox.set_sensitive(true);
            _opacity_adjustment.set_value(100 * SP_SCALE24_TO_FLOAT(query->opacity.value));
            break;
    }

    //query now for current filter mode and average blurring of selection
    const int blend_result = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_BLEND);
    switch(blend_result) {
        case QUERY_STYLE_NOTHING:
            _fe_cb.set_sensitive(false);
            break;
        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_SAME:
            _fe_cb.set_blend_mode(query->filter_blend_mode.value);
            _fe_cb.set_sensitive(true);
            break;
        case QUERY_STYLE_MULTIPLE_DIFFERENT:
            // TODO: set text
            _fe_cb.set_sensitive(false);
            break;
    }

    if(blend_result == QUERY_STYLE_SINGLE || blend_result == QUERY_STYLE_MULTIPLE_SAME) {
        int blur_result = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_BLUR);
        switch (blur_result) {
            case QUERY_STYLE_NOTHING: //no blurring
                _fe_cb.set_blur_sensitive(false);
                break;
            case QUERY_STYLE_SINGLE:
            case QUERY_STYLE_MULTIPLE_AVERAGED:
            case QUERY_STYLE_MULTIPLE_SAME:
                NR::Maybe<NR::Rect> bbox = sp_desktop_selection(desktop)->bounds();
                if (bbox) {
                    double perimeter = bbox->extent(NR::X) + bbox->extent(NR::Y);
                    _fe_cb.set_blur_sensitive(true);
                    //update blur widget value
                    float radius = query->filter_gaussianBlur_deviation.value;
                    float percent = radius * 400 / perimeter; // so that for a square, 100% == half side
                    _fe_cb.set_blur_value(percent);
                }
                break;
        }
    }

    sp_style_unref(query);

    _blocked = false;
}

}
}
}

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
