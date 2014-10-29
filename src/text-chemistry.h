#ifndef SEEN_TEXT_CHEMISTRY_H
#define SEEN_TEXT_CHEMISTRY_H

// TODO move to selection-chemistry?

/*
 * Text commands
 *
 * Authors:
 *   bulia byak
 *
 * Copyright (C) 2004 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

void text_put_on_path(void);
void text_remove_from_path(void);
void text_remove_all_kerns(void);
void text_flow_into_shape();
void text_unflow();
void flowtext_to_text();

#endif // SEEN_TEXT_CHEMISTRY_H

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
