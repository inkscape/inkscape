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
namespace Dialogs {


GlyphsPanel &GlyphsPanel::getInstance()
{
    return *new GlyphsPanel();
}


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
    label(0)
{
    Gtk::Table *table = new Gtk::Table(3, 1, false);
    _getContents()->pack_start(*Gtk::manage(table), Gtk::PACK_EXPAND_WIDGET);
    guint row = 0;

// -------------------------------

    GtkWidget *fontsel = sp_font_selector_new();
    g_signal_connect( G_OBJECT(fontsel), "font_set", G_CALLBACK(fontChangeCB), this );

    table->attach(*Gtk::manage(Glib::wrap(fontsel)),
                  0, 1, row, row + 1,
                  Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL);
    row++;


// -------------------------------

    GlyphColumns *columns = getColumns();

    iconView = new Gtk::IconView(store);
    iconView->set_text_column(columns->name);
    //iconView->set_columns(16);

    iconView->signal_item_activated().connect(sigc::mem_fun(*this, &GlyphsPanel::glyphActivated));
    iconView->signal_selection_changed().connect(sigc::mem_fun(*this, &GlyphsPanel::glyphSelectionChanged));


    Gtk::ScrolledWindow *scroller = new Gtk::ScrolledWindow();
    scroller->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
    scroller->add(*Gtk::manage(iconView));
    table->attach(*Gtk::manage(scroller),
                  0, 1, row, row + 1,
                  Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
    row++;

// -------------------------------

    Gtk::HBox *box = new Gtk::HBox();

    entry = new Gtk::Entry();
    box->pack_start(*Gtk::manage(entry));

    label = new Gtk::Label("      ");
    box->pack_start(*Gtk::manage(label), Gtk::PACK_EXPAND_PADDING);

    GtkWidget *applyBtn = gtk_button_new_from_stock(GTK_STOCK_APPLY);
    GTK_WIDGET_SET_FLAGS(applyBtn, GTK_CAN_DEFAULT | GTK_HAS_DEFAULT);
    gtk_widget_set_sensitive(applyBtn, FALSE);

    box->pack_end(*Gtk::manage(Glib::wrap(applyBtn)), Gtk::PACK_SHRINK);

    table->attach( *Gtk::manage(box),
                  0, 1, row, row + 1,
                   Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
    row++;

// -------------------------------


    show_all_children();

    restorePanelPrefs();
}

GlyphsPanel::~GlyphsPanel()
{
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

        gchar * tmp = g_strdup_printf("U+%04X", ch);
        label->set_text(tmp);
    }
}

void GlyphsPanel::fontChangeCB(SPFontSelector *fontsel, font_instance *font, GlyphsPanel *self)
{
    if (self) {
        self->handleFontChange(fontsel, font);
    }
}


void GlyphsPanel::handleFontChange(SPFontSelector * /*fontsel*/, font_instance *font)
{
    if (font) {
        gunichar maxUni = 0;
        std::vector<gunichar> present;
        for (gunichar ch = 0; ch < 65536; ch++) {
            int glyphId = font->MapUnicodeChar(ch);
            if (glyphId > 0) {
                maxUni = std::max(maxUni, ch);
                present.push_back(ch);
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
