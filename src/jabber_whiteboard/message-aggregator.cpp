/**
 * Aggregates individual serialized XML::Events into larger packages 
 * for more efficient delivery
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm.h>
#include "jabber_whiteboard/message-aggregator.h"

namespace Inkscape {

namespace Whiteboard {

bool
MessageAggregator::addOne(Glib::ustring const& msg, Glib::ustring& buf)
{
	// 1.  If msg.bytes() > maximum size and the buffer is clear,
	// then we have to send an oversize packet -- 
	// we won't be able to deliver the message any other way.
	// Add it to the buffer and return true.  Any further attempt to 
	// aggregate a message will be handled by condition #2.
	if (msg.bytes() > MessageAggregator::MAX_SIZE && buf.empty()) {
		buf += msg;
		return true;
	}

	// 2.  If msg.bytes() + buf.bytes() > maximum size, return false.
	// The user of this class is responsible for retrieving the aggregated message,
	// doing something with it, clearing the buffer, and trying again.
	// Otherwise, append the message to the buffer and return true.
	if (msg.bytes() + buf.bytes() > MessageAggregator::MAX_SIZE) {
		return false;
	} else {
		buf += msg;
		return true;
	}
}

bool
MessageAggregator::addOne(Glib::ustring const& msg)
{
	return this->addOne(msg, this->_buf);
}

}

}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
