/*
 * SVG <switch> implementation
 *
 * Authors:
 *   Andrius R. <knutux@gmail.com>
 *   MenTaLguY  <mental@rydia.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>

#include "sp-switch.h"
#include "display/drawing-group.h"
#include "conditions.h"

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#include "sp-factory.h"

namespace {
	SPObject* createSwitch() {
		return new SPSwitch();
	}

	bool switchRegistered = SPFactory::instance().registerObject("svg:switch", createSwitch);
}

SPSwitch::SPSwitch() : SPGroup() {
    this->_cached_item = 0;
}

SPSwitch::~SPSwitch() {
    _releaseLastItem(_cached_item);
}

SPObject *SPSwitch::_evaluateFirst() {
    SPObject *first = 0;

    for (SPObject *child = this->firstChild() ; child && !first ; child = child->getNext() ) {
        if (SP_IS_ITEM(child) && sp_item_evaluate(SP_ITEM(child))) {
        	first = child;
        }
    }

    return first;
}

GSList *SPSwitch::_childList(bool add_ref, SPObject::Action action) {
    if ( action != SPObject::ActionGeneral ) {
        return this->childList(add_ref, action);
    }

    SPObject *child = _evaluateFirst();
    if (NULL == child)
        return NULL;

    if (add_ref) {
        //g_object_ref (G_OBJECT (child));
    	sp_object_ref(child);
    }

    return g_slist_prepend (NULL, child);
}

const char *SPSwitch::displayName() const {
    return _("Conditional Group");
}

gchar *SPSwitch::description() const {
    gint len = this->getItemCount();
    return g_strdup_printf(
        ngettext(_("of <b>%d</b> object"), _("of <b>%d</b> objects"), len), len);
}

void SPSwitch::child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref) {
    SPGroup::child_added(child, ref);

    this->_reevaluate(true);
}

void SPSwitch::remove_child(Inkscape::XML::Node *child) {
    SPGroup::remove_child(child);

    this->_reevaluate();
}

void SPSwitch::order_changed (Inkscape::XML::Node *child, Inkscape::XML::Node *old_ref, Inkscape::XML::Node *new_ref)
{
    SPGroup::order_changed(child, old_ref, new_ref);

	this->_reevaluate();
}

void SPSwitch::_reevaluate(bool /*add_to_drawing*/) {
    SPObject *evaluated_child = _evaluateFirst();
    if (!evaluated_child || _cached_item == evaluated_child) {
        return;
    }

    _releaseLastItem(_cached_item);

    for ( GSList *l = _childList(false, SPObject::ActionShow);
            NULL != l ; l = g_slist_remove (l, l->data))
    {
        SPObject *o = SP_OBJECT (l->data);
        if ( !SP_IS_ITEM (o) ) {
            continue;
        }

        SPItem * child = SP_ITEM(o);
        child->setEvaluated(o == evaluated_child);
    }

    _cached_item = evaluated_child;
    _release_connection = evaluated_child->connectRelease(sigc::bind(sigc::ptr_fun(&SPSwitch::_releaseItem), this));

    this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
}

void SPSwitch::_releaseItem(SPObject *obj, SPSwitch *selection)
{
    selection->_releaseLastItem(obj);
}

void SPSwitch::_releaseLastItem(SPObject *obj)
{
    if (NULL == this->_cached_item || this->_cached_item != obj)
        return;

    this->_release_connection.disconnect();
    this->_cached_item = NULL;
}

void SPSwitch::_showChildren (Inkscape::Drawing &drawing, Inkscape::DrawingItem *ai, unsigned int key, unsigned int flags) {
    SPObject *evaluated_child = this->_evaluateFirst();

    GSList *l = this->_childList(false, SPObject::ActionShow);

    while (l) {
        SPObject *o = SP_OBJECT (l->data);

        if (SP_IS_ITEM (o)) {
            SPItem * child = SP_ITEM(o);
            child->setEvaluated(o == evaluated_child);
            Inkscape::DrawingItem *ac = child->invoke_show (drawing, key, flags);

            if (ac) {
                ai->appendChild(ac);
            }
        }

        l = g_slist_remove (l, o);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
