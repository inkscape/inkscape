#ifndef SEEN_SP_DOCUMENT_PRIVATE_H
#define SEEN_SP_DOCUMENT_PRIVATE_H

/*
 * Seldom needed document data
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <map>
#include <stddef.h>
#include <sigc++/sigc++.h>
#include "xml/event-fns.h"
#include "sp-defs.h"
#include "sp-root.h"
#include "document.h"

#include "composite-undo-stack-observer.h"


// XXX only for testing!
#include "console-output-undo-observer.h"

namespace Inkscape {
namespace XML {
class Event;
}
}


struct SPDocumentPrivate {
	typedef std::map<GQuark, SPDocument::IDChangedSignal> IDChangedSignalMap;
	typedef std::map<GQuark, SPDocument::ResourcesChangedSignal> ResourcesChangedSignalMap;

	GHashTable *iddef;	/**< Dictionary of id -> SPObject mappings */
	GHashTable *reprdef;   /**< Dictionary of Inkscape::XML::Node -> SPObject mappings */

	unsigned long serial;

	/** Dictionary of signals for id changes */
	IDChangedSignalMap id_changed_signals;

	/* Resources */
	/* It is GHashTable of GSLists */
	GHashTable *resources;
	ResourcesChangedSignalMap resources_changed_signals;

        sigc::signal<void> destroySignal;
	SPDocument::ModifiedSignal modified_signal;
	SPDocument::URISetSignal uri_set_signal;
	SPDocument::ResizedSignal resized_signal;
	SPDocument::ReconstructionStart _reconstruction_start_signal;
	SPDocument::ReconstructionFinish  _reconstruction_finish_signal;
  SPDocument::CommitSignal commit_signal;

	/* Undo/Redo state */
	bool sensitive; /* If we save actions to undo stack */
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

#endif // SEEN_SP_DOCUMENT_PRIVATE_H
