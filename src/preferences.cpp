/** \file
 * \brief  Prefs handling implementation
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>

#include "preferences-skeleton.h"
#include "xml/repr.h"
#include "dialogs/input.h"
#include "inkscape.h"
#include "preferences.h"

#define PREFERENCES_FILE "preferences.xml"

static Inkscape::XML::Document *_preferences;
static bool _save_preferences;

namespace Inkscape {

void
Preferences::loadSkeleton()
{
    _preferences = sp_repr_read_mem (preferences_skeleton, PREFERENCES_SKELETON_SIZE, 0);
}

Inkscape::XML::Document*
Preferences::get()
{
    return _preferences;
}

/**
 *  Attempts to load the preferences file indicated by the global PREFERENCES_FILE
 *  parameter.  If it cannot load it, the default preferences_skeleton will be used
 *  instead.
 */
void
Preferences::load()
{
    /// \todo this still uses old Gtk+ code which should be somewhere else
    if (inkscape_load_config (PREFERENCES_FILE, 
                              _preferences,
                              preferences_skeleton, 
                              PREFERENCES_SKELETON_SIZE,
                              _("%s is not a regular file.\n%s"),
                              _("%s not a valid XML file, or\n"
                                "you don't have read permissions on it.\n%s"),
                              _("%s is not a valid preferences file.\n%s"),
                              _("Inkscape will run with default settings.\n"
                                "New settings will not be saved.")))
    {
        sp_input_load_from_preferences();
        _save_preferences = true;
    } else
        _save_preferences = false;
}

void
Preferences::save()
{
    if (!_preferences || ! _save_preferences)
        return;
    
    gchar *fn = profile_path (PREFERENCES_FILE);
    (void) sp_repr_save_file (_preferences, fn);
    g_free (fn);
}


} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:75
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
