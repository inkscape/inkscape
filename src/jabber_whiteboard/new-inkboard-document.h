/**
 * new-inkboard-document.h
 * Functions to create new Inkboard documents, based off of sp_document_new /
 * sp_file_new
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __INKSCAPE_WHITEBOARD_NEW_DOCUMENT_H__
#define __INKSCAPE_WHITEBOARD_NEW_DOCUMENT_H__

#include <glibmm.h>

class SPDocument;
class SPDesktop;

namespace Inkscape {

namespace Whiteboard {

SPDocument* makeInkboardDocument(int code, gchar const* rootname, State::SessionType type, Glib::ustring const& to);
SPDesktop* makeInkboardDesktop(SPDocument* doc);

}

}

#endif
