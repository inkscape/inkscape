/**
 * Whiteboard session file object
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_SESSION_FILE_H__
#define __WHITEBOARD_SESSION_FILE_H__

#include <glibmm.h>
#include "util/list-container.h"

struct SPDesktop;

namespace Inkscape {

namespace Whiteboard {

typedef Glib::RefPtr< Glib::IOChannel > SessionFilePtr;
typedef Util::ListContainer< Glib::ustring const > SessionQueue;

class SessionFile {
public:
	SessionFile(Glib::ustring const& filename, bool reading, bool compress);
	~SessionFile();

	gint64 nextMessageFrom(gint64 from, Glib::ustring& buf);

	void addMessage(Glib::ustring const& message);
	void commit();
	void close();

	Glib::ustring const& filename()
	{
		return this->_filename;
	}

	bool isReadOnly()
	{
		return this->_reading;
	}

	bool eof()
	{
		return this->_ateof;
	}

private:
	SessionQueue changes;
	SessionFilePtr fptr;
	Glib::ustring _filename;

	bool _ateof;
	bool _compress;
	bool _reading;
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
