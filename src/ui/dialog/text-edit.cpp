/**
 * @file
 * Text editing dialog.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   Abhishek Sharma
 *   John Smith
 *   Tavmjong Bah
 *
 * Copyright (C) 1999-2013 Authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "text-edit.h"
#include <libnrtype/font-instance.h>
#include <gtk/gtk.h>

#ifdef WITH_GTKSPELL
extern "C" {
# include <gtkspell/gtkspell.h>
}
#endif

#include <gtkmm/stock.h>
#include <libnrtype/font-instance.h>
#include <libnrtype/font-lister.h>
#include <xml/repr.h>

#include "macros.h"
#include "helper/window.h"
#include "inkscape.h"
#include "document.h"
#include "desktop.h"
#include "desktop-style.h"

#include "document-undo.h"
#include "selection.h"
#include "style.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "text-editing.h"
#include "ui/icon-names.h"
#include "preferences.h"
#include "verbs.h"
#include "ui/interface.h"
#include "svg/css-ostringstream.h"
#include "widgets/icon.h"
#include "widgets/font-selector.h"
#include <glibmm/i18n.h>
#include <glibmm/markup.h>
#include "util/units.h"
#include "sp-textpath.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

TextEdit::TextEdit()
    : UI::Widget::Panel("", "/dialogs/textandfont", SP_VERB_DIALOG_TEXT),
      font_label(_("_Font"), true),
      layout_frame(),
      text_label(_("_Text"), true),
      vari_label(_("_Variants"), true),
      setasdefault_button(_("Set as _default")),
      close_button(Gtk::Stock::CLOSE),
      apply_button(Gtk::Stock::APPLY),
      desktop(NULL),
      deskTrack(),
      selectChangedConn(),
      subselChangedConn(),
      selectModifiedConn(),
      /*
           TRANSLATORS: Test string used in text and font dialog (when no
           * text has been entered) to get a preview of the font.  Choose
           * some representative characters that users of your locale will be
           * interested in.*/
      blocked(false),
      samplephrase(_("AaBbCcIiPpQq12369$\342\202\254\302\242?.;/()"))
{

    /* Font selector */
    GtkWidget *fontsel = sp_font_selector_new ();
    gtk_widget_set_size_request (fontsel, 0, 150);
    fsel = SP_FONT_SELECTOR(fontsel);
    fontsel_hbox.pack_start(*Gtk::manage(Glib::wrap(fontsel)), true, true);

    /* Align buttons */
    styleButton(&align_left,    _("Align left"),                 INKSCAPE_ICON("format-justify-left"),    NULL);
    styleButton(&align_center,  _("Align center"),               INKSCAPE_ICON("format-justify-center"), &align_left);
    styleButton(&align_right,   _("Align right"),                INKSCAPE_ICON("format-justify-right"),  &align_left);
    styleButton(&align_justify, _("Justify (only flowed text)"), INKSCAPE_ICON("format-justify-fill"),   &align_left);

#if WITH_GTKMM_3_0
    align_sep.set_orientation(Gtk::ORIENTATION_VERTICAL);
#endif

    layout_hbox.pack_start(align_sep, false, false, 10);

    /* Direction buttons */
    styleButton(&text_horizontal, _("Horizontal text"), INKSCAPE_ICON("format-text-direction-horizontal"), NULL);
    styleButton(&text_vertical, _("Vertical text"), INKSCAPE_ICON("format-text-direction-vertical"), &text_horizontal);

#if WITH_GTKMM_3_0
    text_sep.set_orientation(Gtk::ORIENTATION_VERTICAL);
#endif

    layout_hbox.pack_start(text_sep, false, false, 10);

    /* Line Spacing */
    GtkWidget *px = sp_icon_new( Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_ICON("text_line_spacing") );
    layout_hbox.pack_start(*Gtk::manage(Glib::wrap(px)), false, false);

    spacing_combo = gtk_combo_box_text_new_with_entry ();
    gtk_widget_set_size_request (spacing_combo, 90, -1);

    const gchar *spacings[] = {"50%", "80%", "90%", "100%", "110%", "120%", "130%", "140%", "150%", "200%", "300%", NULL};
    for (int i = 0; spacings[i]; i++) {
        gtk_combo_box_text_append_text((GtkComboBoxText *) spacing_combo, spacings[i]);
    }

    gtk_widget_set_tooltip_text (px, _("Spacing between lines (percent of font size)"));
    gtk_widget_set_tooltip_text (spacing_combo, _("Spacing between lines (percent of font size)"));
    layout_hbox.pack_start(*Gtk::manage(Glib::wrap(spacing_combo)), false, false);
    layout_frame.set_padding(4,4,4,4);
    layout_frame.add(layout_hbox);

    // Text start Offset
    {
        startOffset = gtk_combo_box_text_new_with_entry ();
        gtk_widget_set_size_request(startOffset, 90, -1);

        const gchar *spacings[] = {"0%", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%", NULL};
        for (int i = 0; spacings[i]; i++) {
            gtk_combo_box_text_append_text(reinterpret_cast<GtkComboBoxText *>(startOffset), spacings[i]);
        }
        gtk_entry_set_text(reinterpret_cast<GtkEntry *>(gtk_bin_get_child(reinterpret_cast<GtkBin *>(startOffset))), "0%");

        gtk_widget_set_tooltip_text(startOffset, _("Text path offset"));

#if WITH_GTKMM_3_0
        Gtk::Separator *sep = Gtk::manage(new Gtk::Separator());
        sep->set_orientation(Gtk::ORIENTATION_VERTICAL);
#else
        Gtk::VSeparator *sep = Gtk::manage(new Gtk::VSeparator);
#endif
        layout_hbox.pack_start(*sep, false, false, 10);

        layout_hbox.pack_start(*Gtk::manage(Glib::wrap(startOffset)), false, false);
    }

    /* Font preview */
    preview_label.set_ellipsize(Pango::ELLIPSIZE_END);
    preview_label.set_justify(Gtk::JUSTIFY_CENTER);
    preview_label.set_line_wrap(FALSE);

    font_vbox.pack_start(fontsel_hbox, true, true);
    font_vbox.pack_start(layout_frame, false, false, VB_MARGIN);
    font_vbox.pack_start(preview_label, true, true, VB_MARGIN);

    /* Text tab */
    scroller.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
    scroller.set_shadow_type(Gtk::SHADOW_IN);

    text_buffer = gtk_text_buffer_new (NULL);
    text_view = gtk_text_view_new_with_buffer (text_buffer);
    gtk_text_view_set_wrap_mode ((GtkTextView *) text_view, GTK_WRAP_WORD);

#ifdef WITH_GTKSPELL
    GError *error = NULL;

/*
       TODO: Use computed xml:lang attribute of relevant element, if present, to specify the
       language (either as 2nd arg of gtkspell_new_attach, or with explicit
       gtkspell_set_language call in; see advanced.c example in gtkspell docs).
       onReadSelection looks like a suitable place.
*/
    if (gtkspell_new_attach(GTK_TEXT_VIEW(text_view), NULL, &error) == NULL) {
        g_print("gtkspell error: %s\n", error->message);
        g_error_free(error);
    }
#endif

    gtk_widget_set_size_request (text_view, -1, 64);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), TRUE);
    scroller.add(*Gtk::manage(Glib::wrap(text_view)));
    text_vbox.pack_start(scroller, true, true, 0);

    notebook.append_page(font_vbox, font_label);
    notebook.append_page(text_vbox, text_label);
    notebook.append_page(vari_vbox, vari_label);
    
    /* Buttons */
    setasdefault_button.set_use_underline(true);
    apply_button.set_can_default();
    button_row.pack_start(setasdefault_button, false, false, 0);
    button_row.pack_end(close_button, false, false, VB_MARGIN);
    button_row.pack_end(apply_button, false, false, VB_MARGIN);

    Gtk::Box *contents = _getContents();
    contents->set_spacing(4);
    contents->pack_start(notebook, true, true);
    contents->pack_start(button_row, false, false, VB_MARGIN);

    /* Signal handlers */
    g_signal_connect ( G_OBJECT (fontsel), "font_set", G_CALLBACK (onFontChange), this );
    g_signal_connect ( G_OBJECT (spacing_combo), "changed", G_CALLBACK (onLineSpacingChange), this );
    g_signal_connect ( G_OBJECT (text_buffer), "changed", G_CALLBACK (onTextChange), this );
    g_signal_connect(startOffset, "changed", G_CALLBACK(onStartOffsetChange), this);
    setasdefault_button.signal_clicked().connect(sigc::mem_fun(*this, &TextEdit::onSetDefault));
    apply_button.signal_clicked().connect(sigc::mem_fun(*this, &TextEdit::onApply));
    close_button.signal_clicked().connect(sigc::bind(_signal_response.make_slot(), GTK_RESPONSE_CLOSE));

    desktopChangeConn = deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &TextEdit::setTargetDesktop) );
    deskTrack.connect(GTK_WIDGET(gobj()));

    show_all_children();
}

