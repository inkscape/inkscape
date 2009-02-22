#ifndef __INKSCAPE_WHITEBOARD_MESSAGE_VERIFIER_H__
#define __INKSCAPE_WHITEBOARD_MESSAGE_VERIFIER_H__

/**
 * Inkscape::Whiteboard::MessageVerifier -- performs basic XMPP-related
 * validity checks on incoming messages
 *
 * Authors:
 * David Yip <yipdw@alumni.rose-hulman.edu>
 *
 * Copyright (c) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

namespace Inkscape {

namespace Whiteboard {

 /**
  * The class has been written, but I forgot to commit that file to SVN,
  * and the only other copy I have is on a computer that I do not currently
  * have access to.  So, for now, this is just a placeholder with enums
  * to get things working.
  */

enum MessageValidityTestResult {
	RESULT_VALID,
	RESULT_INVALID
};

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
