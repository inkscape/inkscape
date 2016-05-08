/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information
 */

#include <ui/view/view.h>
#include <desktop.h>
#include <helper/action.h>
#include <helper/action-context.h>
#include <selection.h>
#include <verbs.h>
#include <inkscape.h>
#include <document.h>

#include <glibmm/i18n.h>

#include "main-cmdlineact.h"

namespace Inkscape {

std::list <CmdLineAction *> CmdLineAction::_list;

CmdLineAction::CmdLineAction (bool isVerb, gchar const * arg) : _isVerb(isVerb), _arg(NULL) {
	if (arg != NULL) {
		_arg = g_strdup(arg);
	}

	_list.insert(_list.end(), this);

	return;
}

CmdLineAction::~CmdLineAction () {
	if (_arg != NULL) {
		g_free(_arg);
	}
}

void
CmdLineAction::doIt (ActionContext const & context) {
	//printf("Doing: %s\n", _arg);
	if (_isVerb) {
		Inkscape::Verb * verb = Inkscape::Verb::getbyid(_arg);
		if (verb == NULL) {
			printf(_("Unable to find verb ID '%s' specified on the command line.\n"), _arg);
			return;
		}
		SPAction * action = verb->get_action(context);
		sp_action_perform(action, NULL);
	} else {
		if (context.getDocument() == NULL || context.getSelection() == NULL) { return; }

		SPDocument * doc = context.getDocument();
		SPObject * obj = doc->getObjectById(_arg);
		if (obj == NULL) {
			printf(_("Unable to find node ID: '%s'\n"), _arg);
			return;
		}

		Inkscape::Selection * selection = context.getSelection();
		selection->add(obj, false);
	}
	return;
}

bool
CmdLineAction::doList (ActionContext const & context) {
  bool hasActions = !_list.empty();
	for (std::list<CmdLineAction *>::iterator i = _list.begin();
			i != _list.end(); ++i) {
		CmdLineAction * entry = *i;
		entry->doIt(context);
	}
  return hasActions;
}

bool
CmdLineAction::idle (void) {
	std::list<SPDesktop *> desktops;
	INKSCAPE.get_all_desktops(desktops);

	// We're going to assume one desktop per document, because no one
	// should have had time to make more at this point.
	for (std::list<SPDesktop *>::iterator i = desktops.begin();
			i != desktops.end(); ++i) {
		SPDesktop * desktop = *i;
		//Inkscape::UI::View::View * view = dynamic_cast<Inkscape::UI::View::View *>(desktop);    
		doList(ActionContext(desktop));
	}
	return false;
}

} // Inkscape

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