TextEdit::~TextEdit()
{
    selectModifiedConn.disconnect();
    subselChangedConn.disconnect();
    selectChangedConn.disconnect();
    desktopChangeConn.disconnect();
    deskTrack.disconnect();
}

void TextEdit::styleButton(Gtk::RadioButton *button, gchar const *tooltip, gchar const *icon_name, Gtk::RadioButton *group_button )
{
    GtkWidget *icon = sp_icon_new( Inkscape::ICON_SIZE_SMALL_TOOLBAR, icon_name );
    if (!GTK_IS_IMAGE(icon)) {
        icon = gtk_image_new_from_icon_name ( icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR );
    }

    if (group_button) {
        Gtk::RadioButton::Group group = group_button->get_group();
        button->set_group(group);
    }

    button->add(*Gtk::manage(Glib::wrap(icon)));
    button->set_tooltip_text(tooltip);
    button->set_relief(Gtk::RELIEF_NONE);
    button->set_mode(false);
    button->signal_clicked().connect(sigc::mem_fun(*this, &TextEdit::onToggle));

    layout_hbox.pack_start(*button, false, false);
}

void TextEdit::onSelectionModified(guint flags )
{
    gboolean style, content;

    style =  ((flags & ( SP_OBJECT_CHILD_MODIFIED_FLAG |
                    SP_OBJECT_STYLE_MODIFIED_FLAG  )) != 0 );

    content = ((flags & ( SP_OBJECT_CHILD_MODIFIED_FLAG |
                    SP_TEXT_CONTENT_MODIFIED_FLAG  )) != 0 );

    onReadSelection (style, content);
}

