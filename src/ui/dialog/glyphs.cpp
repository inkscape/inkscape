/**
 * Glyph selector dialog.
 */

/* Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2010 Jon A. Cruz
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>

#include <glibmm/i18n.h>
#include <gtkmm/entry.h>
#include <gtkmm/iconview.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/widget.h>

#include <gtk/gtkbutton.h>
#include <gtk/gtkstock.h>

#include "glyphs.h"

#include "verbs.h"
#include "widgets/font-selector.h"
#include "libnrtype/font-instance.h"

namespace Inkscape {
namespace UI {
namespace Dialog {


GlyphsPanel &GlyphsPanel::getInstance()
{
    return *new GlyphsPanel();
}


#if GLIB_CHECK_VERSION(2,14,0)
static std::map<GUnicodeScript, Glib::ustring> &getScriptToName()
{
    static bool init = false;
    static std::map<GUnicodeScript, Glib::ustring> mappings;
    if (!init) {
        init = true;
        mappings[G_UNICODE_SCRIPT_INVALID_CODE]         = _("all");
        mappings[G_UNICODE_SCRIPT_COMMON]               = _("common");
        mappings[G_UNICODE_SCRIPT_INHERITED]            = _("inherited");
        mappings[G_UNICODE_SCRIPT_ARABIC]               = _("Arabic");
        mappings[G_UNICODE_SCRIPT_ARMENIAN]             = _("Armenian");
        mappings[G_UNICODE_SCRIPT_BENGALI]              = _("Bengali");
        mappings[G_UNICODE_SCRIPT_BOPOMOFO]             = _("Bopomofo");
        mappings[G_UNICODE_SCRIPT_CHEROKEE]             = _("Cherokee");
        mappings[G_UNICODE_SCRIPT_COPTIC]               = _("Coptic");
        mappings[G_UNICODE_SCRIPT_CYRILLIC]             = _("Cyrillic");
        mappings[G_UNICODE_SCRIPT_DESERET]              = _("Deseret");
        mappings[G_UNICODE_SCRIPT_DEVANAGARI]           = _("Devanagari");
        mappings[G_UNICODE_SCRIPT_ETHIOPIC]             = _("Ethiopic");
        mappings[G_UNICODE_SCRIPT_GEORGIAN]             = _("Georgian");
        mappings[G_UNICODE_SCRIPT_GOTHIC]               = _("Gothic");
        mappings[G_UNICODE_SCRIPT_GREEK]                = _("Greek");
        mappings[G_UNICODE_SCRIPT_GUJARATI]             = _("Gujarati");
        mappings[G_UNICODE_SCRIPT_GURMUKHI]             = _("Gurmukhi");
        mappings[G_UNICODE_SCRIPT_HAN]                  = _("Han");
        mappings[G_UNICODE_SCRIPT_HANGUL]               = _("Hangul");
        mappings[G_UNICODE_SCRIPT_HEBREW]               = _("Hebrew");
        mappings[G_UNICODE_SCRIPT_HIRAGANA]             = _("Hiragana");
        mappings[G_UNICODE_SCRIPT_KANNADA]              = _("Kannada");
        mappings[G_UNICODE_SCRIPT_KATAKANA]             = _("Katakana");
        mappings[G_UNICODE_SCRIPT_KHMER]                = _("Khmer");
        mappings[G_UNICODE_SCRIPT_LAO]                  = _("Lao");
        mappings[G_UNICODE_SCRIPT_LATIN]                = _("Latin");
        mappings[G_UNICODE_SCRIPT_MALAYALAM]            = _("Malayalam");
        mappings[G_UNICODE_SCRIPT_MONGOLIAN]            = _("Mongolian");
        mappings[G_UNICODE_SCRIPT_MYANMAR]              = _("Myanmar");
        mappings[G_UNICODE_SCRIPT_OGHAM]                = _("Ogham");
        mappings[G_UNICODE_SCRIPT_OLD_ITALIC]           = _("Old Italic");
        mappings[G_UNICODE_SCRIPT_ORIYA]                = _("Oriya");
        mappings[G_UNICODE_SCRIPT_RUNIC]                = _("Runic");
        mappings[G_UNICODE_SCRIPT_SINHALA]              = _("Sinhala");
        mappings[G_UNICODE_SCRIPT_SYRIAC]               = _("Syriac");
        mappings[G_UNICODE_SCRIPT_TAMIL]                = _("Tamil");
        mappings[G_UNICODE_SCRIPT_TELUGU]               = _("Telugu");
        mappings[G_UNICODE_SCRIPT_THAANA]               = _("Thaana");
        mappings[G_UNICODE_SCRIPT_THAI]                 = _("Thai");
        mappings[G_UNICODE_SCRIPT_TIBETAN]              = _("Tibetan");
        mappings[G_UNICODE_SCRIPT_CANADIAN_ABORIGINAL]  = _("Canadian Aboriginal");
        mappings[G_UNICODE_SCRIPT_YI]                   = _("Yi");
        mappings[G_UNICODE_SCRIPT_TAGALOG]              = _("Tagalog");
        mappings[G_UNICODE_SCRIPT_HANUNOO]              = _("Hanunoo");
        mappings[G_UNICODE_SCRIPT_BUHID]                = _("Buhid");
        mappings[G_UNICODE_SCRIPT_TAGBANWA]             = _("Tagbanwa");
        mappings[G_UNICODE_SCRIPT_BRAILLE]              = _("Braille");
        mappings[G_UNICODE_SCRIPT_CYPRIOT]              = _("Cypriot");
        mappings[G_UNICODE_SCRIPT_LIMBU]                = _("Limbu");
        mappings[G_UNICODE_SCRIPT_OSMANYA]              = _("Osmanya");
        mappings[G_UNICODE_SCRIPT_SHAVIAN]              = _("Shavian");
        mappings[G_UNICODE_SCRIPT_LINEAR_B]             = _("Linear B");
        mappings[G_UNICODE_SCRIPT_TAI_LE]               = _("Tai Le");
        mappings[G_UNICODE_SCRIPT_UGARITIC]             = _("Ugaritic");
        mappings[G_UNICODE_SCRIPT_NEW_TAI_LUE]          = _("New Tai Lue");
        mappings[G_UNICODE_SCRIPT_BUGINESE]             = _("Buginese");
        mappings[G_UNICODE_SCRIPT_GLAGOLITIC]           = _("Glagolitic");
        mappings[G_UNICODE_SCRIPT_TIFINAGH]             = _("Tifinagh");
        mappings[G_UNICODE_SCRIPT_SYLOTI_NAGRI]         = _("Syloti Nagri");
        mappings[G_UNICODE_SCRIPT_OLD_PERSIAN]          = _("Old Persian");
        mappings[G_UNICODE_SCRIPT_KHAROSHTHI]           = _("Kharoshthi");
        mappings[G_UNICODE_SCRIPT_UNKNOWN]              = _("unassigned");
        mappings[G_UNICODE_SCRIPT_BALINESE]             = _("Balinese");
        mappings[G_UNICODE_SCRIPT_CUNEIFORM]            = _("Cuneiform");
        mappings[G_UNICODE_SCRIPT_PHOENICIAN]           = _("Phoenician");
        mappings[G_UNICODE_SCRIPT_PHAGS_PA]             = _("Phags-pa");
        mappings[G_UNICODE_SCRIPT_NKO]                  = _("N'Ko");

#if GLIB_CHECK_VERSION(2,14,0)
        mappings[G_UNICODE_SCRIPT_KAYAH_LI]             = _("Kayah Li");
        mappings[G_UNICODE_SCRIPT_LEPCHA]               = _("Lepcha");
        mappings[G_UNICODE_SCRIPT_REJANG]               = _("Rejang");
        mappings[G_UNICODE_SCRIPT_SUNDANESE]            = _("Sundanese");
        mappings[G_UNICODE_SCRIPT_SAURASHTRA]           = _("Saurashtra");
        mappings[G_UNICODE_SCRIPT_CHAM]                 = _("Cham");
        mappings[G_UNICODE_SCRIPT_OL_CHIKI]             = _("Ol Chiki");
        mappings[G_UNICODE_SCRIPT_VAI]                  = _("Vai");
        mappings[G_UNICODE_SCRIPT_CARIAN]               = _("Carian");
        mappings[G_UNICODE_SCRIPT_LYCIAN]               = _("Lycian");
        mappings[G_UNICODE_SCRIPT_LYDIAN]               = _("Lydian");
#endif // GLIB_CHECK_VERSION(2,14,0)
    }
    return mappings;
}
#endif // GLIB_CHECK_VERSION(2,14,0)


class GlyphColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    Gtk::TreeModelColumn<gunichar> code;
    Gtk::TreeModelColumn<Glib::ustring> name;

    GlyphColumns()
    {
        add(code);
        add(name);
    }
};

GlyphColumns *GlyphsPanel::getColumns()
{
    static GlyphColumns *columns = new GlyphColumns();

    return columns;
}

/**
 * Constructor
 */
