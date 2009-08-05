#ifndef __SP_DOCUMENT_PRIVATE_H__
#define __SP_DOCUMENT_PRIVATE_H__

/*
 * Seldom needed document data
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <map>
#include <sigc++/sigc++.h>
#include "xml/event-fns.h"
#include "sp-defs.h"
#include "sp-root.h"
#include "document.h"

#include "composite-undo-stack-observer.h"

#include "event-log.h"

// XXX only for testing!
#include "console-output-undo-observer.h"

#define SP_DOCUMENT_DEFS(d) ((SPObject *) SP_ROOT (SP_DOCUMENT_ROOT (d))->defs)

namespace Inkscape {
namespace XML {
class Event;
}
}


struct DocumentPrivate {
	typedef std::map<GQuark, Document::IDChangedSignal> IDChangedSignalMap;
	typedef std::map<GQuark, Document::ResourcesChangedSignal> ResourcesChangedSignalMap;

	GHashTable *iddef;	/**< Dictionary of id -> SPObject mappings */
	GHashTable *reprdef;   /**< Dictionary of Inkscape::XML::Node -> SPObject mappings */

	unsigned long serial;

	/** Dictionary of signals for id changes */
	IDChangedSignalMap id_changed_signals;

	/* Resources */
	/* It is GHashTable of GSLists */
	GHashTable *resources;
	ResourcesChangedSignalMap resources_changed_signals;

	Document::ModifiedSignal modified_signal;
	Document::URISetSignal uri_set_signal;
	Document::ResizedSignal resized_signal;
	Document::ReconstructionStart _reconstruction_start_signal;
	Document::ReconstructionFinish  _reconstruction_finish_signal;
  Document::CommitSignal commit_signal;

	/* Undo/Redo state */
	bool sensitive: true; /* If we save actions to undo stack */
	Inkscape::XML::Event * partial; /* partial undo log when interrupted */
	int history_size;
	GSList * undo; /* Undo stack of reprs */
	GSList * redo; /* Redo stack of reprs */

	/* Undo listener */
	Inkscape::CompositeUndoStackObserver undoStackObservers;

	// XXX only for testing!
	Inkscape::ConsoleOutputUndoObserver console_output_undo_observer;

	bool seeking;
};

#endif
