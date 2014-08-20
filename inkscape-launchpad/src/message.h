/*
 * status-message-related types
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_MESSAGE_H
#define SEEN_INKSCAPE_MESSAGE_H

namespace Inkscape {

/**
 * A hint about the meaning of a message; is it an ordinary message,
 * a message advising the user of some significant or unexpected condition,
 * or a message indicating an unambiguous error.
 */
enum MessageType {
    NORMAL_MESSAGE,
    IMMEDIATE_MESSAGE,
    WARNING_MESSAGE,
    ERROR_MESSAGE,
    INFORMATION_MESSAGE
};

/**
 * An integer ID which identifies a displayed message in a particular
 * Inkscape::MessageStack
 *
 * @see Inkscape::MessageStack
 */
typedef unsigned long MessageId;

}

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
