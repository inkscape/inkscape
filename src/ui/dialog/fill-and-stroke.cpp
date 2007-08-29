/**
 * \brief Fill and Stroke dialog, 
 * based on sp_object_properties_dialog
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2004--2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "desktop-handles.h"
#include "desktop-style.h"
#include "document.h"
#include "fill-and-stroke.h"
#include "filter-chemistry.h"
#include "inkscape.h"
#include "inkscape-stock.h"
#include "selection.h"
#include "style.h"
#include "svg/css-ostringstream.h"
#include "verbs.h"
#include "xml/repr.h"
#include "widgets/icon.h"


#include "dialogs/fill-style.h"
#include "dialogs/stroke-style.h"

#include <widgets/paint-selector.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

void on_selection_changed(Inkscape::Application *inkscape, 
                          Inkscape::Selection *selection,
                          FillAndStroke *dlg)
{
    dlg->selectionChanged(inkscape, selection);
}

void on_selection_modified(Inkscape::Application *inkscape, 
                           Inkscape::Selection *selection, 
                           guint flags,
                           FillAndStroke *dlg)
{
    dlg->selectionChanged(inkscape, selection);
}


FillAndStroke::FillAndStroke(Behavior::BehaviorFactory behavior_factory) 
    : Dialog (behavior_factory, "dialogs.fillstroke", SP_VERB_DIALOG_FILL_STROKE),
      _page_fill(1, 1, true, true),
      _page_stroke_paint(1, 1, true, true),
      _page_stroke_style(1, 1, true, true),
      _fe_vbox(false, 0),
      _fe_alignment(1, 1, 1, 1),
      _opacity_vbox(false, 0),
      _opacity_label_box(false, 0),
      _opacity_label(_("Master _opacity, %"), 0.0, 1.0, true),
      _opacity_adjustment(100.0, 0.0, 100.0, 1.0, 1.0, 0.0),
      _opacity_hscale(_opacity_adjustment),
      _opacity_spin_button(_opacity_adjustment, 0.01, 1),
      _blocked(false)
{
    Gtk::VBox *vbox = get_vbox();
    vbox->set_spacing(0);

    vbox->pack_start(_notebook, true, true);

    _notebook.append_page(_page_fill, _createPageTabLabel(_("Fill"), INKSCAPE_STOCK_PROPERTIES_FILL_PAGE));
    _notebook.append_page(_page_stroke_paint, _createPageTabLabel(_("Stroke _paint"), INKSCAPE_STOCK_PROPERTIES_STROKE_PAINT_PAGE));
    _notebook.append_page(_page_stroke_style, _createPageTabLabel(_("Stroke st_yle"), INKSCAPE_STOCK_PROPERTIES_STROKE_PAGE));

    _layoutPageFill();
    _layoutPageStrokePaint();
    _layoutPageStrokeStyle();

    // Filter Effects
    vbox->pack_start(_fe_vbox, false, false, 2);
    _fe_alignment.set_padding(0, 0, 4, 0);
    _fe_alignment.add(_fe_cb);
    _fe_vbox.pack_start(_fe_alignment, false, false, 0);

    _fe_cb.signal_blend_blur_changed().connect(sigc::mem_fun(*this, &Inkscape::UI::Dialog::FillAndStroke::_blendBlurValueChanged));
    
    // Opacity
    vbox->pack_start(_opacity_vbox, false, false, 2);
    _opacity_label_box.pack_start(_opacity_label, false, false, 4);
    _opacity_vbox.pack_start(_opacity_label_box, false, false, 0);
    _opacity_vbox.pack_start(_opacity_hbox, false, false, 0);
    _opacity_hbox.pack_start(_opacity_hscale, true, true, 4);
    _opacity_hbox.pack_start(_opacity_spin_button, false, false, 0);
    _opacity_hscale.set_draw_value(false);
    _opacity_adjustment.signal_value_changed().connect(sigc::mem_fun(*this, &Inkscape::UI::Dialog::FillAndStroke::_opacityValueChanged));

    // these callbacks are only for the blur and master opacity update; the tabs above take care of themselves
    g_signal_connect ( G_OBJECT (INKSCAPE), "change_selection", G_CALLBACK (on_selection_changed), this );
    g_signal_connect ( G_OBJECT (INKSCAPE), "change_subselection", G_CALLBACK (on_selection_changed), this );
    g_signal_connect ( G_OBJECT (INKSCAPE), "modify_selection", G_CALLBACK (on_selection_modified), this );
    g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (on_selection_changed), this );

    selectionChanged(INKSCAPE, sp_desktop_selection(SP_ACTIVE_DESKTOP));

    show_all_children();
}

FillAndStroke::~FillAndStroke() 
{
}

void
FillAndStroke::_layoutPageFill()
{
    Gtk::Widget *fs = manage(Glib::wrap(sp_fill_style_widget_new()));
    _page_fill.table().attach(*fs, 0, 1, 0, 1);
}

void
FillAndStroke::_layoutPageStrokePaint()
{
    Gtk::Widget *ssp = manage(Glib::wrap(sp_stroke_style_paint_widget_new()));
    _page_stroke_paint.table().attach(*ssp, 0, 1, 0, 1);
}

void
FillAndStroke::_layoutPageStrokeStyle()
{
    Gtk::Widget *ssl = manage(Glib::wrap(sp_stroke_style_line_widget_new()));
    _page_stroke_style.table().attach(*ssl, 0, 1, 0, 1);
}

void
FillAndStroke::_blendBlurValueChanged()
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

    sp_document_maybe_done (sp_desktop_document (SP_ACTIVE_DESKTOP), "fillstroke:blur", SP_VERB_DIALOG_FILL_STROKE,  _("Change blur"));

    // resume interruptibility
    sp_canvas_end_forced_full_redraws(sp_desktop_canvas(desktop));

    _blocked = false;
}

void
FillAndStroke::_opacityValueChanged()
{
    if (_blocked)
        return;
    _blocked = true;

    // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed; here it results in crash 1580903
    // UPDATE: crash fixed in GTK+ 2.10.7 (bug 374378), remove this as soon as it's reasonably common
    // (though this only fixes the crash, not the multiple change events)
    sp_canvas_force_full_redraw_after_interruptions(sp_desktop_canvas(SP_ACTIVE_DESKTOP), 0);

    SPCSSAttr *css = sp_repr_css_attr_new ();

    Inkscape::CSSOStringStream os;
    os << CLAMP (_opacity_adjustment.get_value() / 100, 0.0, 1.0);
    sp_repr_css_set_property (css, "opacity", os.str().c_str());

    sp_desktop_set_style (SP_ACTIVE_DESKTOP, css);

    sp_repr_css_attr_unref (css);

    sp_document_maybe_done (sp_desktop_document (SP_ACTIVE_DESKTOP), "fillstroke:opacity", SP_VERB_DIALOG_FILL_STROKE, 
                            _("Change opacity"));

    // resume interruptibility
    sp_canvas_end_forced_full_redraws(sp_desktop_canvas(SP_ACTIVE_DESKTOP));

    _blocked = false;
}

void
FillAndStroke::selectionChanged(Inkscape::Application *inkscape,
                                Inkscape::Selection *selection)
{
    if (_blocked)
        return;
    _blocked = true;

    // create temporary style
    SPStyle *query = sp_style_new (SP_ACTIVE_DOCUMENT);
    // query style from desktop into it. This returns a result flag and fills query with the style of subselection, if any, or selection
    int result = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_MASTEROPACITY);

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
    const int blend_result = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_BLEND);
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
        int blur_result = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_BLUR);
        switch (blur_result) {
            case QUERY_STYLE_NOTHING: //no blurring
                _fe_cb.set_blur_sensitive(false);
                break;
            case QUERY_STYLE_SINGLE:
            case QUERY_STYLE_MULTIPLE_AVERAGED:
            case QUERY_STYLE_MULTIPLE_SAME: 
                NR::Maybe<NR::Rect> bbox = sp_desktop_selection(SP_ACTIVE_DESKTOP)->bounds();
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

Gtk::HBox&
FillAndStroke::_createPageTabLabel(const Glib::ustring& label, const char *label_image)
{
    Gtk::HBox *_tab_label_box = manage(new Gtk::HBox(false, 0));
    _tab_label_box->pack_start(*Glib::wrap(sp_icon_new(Inkscape::ICON_SIZE_DECORATION,
                                                       label_image)));

    Gtk::Label *_tab_label = manage(new Gtk::Label(label, true));
    _tab_label_box->pack_start(*_tab_label);
    _tab_label_box->show_all();

    return *_tab_label_box;
}

} // namespace Dialog
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