void TextEdit::onReadSelection ( gboolean dostyle, gboolean /*docontent*/ )
{
    if (blocked)
        return;

    if (!desktop || SP_ACTIVE_DESKTOP != desktop)
    {
        return;
    }

    blocked = true;

    SPItem *text = getSelectedTextItem ();

    Glib::ustring phrase = samplephrase;

    if (text)
    {
        guint items = getSelectedTextCount ();
        if (items == 1) {
            gtk_widget_set_sensitive (text_view, TRUE);
            gtk_widget_set_sensitive( startOffset, SP_IS_TEXT_TEXTPATH(text) );
            if (SP_IS_TEXT_TEXTPATH(text)) {
                SPTextPath *tp = SP_TEXTPATH(text->firstChild());
                if (tp->getAttribute("startOffset")) {
                    gtk_entry_set_text(reinterpret_cast<GtkEntry *>(gtk_bin_get_child(reinterpret_cast<GtkBin *>(startOffset))), tp->getAttribute("startOffset"));
                }
            }
        } else {
            gtk_widget_set_sensitive (text_view, FALSE);
            gtk_widget_set_sensitive( startOffset, FALSE );
        }
        apply_button.set_sensitive ( false );
        setasdefault_button.set_sensitive ( true );

        //if (docontent) {  // When would we NOT want to show the content ?
            gchar *str;
            str = sp_te_get_string_multiline (text);
            if (str) {
                if (items == 1) {
                    gtk_text_buffer_set_text (text_buffer, str, strlen (str));
                    gtk_text_buffer_set_modified (text_buffer, FALSE);
                }
                phrase = str;

            } else {
                gtk_text_buffer_set_text (text_buffer, "", 0);
            }
        //} // end of if (docontent)
        text->getRepr(); // was being called but result ignored. Check this.
    } else {
        gtk_widget_set_sensitive (text_view, FALSE);
        gtk_widget_set_sensitive( startOffset, FALSE );
        apply_button.set_sensitive ( false );
        setasdefault_button.set_sensitive ( false );
    }

    if (dostyle) {
        // create temporary style
        SPStyle query(SP_ACTIVE_DOCUMENT);
        // query style from desktop into it. This returns a result flag and fills query with the style of subselection, if any, or selection
        //int result_fontspec = sp_desktop_query_style (SP_ACTIVE_DESKTOP, &query, QUERY_STYLE_PROPERTY_FONT_SPECIFICATION);
        int result_family = sp_desktop_query_style (SP_ACTIVE_DESKTOP, &query, QUERY_STYLE_PROPERTY_FONTFAMILY);
        int result_style = sp_desktop_query_style (SP_ACTIVE_DESKTOP, &query, QUERY_STYLE_PROPERTY_FONTSTYLE);
        int result_numbers = sp_desktop_query_style (SP_ACTIVE_DESKTOP, &query, QUERY_STYLE_PROPERTY_FONTNUMBERS);

        // If querying returned nothing, read the style from the text tool prefs (default style for new texts)
        // (Ok to not get a font specification - must just rely on the family and style in that case)
        if (result_family == QUERY_STYLE_NOTHING || result_style == QUERY_STYLE_NOTHING
                || result_numbers == QUERY_STYLE_NOTHING) {
            query.readFromPrefs("/tools/text");
        }

        // FIXME: process result_family/style == QUERY_STYLE_MULTIPLE_DIFFERENT by showing "Many" in the lists

        Inkscape::FontLister* fontlister = Inkscape::FontLister::get_instance();

        // This is normally done for us by text-toolbar but only when we are in text editing context
        fontlister->update_font_list(this->desktop->getDocument());
        fontlister->selection_update();

        Glib::ustring fontspec = fontlister->get_fontspec();

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int unit = prefs->getInt("/options/font/unitType", SP_CSS_UNIT_PT);
        double size = sp_style_css_size_px_to_units(query.font_size.computed, unit); 
        sp_font_selector_set_fontspec(fsel, fontspec, size );

        setPreviewText (fontspec, phrase);

        if (query.text_anchor.computed == SP_CSS_TEXT_ANCHOR_START) {
            if (query.text_align.computed == SP_CSS_TEXT_ALIGN_JUSTIFY) {
                align_justify.set_active();
            } else {
                align_left.set_active();
            }
        } else if (query.text_anchor.computed == SP_CSS_TEXT_ANCHOR_MIDDLE) {
            align_center.set_active();
        } else {
            align_right.set_active();
        }

        if (query.writing_mode.computed == SP_CSS_WRITING_MODE_LR_TB) {
            text_horizontal.set_active();
        } else {
            text_vertical.set_active();
        }

        double height;
        if (query.line_height.normal) height = Inkscape::Text::Layout::LINE_HEIGHT_NORMAL;
        else if (query.line_height.unit == SP_CSS_UNIT_PERCENT)
            height = query.line_height.value;
        else height = query.line_height.computed;
        gchar *sstr = g_strdup_printf ("%d%%", (int) floor(height * 100 + 0.5));

        gtk_entry_set_text ((GtkEntry *) gtk_bin_get_child ((GtkBin *) spacing_combo), sstr);
        g_free(sstr);

        // Update font variant widget
        //int result_variants =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, &query, QUERY_STYLE_PROPERTY_FONTVARIANTS);
        vari_vbox.update( query.font_variant_ligatures.computed, query.font_variant_ligatures.value );

    }
    blocked = false;
}


