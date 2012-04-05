#ifndef SEEN_SP_INTERFACE_H
#define SEEN_SP_INTERFACE_H

/*
 * Main UI stuff
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2012 Kris De Gussem
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtk.h>
#include <gtkmm/menu.h>

#include "sp-item.h"

class SPViewWidget;

namespace Inkscape {

class Verb;

namespace UI {
namespace View {
class View;
} // namespace View
} // namespace UI
} // namespace Inkscape

/**
 *  Create a new document window.
 */
void sp_create_window (SPViewWidget *vw, gboolean editable);

/**
 *
 */
void sp_ui_close_view (GtkWidget *widget);

/**
 *
 */
void sp_ui_new_view (void);
void sp_ui_new_view_preview (void);

/**
 *
 */
unsigned int sp_ui_close_all (void);

/**
 *
 */
GtkWidget *sp_ui_main_menubar (Inkscape::UI::View::View *view);

static GtkWidget *sp_ui_menu_append_item_from_verb(GtkMenu *menu, Inkscape::Verb *verb, Inkscape::UI::View::View *view, bool radio = false, GSList *group = NULL);


/**
 *
 */
void sp_menu_append_recent_documents (GtkWidget *menu);


/**
 *
 */
void sp_ui_dialog_title_string (Inkscape::Verb * verb, gchar* c);


/**
 *
 */
void sp_ui_error_dialog (const gchar * message);
bool sp_ui_overwrite_file (const gchar * filename);

class ContextMenu : public Gtk::Menu
{
    public:
        ContextMenu(Inkscape::UI::View::View *view, SPItem *item);
        ~ContextMenu(void);
    private:
        SPItem *_item;
		SPObject *_object;
		SPDesktop *_desktop;
		
		std::vector<Gtk::SeparatorMenuItem*> separators;
        Gtk::MenuItem MIGroup;
        Gtk::MenuItem MIParent;
        
        Gtk::SeparatorMenuItem* AddSeparator(void);
        void AppendItemFromVerb(Inkscape::Verb *verb, Inkscape::UI::View::View *view);
		void MakeObjectMenu (void);
		void MakeItemMenu   (void);
		void MakeGroupMenu  (void);
		void MakeAnchorMenu (void);
		void MakeImageMenu  (void);
		void MakeShapeMenu  (void);
		void MakeTextMenu   (void);
		
		void EnterGroup(Gtk::MenuItem* mi);
		void LeaveGroup(void);
		void ItemProperties(void);
		void ItemSelectThis(void);
		void SelectSameFillStroke(void);
		void ItemCreateLink(void);
		void SetMask(void);
		void ReleaseMask(void);
		void SetClip(void);
		void ReleaseClip(void);
		
		void ActivateUngroup(void);
		
		void AnchorLinkProperties(void);
		void AnchorLinkFollow(void);
		void AnchorLinkRemove(void);
		
		void ImageProperties(void);
		void ImageEdit(void);
		Glib::ustring getImageEditorName();
		void ImageEmbed(void);
		void ImageExtract(void);
		
		void FillSettings(void);
		
		void TextSettings(void);
		void SpellcheckSettings(void);
};

#endif // SEEN_SP_INTERFACE_H

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
