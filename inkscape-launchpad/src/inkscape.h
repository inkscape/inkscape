#ifndef __INKSCAPE_H__
#define __INKSCAPE_H__

/*
 * Interface to main application
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <list>
#include <glib.h>

class SPDesktop;
class SPDocument;

namespace Inkscape {
namespace UI {
namespace Tools {

class ToolBase;

}
}
}

struct InkscapeApplication;

namespace Inkscape {
    class ActionContext;
    namespace XML {
        class Node;
        struct Document;
        }
}

#define INKSCAPE inkscape_get_instance()

void inkscape_autosave_init();

void inkscape_application_init (const gchar *argv0, gboolean use_gui);

bool inkscape_load_config (const gchar *filename, Inkscape::XML::Document *config, const gchar *skeleton, unsigned int skel_size, const gchar *e_notreg, const gchar *e_notxml, const gchar *e_notsp, const gchar *warn);

/* Menus */
bool inkscape_load_menus (InkscapeApplication * inkscape);
bool inkscape_save_menus (InkscapeApplication * inkscape);
Inkscape::XML::Node *inkscape_get_menus (InkscapeApplication * inkscape);

InkscapeApplication *inkscape_get_instance();
gboolean inkscape_use_gui();

bool inkscapeIsCrashing();

SPDesktop * inkscape_find_desktop_by_dkey (unsigned int dkey);

#define SP_ACTIVE_EVENTCONTEXT inkscape_active_event_context ()
Inkscape::UI::Tools::ToolBase * inkscape_active_event_context (void);

#define SP_ACTIVE_DOCUMENT inkscape_active_document ()
SPDocument * inkscape_active_document (void);

#define SP_ACTIVE_DESKTOP inkscape_active_desktop ()
SPDesktop * inkscape_active_desktop (void);

// Use this function to get selection model etc for a document, if possible!
// The "active" alternative below has all the horrible static cling of a singleton.
Inkscape::ActionContext
inkscape_action_context_for_document(SPDocument *doc);

// More horrible static cling... sorry about this. Should really replace all of
// the static stuff with a single instance of some kind of engine class holding
// all the document / non-GUI stuff, and an optional GUI class that behaves a
// bit like SPDesktop does currently. Then it will be easier to write good code
// that doesn't just expect a GUI all the time (like lots of the app currently
// does).
// Also, while the "active" document / desktop concepts are convenient, they
// appear to have been abused somewhat, further increasing static cling.
Inkscape::ActionContext inkscape_active_action_context();

bool inkscape_is_sole_desktop_for_document(SPDesktop const &desktop);

gchar *homedir_path(const char *filename);
gchar *profile_path(const char *filename);

/* Inkscape desktop stuff */
void inkscape_activate_desktop (SPDesktop * desktop);
void inkscape_switch_desktops_next ();
void inkscape_switch_desktops_prev ();
void inkscape_get_all_desktops (std::list< SPDesktop* >& listbuf);

void inkscape_dialogs_hide ();
void inkscape_dialogs_unhide ();
void inkscape_dialogs_toggle ();

void inkscape_external_change ();
void inkscape_subselection_changed (SPDesktop *desktop);

/* Moved document add/remove functions into public inkscape.h as they are used
  (rightly or wrongly) by console-mode functions */
void inkscape_add_document (SPDocument *document);
bool inkscape_remove_document (SPDocument *document);

/*
 * fixme: This has to be rethought
 */

void inkscape_refresh_display (InkscapeApplication *inkscape);

/*
 * fixme: This also
 */

void inkscape_exit (InkscapeApplication *inkscape);

#endif

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
