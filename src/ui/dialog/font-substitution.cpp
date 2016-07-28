/*
 * Authors:
 *
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/messagedialog.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>

#include <set>
#include "font-substitution.h"

#include "inkscape.h"
#include "desktop.h"
#include "document.h"
#include "selection.h"

#include "ui/dialog-events.h"

#include "selection-chemistry.h"
#include "preferences.h"

#include "xml/repr.h"

#include "sp-defs.h"
#include "sp-root.h"
#include "sp-text.h"
#include "sp-textpath.h"
#include "sp-flowtext.h"
#include "sp-flowdiv.h"
#include "sp-tspan.h"
#include "sp-tref.h"
#include "style.h"
#include "text-editing.h"

#include "libnrtype/FontFactory.h"
#include "libnrtype/font-instance.h"

#include <glibmm/i18n.h>
#include <glibmm/regex.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

FontSubstitution::FontSubstitution()
{
}

FontSubstitution::~FontSubstitution()
{
}

void
FontSubstitution::checkFontSubstitutions(SPDocument* doc)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int show_dlg = prefs->getInt("/options/font/substitutedlg", 0);
    if (show_dlg) {
        Glib::ustring out;
        std::vector<SPItem*> l =  getFontReplacedItems(doc, &out);
        if (out.length() > 0) {
            show(out, l);
        }
    }
}

void
FontSubstitution::show(Glib::ustring out, std::vector<SPItem*> &l)
{
   Gtk::MessageDialog warning(_("\nSome fonts are not available and have been substituted."),
                       false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
   warning.set_resizable(true);
   warning.set_title(_("Font substitution"));

   GtkWidget *dlg = GTK_WIDGET(warning.gobj());
   sp_transientize(dlg);

   Gtk::TextView * textview = new Gtk::TextView();
   textview->set_editable(false);
   textview->set_wrap_mode(Gtk::WRAP_WORD);
   textview->show();
   textview->get_buffer()->set_text(_(out.c_str()));

   Gtk::ScrolledWindow * scrollwindow = new Gtk::ScrolledWindow();
   scrollwindow->add(*textview);
   scrollwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrollwindow->set_shadow_type(Gtk::SHADOW_IN);
   scrollwindow->set_size_request(0, 100);
   scrollwindow->show();

   Gtk::CheckButton *cbSelect = new Gtk::CheckButton();
   cbSelect->set_label(_("Select all the affected items"));
   cbSelect->set_active(true);
   cbSelect->show();

   Gtk::CheckButton *cbWarning = new Gtk::CheckButton();
   cbWarning->set_label(_("Don't show this warning again"));
   cbWarning->show();

#if GTK_CHECK_VERSION(3,0,0)
   Gtk::Box * box = warning.get_content_area();
#else
   Gtk::Box * box = warning.get_vbox();
#endif
   box->set_spacing(2);
   box->pack_start(*scrollwindow, true, true, 4);
   box->pack_start(*cbSelect, false, false, 0);
   box->pack_start(*cbWarning, false, false, 0);

   warning.run();

   if (cbWarning->get_active()) {
       Inkscape::Preferences *prefs = Inkscape::Preferences::get();
       prefs->setInt("/options/font/substitutedlg", 0);
   }

   if (cbSelect->get_active()) {

       SPDesktop *desktop = SP_ACTIVE_DESKTOP;
       Inkscape::Selection *selection = desktop->getSelection();
       selection->clear();
       selection->setList(l);
   }

}

/*
 * Find all the fonts that are in the document but not available on the users system
 * and have been substituted for other fonts
 *
 * Return a list of SPItems where fonts have been substituted.
 *
 * Walk thru all the objects ...
 * a. Build up a list of the objects with fonts defined in the style attribute
 * b. Build up a list of the objects rendered fonts - taken for the objects layout/spans
 * If there are fonts in a. that are not in b. then those fonts have been substituted.
 */
