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

#include <glibmm.h>
#include <glibmm/i18n.h>

#include "util/list-container.h"

#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/node-utilities.h"
#include "jabber_whiteboard/typedefs.h"

#include "jabber_whiteboard/session-file.h"


namespace Inkscape {

namespace Whiteboard {

SessionFile::SessionFile(Glib::ustring const& filename, bool reading, bool compress) : _filename(filename), _compress(compress), _reading(reading)
{
	try {

		if (!reading) {
			this->fptr = Glib::IOChannel::create_from_file(filename, "w+");
		} else {
			this->fptr = Glib::IOChannel::create_from_file(filename, "r");
		}
		this->_ateof = false;
	} catch (Glib::FileError) {
		throw;
	}
}

SessionFile::~SessionFile() 
{
	if (!this->_reading) {
		this->commit();
	}
	this->close();
}

gint64
SessionFile::nextMessageFrom(gint64 from, Glib::ustring& buf)
{
	try {
		Glib::ustring line;
		Glib::IOStatus st;
		Node part;

		gint64 accum = from;
		buf = "";
		this->fptr->seek(accum, Glib::SEEK_TYPE_SET);

		while(part.tag != MESSAGE_COMMIT) {
			st = this->fptr->read_line(line);
			if (st == Glib::IO_STATUS_EOF) {
				break;
			} else {
				accum += line.bytes();
				this->fptr->seek(accum);
				MessageUtilities::getFirstMessageTag(part, line);
				buf += line;
				line.clear();
			}
		} 
		
		if (st == Glib::IO_STATUS_NORMAL) {
			// reset eof flag if successful
			this->_ateof = false;
			return from + buf.bytes();
		} else {
			if (st == Glib::IO_STATUS_EOF) {
				this->_ateof = true;
			}
			return from;
		}
	} catch (Glib::IOChannelError e) {
		g_warning("Could not read next message due to I/O error (error: %s)!", e.what().data());
	} catch (Glib::ConvertError e) {
		g_warning("Could not read next message due to charset conversion error (error: %s)!", e.what().data());
	}

	return from;
}

void
SessionFile::addMessage(Glib::ustring const& message)
{
	Glib::ustring msg = message;
	this->changes.push_back(msg);
}

void
SessionFile::commit()
{
	if (!this->_reading) {
		SessionQueue::iterator i = changes.begin();
		for(; i != changes.end(); i++) {
			try {
				fptr->write(*i);
				changes.erase(i);
			} catch (Glib::IOChannelError e) {
				g_warning("Caught I/O exception (error string: %s) while committing change to session file %s.  Attempting to commit remaining changes; session file will be inconsistent with whiteboard session history.", e.what().c_str(), this->_filename.c_str());
			} catch (Glib::ConvertError e) {
				g_warning("Caught character set conversion error (error string: %s) while committing change to session file %s.  Attempting to commit remaining changes; session file will be inconsistent with whiteboard session history.", e.what().c_str(), this->_filename.c_str());
			}
		}
		fptr->write("\n");
	}
}

void
SessionFile::close()
{
	fptr->close(true);
}

}

}

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