void TextEdit::setPreviewText (Glib::ustring font_spec, Glib::ustring phrase)
{
    if (font_spec.empty()) {
        return;
    }

    Glib::ustring phrase_escaped = Glib::Markup::escape_text( phrase );

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int unit = prefs->getInt("/options/font/unitType", SP_CSS_UNIT_PT);
    double pt_size = Inkscape::Util::Quantity::convert(sp_style_css_size_units_to_px(sp_font_selector_get_size(fsel), unit), "px", "pt");

    // Pango font size is in 1024ths of a point
    // C++11: Glib::ustring size = std::to_string( int(pt_size * PANGO_SCALE) );
    std::ostringstream size_st;
    size_st << int(pt_size * PANGO_SCALE); // Markup code expects integers

    Glib::ustring markup = "<span font=\'" + font_spec +
        "\' size=\'" + size_st.str() + "\'>" + phrase_escaped + "</span>";

    preview_label.set_markup(markup.c_str());
}


SPItem *TextEdit::getSelectedTextItem (void)
{
    if (!SP_ACTIVE_DESKTOP)
        return NULL;

    std::vector<SPItem*> tmp=SP_ACTIVE_DESKTOP->getSelection()->itemList();
	for(std::vector<SPItem*>::const_iterator i=tmp.begin();i!=tmp.end();i++)
    {
        if (SP_IS_TEXT(*i) || SP_IS_FLOWTEXT(*i))
            return *i;
    }

    return NULL;
}


