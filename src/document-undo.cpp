#define __SP_DOCUMENT_UNDO_C__

/** \file
 * Undo/Redo stack implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2007  MenTaLguY <mental@rydia.net>
 * Copyright (C) 1999-2003 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 * Using the split document model gives sodipodi a very simple and clean
 * undo implementation. Whenever mutation occurs in the XML tree,
 * SPObject invokes one of the five corresponding handlers of its
 * container document. This writes down a generic description of the
 * given action, and appends it to the recent action list, kept by the
 * document. There will be as many action records as there are mutation
 * events, which are all kept and processed together in the undo
 * stack. Two methods exist to indicate that the given action is completed:
 *
 * \verbatim
   void sp_document_done (SPDocument *document);
   void sp_document_maybe_done (SPDocument *document, const unsigned char *key) \endverbatim
 *
 * Both move the recent action list into the undo stack and clear the
 * list afterwards.  While the first method does an unconditional push,
 * the second one first checks the key of the most recent stack entry. If
 * the keys are identical, the current action list is appended to the
 * existing stack entry, instead of pushing it onto its own.  This
 * behaviour can be used to collect multi-step actions (like winding the
 * Gtk spinbutton) from the UI into a single undoable step.
 *
 * For controls implemented by Sodipodi itself, implementing undo as a
 * single step is usually done in a more efficent way. Most controls have
 * the abstract model of grab, drag, release, and change user
 * action. During the grab phase, all modifications are done to the
 * SPObject directly - i.e. they do not change XML tree, and thus do not
 * generate undo actions either.  Only at the release phase (normally
 * associated with releasing the mousebutton), changes are written back
 * to the XML tree, thus generating only a single set of undo actions.
 * (Lauris Kaplinski)
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif



#if HAVE_STRING_H
#endif


#if HAVE_STDLIB_H
#endif

#include <string>
#include <cstring>
#include "xml/repr.h"
#include "document-private.h"
#include "inkscape.h"
#include "debug/event-tracker.h"
#include "debug/simple-event.h"
#include "debug/timestamp.h"
#include "event.h"


/*
 * Undo & redo
 */
/**
 * Set undo sensitivity.
 *
 * \note
 *   Since undo sensitivity needs to be nested, setting undo sensitivity
 *   should be done like this:
 *\verbatim
        bool saved = sp_document_get_undo_sensitive(document);
        sp_document_set_undo_sensitive(document, false);
        ... do stuff ...
        sp_document_set_undo_sensitive(document, saved);  \endverbatim
 */
void
sp_document_set_undo_sensitive (SPDocument *doc, bool sensitive)
{
	g_assert (doc != NULL);
	g_assert (doc->priv != NULL);

	if ( sensitive == doc->priv->sensitive )
		return;

	if (sensitive) {
		sp_repr_begin_transaction (doc->rdoc);
	} else {
		doc->priv->partial = sp_repr_coalesce_log (
			doc->priv->partial,
			sp_repr_commit_undoable (doc->rdoc)
		);
	}

	doc->priv->sensitive = sensitive;
}

/*TODO: Throughout the inkscape code tree set/get_undo_sensitive are used for
 * as is shown above.  Perhaps it makes sense to create new functions,
 * undo_ignore, and undo_recall to replace the start and end parts of the above.
 * The main complexity with this is that they have to nest, so you have to store
 * the saved bools in a stack.  Perhaps this is why the above solution is better.
 */

bool sp_document_get_undo_sensitive(SPDocument const *document) {
	g_assert(document != NULL);
	g_assert(document->priv != NULL);

	return document->priv->sensitive;
}

void
sp_document_done (SPDocument *doc, const unsigned int event_type, Glib::ustring event_description)
{
        sp_document_maybe_done (doc, NULL, event_type, event_description);
}

void
sp_document_reset_key (Inkscape::Application */*inkscape*/, SPDesktop */*desktop*/, GtkObject *base)
{
    SPDocument *doc = (SPDocument *) base;
    doc->actionkey = NULL;
}

namespace {

using Inkscape::Debug::Event;
using Inkscape::Debug::SimpleEvent;
using Inkscape::Util::share_static_string;
using Inkscape::Debug::timestamp;
using Inkscape::Verb;

typedef SimpleEvent<Event::INTERACTION> InteractionEvent;

class CommitEvent : public InteractionEvent {
public:

    CommitEvent(SPDocument *doc, const gchar *key, const unsigned int type)
    : InteractionEvent(share_static_string("commit"))
    {
        _addProperty(share_static_string("timestamp"), timestamp());
        gchar *serial = g_strdup_printf("%lu", doc->serial());
        _addProperty(share_static_string("document"), serial);
        g_free(serial);
        Verb *verb = Verb::get(type);
        if (verb) {
            _addProperty(share_static_string("context"), verb->get_id());
        }
        if (key) {
            _addProperty(share_static_string("merge-key"), key);
        }
    }
};

}

