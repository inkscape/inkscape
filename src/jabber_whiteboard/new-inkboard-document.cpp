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

#include <glib.h>
#include <glibmm.h>
#include <glibmm/i18n.h>

#include "document.h"
#include "document-private.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "interface.h"
#include "sp-namedview.h"
#include "desktop-handles.h"

#include "gc-anchored.h"

#include "ui/view/view-widget.h"

#include "application/application.h"
#include "application/editor.h"

#include "util/ucompose.hpp"

#include "xml/node.h"
#include "xml/repr.h"

#include "jabber_whiteboard/inkboard-document.h"
#include "jabber_whiteboard/new-inkboard-document.h"

namespace Inkscape {

namespace Whiteboard {

SPDocument*
makeInkboardDocument(int code, gchar const* rootname, SessionType type, Glib::ustring const& to)
{
	SPDocument* doc;
	InkboardDocument* rdoc = new InkboardDocument(g_quark_from_static_string("xml"), type, to);
	rdoc->setAttribute("version", "1.0");
	rdoc->setAttribute("standalone", "no");
	XML::Node *comment = sp_repr_new_comment(" Created with Inkscape (http://www.inkscape.org/) ");
	rdoc->appendChild(comment);
	GC::release(comment);

	XML::Node* root = sp_repr_new(rootname);
	rdoc->appendChild(root);
	GC::release(root);

	Glib::ustring name = String::ucompose(_("Inkboard session (%1 to %2)"), SessionManager::instance().getClient().getJid(), to);

	doc = sp_document_create(rdoc, NULL, NULL, name.c_str(), TRUE);
	return doc;
}

// TODO: When the switchover to the new GUI is complete, this function should go away
// and be replaced with a call to Inkscape::NSApplication::Editor::createDesktop.  
// It currently only exists to correctly mimic the desktop creation functionality
// in file.cpp.
//
// \see sp_file_new
SPDesktop*
makeInkboardDesktop(SPDocument* doc)
{
	SPDesktop* dt;

	if (NSApplication::Application::getNewGui()) {
		dt = NSApplication::Editor::createDesktop(doc);
	} else {
        SPViewWidget *dtw = sp_desktop_widget_new(sp_document_namedview(doc, NULL));
        g_return_val_if_fail(dtw != NULL, NULL);
        sp_document_unref(doc);

        sp_create_window(dtw, TRUE);
        dt = static_cast<SPDesktop*>(dtw->view);
        sp_namedview_window_from_document(dt);
	}

	return dt;
}

}

}