unsigned TextEdit::getSelectedTextCount (void)
{
    if (!SP_ACTIVE_DESKTOP)
        return 0;

    unsigned int items = 0;

    std::vector<SPItem*> tmp=SP_ACTIVE_DESKTOP->getSelection()->itemList();
	for(std::vector<SPItem*>::const_iterator i=tmp.begin();i!=tmp.end();i++)
    {
        if (SP_IS_TEXT(*i) || SP_IS_FLOWTEXT(*i))
            ++items;
    }

    return items;
}

void TextEdit::onSelectionChange()
{
    onReadSelection (TRUE, TRUE);
}

void TextEdit::updateObjectText ( SPItem *text )
{
        GtkTextIter start, end;

        // write text
        if (gtk_text_buffer_get_modified (text_buffer)) {
            gtk_text_buffer_get_bounds (text_buffer, &start, &end);
            gchar *str = gtk_text_buffer_get_text (text_buffer, &start, &end, TRUE);
            sp_te_set_repr_text_multiline (text, str);
            g_free (str);
            gtk_text_buffer_set_modified (text_buffer, FALSE);
        }
}

SPCSSAttr *TextEdit::fillTextStyle ()
{
        SPCSSAttr *css = sp_repr_css_attr_new ();

        Glib::ustring fontspec = sp_font_selector_get_fontspec (fsel);

        if( !fontspec.empty() ) {

            Inkscape::FontLister *fontlister = Inkscape::FontLister::get_instance();
            fontlister->fill_css( css, fontspec );

            // TODO, possibly move this to FontLister::set_css to be shared.
            Inkscape::CSSOStringStream os;
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            int unit = prefs->getInt("/options/font/unitType", SP_CSS_UNIT_PT);
            if (prefs->getBool("/options/font/textOutputPx", true)) {
                os << sp_style_css_size_units_to_px(sp_font_selector_get_size (fsel), unit) << sp_style_get_css_unit_string(SP_CSS_UNIT_PX);
            } else {
                os << sp_font_selector_get_size (fsel) << sp_style_get_css_unit_string(unit);
            }
            sp_repr_css_set_property (css, "font-size", os.str().c_str());
        }

        // Layout
        if ( align_left.get_active() ) {
            sp_repr_css_set_property (css, "text-anchor", "start");
            sp_repr_css_set_property (css, "text-align", "start");
        } else  if ( align_center.get_active() ) {
            sp_repr_css_set_property (css, "text-anchor", "middle");
            sp_repr_css_set_property (css, "text-align", "center");
        } else  if ( align_right.get_active() ){
            sp_repr_css_set_property (css, "text-anchor", "end");
            sp_repr_css_set_property (css, "text-align", "end");
        } else {
            // Align Justify
            sp_repr_css_set_property (css, "text-anchor", "start");
            sp_repr_css_set_property (css, "text-align", "justify");
        }

        if (text_horizontal.get_active()) {
            sp_repr_css_set_property (css, "writing-mode", "lr");
        } else {
            sp_repr_css_set_property (css, "writing-mode", "tb");
        }

        // Note that CSS 1.1 does not support line-height; we set it for consistency, but also set
        // sodipodi:linespacing for backwards compatibility; in 1.2 we use line-height for flowtext

        const gchar *sstr = gtk_combo_box_text_get_active_text ((GtkComboBoxText *) spacing_combo);
        sp_repr_css_set_property (css, "line-height", sstr);

        return css;
}

