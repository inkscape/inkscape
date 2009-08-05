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
#include <glib/gtypes.h>

struct SPDesktop;
struct Document;
struct SPEventContext;

namespace Inkscape {
    struct Application;
    namespace XML {
        class Node;
        class Document;
        }
}

#define INKSCAPE inkscape_get_instance()

void inkscape_autosave_init();

void inkscape_application_init (const gchar *argv0, gboolean use_gui);

bool inkscape_load_config (const gchar *filename, Inkscape::XML::Document *config, const gchar *skeleton, unsigned int skel_size, const gchar *e_notreg, const gchar *e_notxml, const gchar *e_notsp, const gchar *warn);

/* Menus */
bool inkscape_load_menus (Inkscape::Application * inkscape);
bool inkscape_save_menus (Inkscape::Application * inkscape);
Inkscape::XML::Node *inkscape_get_menus (Inkscape::Application * inkscape);

Inkscape::Application *inkscape_get_instance();

#define SP_ACTIVE_EVENTCONTEXT inkscape_active_event_context ()
SPEventContext * inkscape_active_event_context (void);

#define SP_ACTIVE_DOCUMENT inkscape_active_document ()
Document * inkscape_active_document (void);

#define SP_ACTIVE_DESKTOP inkscape_active_desktop ()
SPDesktop * inkscape_active_desktop (void);

bool inkscape_is_sole_desktop_for_document(SPDesktop const &desktop);

gchar *homedir_path(const char *filename);
gchar *profile_path(const char *filename);

/* Inkscape desktop stuff */
void inkscape_switch_desktops_next ();
void inkscape_switch_desktops_prev ();
void inkscape_get_all_desktops (std::list< SPDesktop* >& listbuf);

void inkscape_dialogs_hide ();
void inkscape_dialogs_unhide ();
void inkscape_dialogs_toggle ();

void inkscape_external_change ();
void inkscape_subselection_changed (SPDesktop *desktop);

/*
 * fixme: This has to be rethought
 */

void inkscape_refresh_display (Inkscape::Application *inkscape);

/*
 * fixme: This also
 */

void inkscape_exit (Inkscape::Application *inkscape);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