GlyphsPanel::GlyphsPanel(gchar const *prefsPath) :
    Inkscape::UI::Widget::Panel("", prefsPath, SP_VERB_DIALOG_GLYPHS, "", false),
    store(Gtk::ListStore::create(*getColumns())),
    iconView(0),
    entry(0),
    label(0),
#if GLIB_CHECK_VERSION(2,14,0)
    scriptCombo(0),
#endif // GLIB_CHECK_VERSION(2,14,0)
    fsel(0),
    targetDesktop(0),
    deskTrack(),
    iconActiveConn(),
    iconSelectConn(),
    scriptSelectConn()
{
    Gtk::Table *table = new Gtk::Table(3, 1, false);
    _getContents()->pack_start(*Gtk::manage(table), Gtk::PACK_EXPAND_WIDGET);
    guint row = 0;

// -------------------------------

    GtkWidget *fontsel = sp_font_selector_new();
    fsel = SP_FONT_SELECTOR(fontsel);
    sp_font_selector_set_font(fsel, sp_font_selector_get_font(fsel), 12.0);

    g_signal_connect( G_OBJECT(fontsel), "font_set", G_CALLBACK(fontChangeCB), this );

    table->attach(*Gtk::manage(Glib::wrap(fontsel)),
                  0, 3, row, row + 1,
                  Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL);
    row++;


// -------------------------------

#if GLIB_CHECK_VERSION(2,14,0)
    {
        Gtk::Label *label = new Gtk::Label(_("Script: "));
        table->attach( *Gtk::manage(label),
                       0, 1, row, row + 1,
                       Gtk::SHRINK, Gtk::SHRINK);

        scriptCombo = new Gtk::ComboBoxText();

        std::map<GUnicodeScript, Glib::ustring> items = getScriptToName();
        for (std::map<GUnicodeScript, Glib::ustring>::iterator it = items.begin(); it != items.end(); ++it)
        {
            scriptCombo->append_text(it->second);
        }

        scriptCombo->set_active_text(getScriptToName()[G_UNICODE_SCRIPT_COMMON]); // default to a smaller set
        scriptSelectConn = scriptCombo->signal_changed().connect(sigc::mem_fun(*this, &GlyphsPanel::rebuild));

        table->attach( *Gtk::manage(scriptCombo),
                       1, 2, row, row + 1,
                       Gtk::SHRINK, Gtk::SHRINK);

        label = new Gtk::Label("");
        table->attach( *Gtk::manage(label),
                       2, 3, row, row + 1,
                       Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
    }

    row++;
#endif // GLIB_CHECK_VERSION(2,14,0)

// -------------------------------

    GlyphColumns *columns = getColumns();

    iconView = new Gtk::IconView(store);
    iconView->set_text_column(columns->name);
    //iconView->set_columns(16);

    iconActiveConn = iconView->signal_item_activated().connect(sigc::mem_fun(*this, &GlyphsPanel::glyphActivated));
    iconSelectConn = iconView->signal_selection_changed().connect(sigc::mem_fun(*this, &GlyphsPanel::glyphSelectionChanged));


    Gtk::ScrolledWindow *scroller = new Gtk::ScrolledWindow();
    scroller->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
    scroller->add(*Gtk::manage(iconView));
    table->attach(*Gtk::manage(scroller),
                  0, 3, row, row + 1,
                  Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
    row++;

// -------------------------------

    Gtk::HBox *box = new Gtk::HBox();

    entry = new Gtk::Entry();
    entry->set_width_chars(18);
    box->pack_start(*Gtk::manage(entry), Gtk::PACK_SHRINK);

    Gtk::Label *pad = new Gtk::Label("    ");
    box->pack_start(*Gtk::manage(pad), Gtk::PACK_SHRINK);

    label = new Gtk::Label("      ");
    box->pack_start(*Gtk::manage(label), Gtk::PACK_SHRINK);

    pad = new Gtk::Label("");
    box->pack_start(*Gtk::manage(pad), Gtk::PACK_EXPAND_WIDGET);


    GtkWidget *applyBtn = gtk_button_new_from_stock(GTK_STOCK_APPLY);
    GTK_WIDGET_SET_FLAGS(applyBtn, GTK_CAN_DEFAULT | GTK_HAS_DEFAULT);
    gtk_widget_set_sensitive(applyBtn, FALSE);

    box->pack_end(*Gtk::manage(Glib::wrap(applyBtn)), Gtk::PACK_SHRINK);

    table->attach( *Gtk::manage(box),
                   0, 3, row, row + 1,
                   Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
    row++;

// -------------------------------


    show_all_children();

    restorePanelPrefs();

    // Connect this up last
    desktopChangeConn = deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &GlyphsPanel::setTargetDesktop) );
    deskTrack.connect(GTK_WIDGET(gobj()));
}

GlyphsPanel::~GlyphsPanel()
{
    iconActiveConn.disconnect();
    iconSelectConn.disconnect();
    scriptSelectConn.disconnect();
    desktopChangeConn.disconnect();
}


void GlyphsPanel::setDesktop(SPDesktop *desktop)
{
    Panel::setDesktop(desktop);
    deskTrack.setBase(desktop);
}

void GlyphsPanel::setTargetDesktop(SPDesktop *desktop)
{
    if (targetDesktop != desktop) {
        targetDesktop = desktop;
    }
}

void GlyphsPanel::glyphActivated(Gtk::TreeModel::Path const & path)
{
    Gtk::ListStore::iterator row = store->get_iter(path);
    gunichar ch = (*row)[getColumns()->code];
    Glib::ustring tmp;
    tmp += ch;

    int startPos = 0;
    int endPos = 0;
    if (entry->get_selection_bounds(startPos, endPos)) {
        // there was something selected.
        entry->delete_text(startPos, endPos);
    }
    startPos = entry->get_position();
    entry->insert_text(tmp, -1, startPos);
    entry->set_position(startPos);
}

void GlyphsPanel::glyphSelectionChanged()
{
    Gtk::IconView::ArrayHandle_TreePaths itemArray = iconView->get_selected_items();
    if (itemArray.empty()) {
        label->set_text("      ");
    } else {
        Gtk::TreeModel::Path const & path = *itemArray.begin();
        Gtk::ListStore::iterator row = store->get_iter(path);
        gunichar ch = (*row)[getColumns()->code];


        Glib::ustring scriptName;
#if GLIB_CHECK_VERSION(2,14,0)
        GUnicodeScript script = g_unichar_get_script(ch);
        std::map<GUnicodeScript, Glib::ustring> mappings = getScriptToName();
        if (mappings.find(script) != mappings.end()) {
            scriptName = mappings[script];
        }
#endif
        gchar * tmp = g_strdup_printf("U+%04X %s", ch, scriptName.c_str());
        label->set_text(tmp);
    }
}

void GlyphsPanel::fontChangeCB(SPFontSelector * /*fontsel*/, font_instance * /*font*/, GlyphsPanel *self)
{
    if (self) {
        self->rebuild();
    }
}


void GlyphsPanel::rebuild()
{
    font_instance *font = fsel ? sp_font_selector_get_font(fsel) : 0;
    if (font) {
        //double  sp_font_selector_get_size (SPFontSelector *fsel);

#if GLIB_CHECK_VERSION(2,14,0)
        GUnicodeScript script = G_UNICODE_SCRIPT_COMMON;
        Glib::ustring scriptName = scriptCombo->get_active_text();
        std::map<GUnicodeScript, Glib::ustring> items = getScriptToName();
        for (std::map<GUnicodeScript, Glib::ustring>::iterator it = items.begin(); it != items.end(); ++it) {
            if (scriptName == it->second) {
                script = it->first;
                break;
            }
        }
#endif // GLIB_CHECK_VERSION(2,14,0)

        // Disconnect the model while we update it. Simple work-around for 5x+ performance boost.
        Glib::RefPtr<Gtk::ListStore> tmp = Gtk::ListStore::create(*getColumns());
        iconView->set_model(tmp);

        std::vector<gunichar> present;
        for (gunichar ch = 1; ch < 65535; ch++) {
            int glyphId = font->MapUnicodeChar(ch);
            if (glyphId > 0) {
#if GLIB_CHECK_VERSION(2,14,0)
                if ((script == G_UNICODE_SCRIPT_INVALID_CODE) || (script == g_unichar_get_script(ch))) {
                    present.push_back(ch);
                }
#else
                present.push_back(ch);
#endif
            }
        }

        GlyphColumns *columns = getColumns();
        store->clear();
        for (std::vector<gunichar>::iterator it = present.begin(); it != present.end(); ++it)
        {
            Gtk::ListStore::iterator row = store->append();
            Glib::ustring tmp;
            tmp += *it;
            (*row)[columns->code] = *it;
            (*row)[columns->name] = tmp;
        }

        // Reconnect the model once it has been updated:
        iconView->set_model(store);
    }
}


} // namespace Dialogs
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