std::vector<SPItem*> FontSubstitution::getFontReplacedItems(SPDocument* doc, Glib::ustring *out)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    std::vector<SPItem*> allList;
    std::vector<SPItem*> outList,x,y;
    std::set<Glib::ustring> setErrors;
    std::set<Glib::ustring> setFontSpans;
    std::map<SPItem *, Glib::ustring> mapFontStyles;

    allList = get_all_items(x, doc->getRoot(), desktop, false, false, true, y);
    for(std::vector<SPItem*>::const_iterator i = allList.begin();i!=allList.end();++i){
        SPItem *item = *i;
        SPStyle *style = item->style;
        Glib::ustring family = "";

        if (is_top_level_text_object (item)) {
            // Should only need to check the first span, since the others should be covered by TSPAN's etc
            family = te_get_layout(item)->getFontFamily(0);
            setFontSpans.insert(family);
        }
        else if (SP_IS_TEXTPATH(item)) {
            SPTextPath const *textpath = SP_TEXTPATH(item);
            if (textpath->originalPath != NULL) {
                family = SP_TEXT(item->parent)->layout.getFontFamily(0);
                setFontSpans.insert(family);
            }
        }
        else if (SP_IS_TSPAN(item) || SP_IS_FLOWTSPAN(item)) {
            // is_part_of_text_subtree (item)
             // TSPAN layout comes from the parent->layout->_spans
             SPObject *parent_text = item;
             while (parent_text && !SP_IS_TEXT(parent_text)) {
                 parent_text = parent_text->parent;
             }
             if (parent_text != NULL) {
                 family = SP_TEXT(parent_text)->layout.getFontFamily(0);
                 // Add all the spans fonts to the set
                 gint ii = 0;
                 for (SPObject *child = parent_text->firstChild() ; child ; child = child->getNext() ) {
                     family = SP_TEXT(parent_text)->layout.getFontFamily(ii);
                     setFontSpans.insert(family);
                     ii++;
                 }
             }
        }

        if (style) {
            gchar const *style_font = NULL;
            if (style->font_family.set)
                style_font = style->font_family.value;
            else if (style->font_specification.set)
                style_font = style->font_specification.value;
            else if (style->font_family.value)
                style_font = style->font_family.value;
            else if (style->font_specification.value)
                style_font = style->font_specification.value;

            if (style_font) {
                if (has_visible_text(item)) {
                    mapFontStyles.insert(std::make_pair (item, style_font));
                }
            }
        }
    }

    // Check if any document styles are not in the actual layout
    std::map<SPItem *, Glib::ustring>::const_reverse_iterator mapIter;
    for (mapIter = mapFontStyles.rbegin(); mapIter != mapFontStyles.rend(); ++mapIter) {
        SPItem *item = mapIter->first;
        Glib::ustring fonts = mapIter->second;

        // CSS font fallbacks can have more that one font listed, split the font list
        std::vector<Glib::ustring> vFonts = Glib::Regex::split_simple("," , fonts);
        bool fontFound = false;
        for(size_t i=0; i<vFonts.size(); i++) {
            Glib::ustring font = vFonts[i];
            // trim whitespace
            size_t startpos = font.find_first_not_of(" \n\r\t");
            size_t endpos = font.find_last_not_of(" \n\r\t");
            if(( std::string::npos == startpos ) || ( std::string::npos == endpos)) {
                continue; // empty font name
            }
            font = font.substr( startpos, endpos-startpos+1 );
            std::set<Glib::ustring>::const_iterator iter = setFontSpans.find(font);
            if (iter != setFontSpans.end() ||
                    font == Glib::ustring("sans-serif") ||
                    font == Glib::ustring("Sans") ||
                    font == Glib::ustring("serif") ||
                    font == Glib::ustring("Serif") ||
                    font == Glib::ustring("monospace") ||
                    font == Glib::ustring("Monospace")) {
                fontFound = true;
                break;
            }
        }
        if (fontFound == false) {
            Glib::ustring subName = getSubstituteFontName(fonts);
            Glib::ustring err = Glib::ustring::compose(
                    _("Font '%1' substituted with '%2'"), fonts.c_str(), subName.c_str());
            setErrors.insert(err);
            outList.push_back(item);
        }
    }

    std::set<Glib::ustring>::const_iterator setIter;
    for (setIter = setErrors.begin(); setIter != setErrors.end(); ++setIter) {
        Glib::ustring err = (*setIter);
        out->append(err + "\n");
        g_warning("%s", err.c_str());
    }

    return outList;
}


Glib::ustring FontSubstitution::getSubstituteFontName (Glib::ustring font)
{
    Glib::ustring out = font;

    PangoFontDescription *descr = pango_font_description_new();
    pango_font_description_set_family(descr,font.c_str());
    font_instance *res = (font_factory::Default())->Face(descr);
    if (res->pFont) {
        PangoFontDescription *nFaceDesc = pango_font_describe(res->pFont);
        out = sp_font_description_get_family(nFaceDesc);
    }
    pango_font_description_free(descr);

    return out;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
