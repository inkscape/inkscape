#ifndef __SP_SHORTCUTS_H__
#define __SP_SHORTCUTS_H__

/*
 * Keyboard shortcut processing
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

namespace Inkscape {
    class Verb;
    namespace UI {
        namespace View {
            class View;
        }
    }
}

/* We define high-bit mask for packing into single int */

#define SP_SHORTCUT_SHIFT_MASK (1 << 24)
#define SP_SHORTCUT_CONTROL_MASK (1 << 25)
#define SP_SHORTCUT_ALT_MASK (1 << 26)
#define SP_SHORTCUT_MODIFIER_MASK (SP_SHORTCUT_SHIFT_MASK|SP_SHORTCUT_CONTROL_MASK|SP_SHORTCUT_ALT_MASK)

/* Returns true if action was performed */
bool sp_shortcut_invoke (unsigned int shortcut, Inkscape::UI::View::View *view);

Inkscape::Verb * sp_shortcut_get_verb (unsigned int shortcut);
unsigned int sp_shortcut_get_primary (Inkscape::Verb * verb); // Returns GDK_VoidSymbol if no shortcut is found.
char* sp_shortcut_get_label (unsigned int shortcut); // Returns the human readable form of the shortcut (or NULL), for example Shift+Ctrl+F. Free the returned string with g_free.

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