void TextEdit::onSetDefault()
{
    SPCSSAttr *css = fillTextStyle ();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    blocked = true;
    prefs->mergeStyle("/tools/text/style", css);
    blocked = false;

    sp_repr_css_attr_unref (css);

    setasdefault_button.set_sensitive ( false );
}

void TextEdit::onApply()
{
    blocked = true;

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    unsigned items = 0;
    const std::vector<SPItem*> item_list = desktop->getSelection()->itemList();
    SPCSSAttr *css = fillTextStyle ();
    sp_desktop_set_style(desktop, css, true);

	for(std::vector<SPItem*>::const_iterator i=item_list.begin();i!=item_list.end();i++){
        // apply style to the reprs of all text objects in the selection
        if (SP_IS_TEXT (*i)) {

            // backwards compatibility:
            (*i)->getRepr()->setAttribute("sodipodi:linespacing", sp_repr_css_property (css, "line-height", NULL));

            ++items;
        }
        else if (SP_IS_FLOWTEXT (*i))
            // no need to set sodipodi:linespacing, because Inkscape never supported it on flowtext
            ++items;
    }

    if (items == 0) {
        // no text objects; apply style to prefs for new objects
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->mergeStyle("/tools/text/style", css);
        setasdefault_button.set_sensitive ( false );

    } else if (items == 1) {
        // exactly one text object; now set its text, too
        SPItem *item = SP_ACTIVE_DESKTOP->getSelection()->singleItem();
        if (SP_IS_TEXT (item) || SP_IS_FLOWTEXT(item)) {
            updateObjectText (item);
        }
    }

    // Update FontLister
    Glib::ustring fontspec = sp_font_selector_get_fontspec (fsel);
    if( !fontspec.empty() ) {
        Inkscape::FontLister *fontlister = Inkscape::FontLister::get_instance();
        fontlister->set_fontspec( fontspec, false );
    }

    // complete the transaction
    DocumentUndo::done(SP_ACTIVE_DESKTOP->getDocument(), SP_VERB_CONTEXT_TEXT,
                       _("Set text style"));
    apply_button.set_sensitive ( false );

    sp_repr_css_attr_unref (css);
    blocked = false;
}