void
sp_document_maybe_done (SPDocument *doc, const gchar *key, const unsigned int event_type,
                        Glib::ustring event_description)
{
	g_assert (doc != NULL);
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

        Inkscape::Debug::EventTracker<CommitEvent> tracker(doc, key, event_type);

	doc->collectOrphans();

	sp_document_ensure_up_to_date (doc);

	sp_document_clear_redo (doc);

	Inkscape::XML::Event *log = sp_repr_coalesce_log (doc->priv->partial, sp_repr_commit_undoable (doc->rdoc));
	doc->priv->partial = NULL;

	if (!log) {
		sp_repr_begin_transaction (doc->rdoc);
		return;
	}

	if (key && doc->actionkey && !strcmp (key, doc->actionkey) && doc->priv->undo) {
                ((Inkscape::Event *)doc->priv->undo->data)->event =
                    sp_repr_coalesce_log (((Inkscape::Event *)doc->priv->undo->data)->event, log);
	} else {
                Inkscape::Event *event = new Inkscape::Event(log, event_type, event_description);
                doc->priv->undo = g_slist_prepend (doc->priv->undo, event);
		doc->priv->history_size++;
		doc->priv->undoStackObservers.notifyUndoCommitEvent(event);
	}

        if (doc->actionkey)
            g_free(doc->actionkey);
	doc->actionkey = key ? g_strdup(key) : NULL;

	doc->virgin = FALSE;
        doc->setModifiedSinceSave();

	sp_repr_begin_transaction (doc->rdoc);

  doc->priv->commit_signal.emit();
}

void
sp_document_cancel (SPDocument *doc)
{
	g_assert (doc != NULL);
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	sp_repr_rollback (doc->rdoc);

	if (doc->priv->partial) {
		sp_repr_undo_log (doc->priv->partial);
		sp_repr_free_log (doc->priv->partial);
		doc->priv->partial = NULL;
	}

	sp_repr_begin_transaction (doc->rdoc);
}

static void finish_incomplete_transaction(SPDocument &doc) {
	SPDocumentPrivate &priv=*doc.priv;
	Inkscape::XML::Event *log=sp_repr_commit_undoable(doc.rdoc);
	if (log || priv.partial) {
		g_warning ("Incomplete undo transaction:");
		priv.partial = sp_repr_coalesce_log(priv.partial, log);
		sp_repr_debug_print_log(priv.partial);
                Inkscape::Event *event = new Inkscape::Event(priv.partial);
		priv.undo = g_slist_prepend(priv.undo, event);
                priv.undoStackObservers.notifyUndoCommitEvent(event);
		priv.partial = NULL;
	}
}

gboolean
sp_document_undo (SPDocument *doc)
{
	using Inkscape::Debug::EventTracker;
	using Inkscape::Debug::SimpleEvent;

	gboolean ret;

	EventTracker<SimpleEvent<Inkscape::Debug::Event::DOCUMENT> > tracker("undo");

	g_assert (doc != NULL);
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	doc->priv->sensitive = FALSE;
        doc->priv->seeking = true;

	doc->actionkey = NULL;

	finish_incomplete_transaction(*doc);

	if (doc->priv->undo) {
		Inkscape::Event *log=(Inkscape::Event *)doc->priv->undo->data;
		doc->priv->undo = g_slist_remove (doc->priv->undo, log);
		sp_repr_undo_log (log->event);
		doc->priv->redo = g_slist_prepend (doc->priv->redo, log);

                doc->setModifiedSinceSave();
                doc->priv->undoStackObservers.notifyUndoEvent(log);

		ret = TRUE;
	} else {
		ret = FALSE;
	}

	sp_repr_begin_transaction (doc->rdoc);

	doc->priv->sensitive = TRUE;
        doc->priv->seeking = false;

	if (ret)
		inkscape_external_change();

	return ret;
}

gboolean
sp_document_redo (SPDocument *doc)
{
	using Inkscape::Debug::EventTracker;
	using Inkscape::Debug::SimpleEvent;

	gboolean ret;

	EventTracker<SimpleEvent<Inkscape::Debug::Event::DOCUMENT> > tracker("redo");

	g_assert (doc != NULL);
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	doc->priv->sensitive = FALSE;
        doc->priv->seeking = true;

	doc->actionkey = NULL;

	finish_incomplete_transaction(*doc);

	if (doc->priv->redo) {
		Inkscape::Event *log=(Inkscape::Event *)doc->priv->redo->data;
		doc->priv->redo = g_slist_remove (doc->priv->redo, log);
		sp_repr_replay_log (log->event);
		doc->priv->undo = g_slist_prepend (doc->priv->undo, log);

                doc->setModifiedSinceSave();
		doc->priv->undoStackObservers.notifyRedoEvent(log);

		ret = TRUE;
	} else {
		ret = FALSE;
	}

	sp_repr_begin_transaction (doc->rdoc);

	doc->priv->sensitive = TRUE;
        doc->priv->seeking = false;

	if (ret)
		inkscape_external_change();

	return ret;
}

void
sp_document_clear_undo (SPDocument *doc)
{
        if (doc->priv->undo)
                doc->priv->undoStackObservers.notifyClearUndoEvent();

	while (doc->priv->undo) {
		GSList *current;

		current = doc->priv->undo;
		doc->priv->undo = current->next;
		doc->priv->history_size--;

                delete ((Inkscape::Event *) current->data);
		g_slist_free_1 (current);
	}
}

void
sp_document_clear_redo (SPDocument *doc)
{
        if (doc->priv->redo)
                doc->priv->undoStackObservers.notifyClearRedoEvent();

	while (doc->priv->redo) {
		GSList *current;

		current = doc->priv->redo;
		doc->priv->redo = current->next;
		doc->priv->history_size--;

                delete ((Inkscape::Event *) current->data);
		g_slist_free_1 (current);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
