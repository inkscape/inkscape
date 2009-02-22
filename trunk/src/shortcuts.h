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

/* Returns true if action was performed */
bool sp_shortcut_invoke (unsigned int shortcut, Inkscape::UI::View::View *view);

Inkscape::Verb * sp_shortcut_get_verb (unsigned int shortcut);
unsigned int sp_shortcut_get_primary (Inkscape::Verb * verb);

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