void TextEdit::onTextChange (GtkTextBuffer *text_buffer, TextEdit *self)
{
    if (!self || self->blocked) {
        return;
    }

    SPItem *text = self->getSelectedTextItem();

    GtkTextIter start;
    GtkTextIter end;
    gtk_text_buffer_get_bounds (text_buffer, &start, &end);
    gchar *str = gtk_text_buffer_get_text(text_buffer, &start, &end, TRUE);
    Glib::ustring fontspec = sp_font_selector_get_fontspec(self->fsel);

    if( !fontspec.empty() ) {
        const gchar *phrase = str && *str ? str : self->samplephrase.c_str();
        self->setPreviewText(fontspec, phrase);
    } else {
        self->preview_label.set_markup("");
    }
    g_free (str);

    if (text) {
        self->apply_button.set_sensitive ( true );
        //self->onApply();
    }
    self->setasdefault_button.set_sensitive ( true);
}

void TextEdit::onFontChange(SPFontSelector * /*fontsel*/, gchar* fontspec, TextEdit *self)
{
    GtkTextIter start, end;
    gchar *str;

    if (!self || self->blocked)
        return;

    SPItem *text = self->getSelectedTextItem ();

    gtk_text_buffer_get_bounds (self->text_buffer, &start, &end);
    str = gtk_text_buffer_get_text (self->text_buffer, &start, &end, TRUE);

    if (fontspec) {
        const gchar *phrase = str && *str ? str : self->samplephrase.c_str();
        self->setPreviewText(fontspec, phrase);
    } else {
        self->preview_label.set_markup("");
    }
    g_free(str);

    if (text) {
        self->apply_button.set_sensitive ( true );
        //self->onApply();
    }
    self->setasdefault_button.set_sensitive ( true );

}

void TextEdit::onStartOffsetChange(GtkTextBuffer * /*text_buffer*/, TextEdit *self)
{
    SPItem *text = self->getSelectedTextItem();
    if (text && SP_IS_TEXT_TEXTPATH(text))
    {
        SPTextPath *tp = SP_TEXTPATH(text->firstChild());
        const gchar *sstr = gtk_combo_box_text_get_active_text(reinterpret_cast<GtkComboBoxText *>(self->startOffset));
        tp->setAttribute("startOffset", sstr);

        DocumentUndo::maybeDone(SP_ACTIVE_DESKTOP->getDocument(), "startOffset", SP_VERB_CONTEXT_TEXT, _("Set text style"));
    }
}

void TextEdit::onToggle()
{
    if (blocked)
        return;

    SPItem *text = getSelectedTextItem ();

    if (text) {
        apply_button.set_sensitive ( true );
        //onApply();
    }
    setasdefault_button.set_sensitive ( true );
}


void TextEdit::onLineSpacingChange(GtkComboBox* /*widget*/, gpointer data)
{
    TextEdit *self  = static_cast<TextEdit *>(data);
    if (!self || self->blocked)
        return;

    SPItem *text = self->getSelectedTextItem ();

    if (text) {
        self->apply_button.set_sensitive ( true );
        //self->onApply();
    }
    self->setasdefault_button.set_sensitive ( true );
}

void TextEdit::setDesktop(SPDesktop *desktop)
{
    Panel::setDesktop(desktop);
    deskTrack.setBase(desktop);
}

void TextEdit::setTargetDesktop(SPDesktop *desktop)
{
    if (this->desktop != desktop) {
        if (this->desktop) {
            selectModifiedConn.disconnect();
            subselChangedConn.disconnect();
            selectChangedConn.disconnect();
        }
        this->desktop = desktop;
        if (desktop && desktop->selection) {
            selectChangedConn = desktop->selection->connectChanged(sigc::hide(sigc::mem_fun(*this, &TextEdit::onSelectionChange)));
            subselChangedConn = desktop->connectToolSubselectionChanged(sigc::hide(sigc::mem_fun(*this, &TextEdit::onSelectionChange)));
            selectModifiedConn = desktop->selection->connectModified(sigc::hide<0>(sigc::mem_fun(*this, &TextEdit::onSelectionModified)));
        }
        //widget_setup();
        onReadSelection (TRUE, TRUE);
    }
}

} //namespace Dialog
} //namespace UI
} //namespace Inkscape

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
