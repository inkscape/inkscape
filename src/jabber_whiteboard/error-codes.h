/**
 * Whiteboard session manager
 * Error codes
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_JABBER_ERROR_CODES_H__
#define __WHITEBOARD_JABBER_ERROR_CODES_H__

namespace Inkscape {

namespace Whiteboard {

namespace ErrorCodes {

static unsigned int const SERVER_CONNECT_FAILED = 502;
static unsigned int const CHAT_HANDLE_IN_USE = 409;

}

}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
