/*
 */

#include "xml/repr.h"
#include "sp-flowdiv.h"
#include "sp-string.h"
#include "document.h"

SPFlowdiv::SPFlowdiv() : SPItem() {
}

SPFlowdiv::~SPFlowdiv() {
}

void SPFlowdiv::release() {
	SPItem::release();
}

void SPFlowdiv::update(SPCtx *ctx, unsigned int flags) {
    SPItemCtx *ictx = reinterpret_cast<SPItemCtx *>(ctx);
    SPItemCtx cctx = *ictx;

    unsigned childflags = flags;
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        childflags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    childflags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList* l = NULL;
    for (SPObject *child = this->firstChild() ; child ; child = child->getNext() ) {
        sp_object_ref(child);
        l = g_slist_prepend(l, child);
    }

    l = g_slist_reverse(l);

    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);

        if (childflags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            if (SP_IS_ITEM(child)) {
                SPItem const &chi = *SP_ITEM(child);
                cctx.i2doc = chi.transform * ictx->i2doc;
                cctx.i2vp = chi.transform * ictx->i2vp;
                child->updateDisplay((SPCtx *)&cctx, childflags);
            } else {
                child->updateDisplay(ctx, childflags);
            }
        }

        sp_object_unref(child);
    }

    SPItem::update(ctx, flags);
}

void SPFlowdiv::modified(unsigned int flags) {
    SPItem::modified(flags);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList *l = NULL;
    for ( SPObject *child = this->firstChild() ; child ; child = child->getNext() ) {
        sp_object_ref(child);
        l = g_slist_prepend(l, child);
    }

    l = g_slist_reverse (l);

    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }

        sp_object_unref(child);
    }
}


void SPFlowdiv::build(SPDocument *doc, Inkscape::XML::Node *repr) {
	this->_requireSVGVersion(Inkscape::Version(1, 2));

	SPItem::build(doc, repr);
}

void SPFlowdiv::set(unsigned int key, const gchar* value) {
	SPItem::set(key, value);
}


Inkscape::XML::Node* SPFlowdiv::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ( flags & SP_OBJECT_WRITE_BUILD ) {
        if ( repr == NULL ) {
            repr = xml_doc->createElement("svg:flowDiv");
        }

        GSList *l = NULL;

        for (SPObject* child = this->firstChild() ; child ; child = child->getNext() ) {
            Inkscape::XML::Node* c_repr = NULL;

            if ( SP_IS_FLOWTSPAN (child) ) {
                c_repr = child->updateRepr(xml_doc, NULL, flags);
            } else if ( SP_IS_FLOWPARA(child) ) {
                c_repr = child->updateRepr(xml_doc, NULL, flags);
            } else if ( SP_IS_STRING(child) ) {
                c_repr = xml_doc->createTextNode(SP_STRING(child)->string.c_str());
            }

            if ( c_repr ) {
                l = g_slist_prepend (l, c_repr);
            }
        }

        while ( l ) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove(l, l->data);
        }
    } else {
        for ( SPObject* child = this->firstChild() ; child ; child = child->getNext() ) {
            if ( SP_IS_FLOWTSPAN (child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_FLOWPARA(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_STRING(child) ) {
                child->getRepr()->setContent(SP_STRING(child)->string.c_str());
            }
        }
    }

    SPItem::write(xml_doc, repr, flags);

    return repr;
}


/*
 *
 */

SPFlowtspan::SPFlowtspan() : SPItem() {
}

SPFlowtspan::~SPFlowtspan() {
}

void SPFlowtspan::release() {
	SPItem::release();
}

void SPFlowtspan::update(SPCtx *ctx, unsigned int flags) {
    SPItemCtx *ictx = reinterpret_cast<SPItemCtx *>(ctx);
    SPItemCtx cctx = *ictx;

    unsigned childflags = flags;
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        childflags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    childflags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList* l = NULL;
    for ( SPObject *child = this->firstChild() ; child ; child = child->getNext() ) {
        sp_object_ref(child);
        l = g_slist_prepend(l, child);
    }

    l = g_slist_reverse (l);

    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);

        if (childflags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            if (SP_IS_ITEM(child)) {
                SPItem const &chi = *SP_ITEM(child);
                cctx.i2doc = chi.transform * ictx->i2doc;
                cctx.i2vp = chi.transform * ictx->i2vp;
                child->updateDisplay((SPCtx *)&cctx, childflags);
            } else {
                child->updateDisplay(ctx, childflags);
            }
        }

        sp_object_unref(child);
    }

    SPItem::update(ctx, flags);
}

void SPFlowtspan::modified(unsigned int flags) {
    SPItem::modified(flags);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList *l = NULL;
    for ( SPObject *child = this->firstChild() ; child ; child = child->getNext() ) {
        sp_object_ref(child);
        l = g_slist_prepend(l, child);
    }

    l = g_slist_reverse (l);

    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }

        sp_object_unref(child);
    }
}


void SPFlowtspan::build(SPDocument *doc, Inkscape::XML::Node *repr) {
	SPItem::build(doc, repr);
}

void SPFlowtspan::set(unsigned int key, const gchar* value) {
	SPItem::set(key, value);
}

Inkscape::XML::Node *SPFlowtspan::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ( flags&SP_OBJECT_WRITE_BUILD ) {
        if ( repr == NULL ) {
            repr = xml_doc->createElement("svg:flowSpan");
        }

        GSList *l = NULL;

        for ( SPObject* child = this->firstChild() ; child ; child = child->getNext() ) {
            Inkscape::XML::Node* c_repr = NULL;

            if ( SP_IS_FLOWTSPAN(child) ) {
                c_repr = child->updateRepr(xml_doc, NULL, flags);
            } else if ( SP_IS_FLOWPARA(child) ) {
                c_repr = child->updateRepr(xml_doc, NULL, flags);
            } else if ( SP_IS_STRING(child) ) {
                c_repr = xml_doc->createTextNode(SP_STRING(child)->string.c_str());
            }

            if ( c_repr ) {
                l = g_slist_prepend(l, c_repr);
            }
        }

        while ( l ) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove(l, l->data);
        }
    } else {
        for ( SPObject* child = this->firstChild() ; child ; child = child->getNext() ) {
            if ( SP_IS_FLOWTSPAN(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_FLOWPARA(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_STRING(child) ) {
                child->getRepr()->setContent(SP_STRING(child)->string.c_str());
            }
        }
    }

    SPItem::write(xml_doc, repr, flags);

    return repr;
}


/*
 *
 */
SPFlowpara::SPFlowpara() : SPItem() {
}

SPFlowpara::~SPFlowpara() {
}

void SPFlowpara::release() {
	SPItem::release();
}

void SPFlowpara::update(SPCtx *ctx, unsigned int flags) {
    SPItemCtx *ictx = reinterpret_cast<SPItemCtx *>(ctx);
    SPItemCtx cctx = *ictx;

    SPItem::update(ctx, flags);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList* l = NULL;
    for ( SPObject *child = this->firstChild() ; child ; child = child->getNext() ) {
        sp_object_ref(child);
        l = g_slist_prepend(l, child);
    }

    l = g_slist_reverse (l);

    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);

        if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            if (SP_IS_ITEM(child)) {
                SPItem const &chi = *SP_ITEM(child);
                cctx.i2doc = chi.transform * ictx->i2doc;
                cctx.i2vp = chi.transform * ictx->i2vp;
                child->updateDisplay((SPCtx *)&cctx, flags);
            } else {
                child->updateDisplay(ctx, flags);
            }
        }

        sp_object_unref(child);
    }
}

void SPFlowpara::modified(unsigned int flags) {
    SPItem::modified(flags);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList *l = NULL;
    for ( SPObject *child = this->firstChild() ; child ; child = child->getNext() ) {
        sp_object_ref(child);
        l = g_slist_prepend(l, child);
    }

    l = g_slist_reverse (l);

    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }

        sp_object_unref(child);
    }
}


void SPFlowpara::build(SPDocument *doc, Inkscape::XML::Node *repr) {
	SPItem::build(doc, repr);
}

void SPFlowpara::set(unsigned int key, const gchar* value) {
	SPItem::set(key, value);
}

Inkscape::XML::Node *SPFlowpara::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ( flags&SP_OBJECT_WRITE_BUILD ) {
        if ( repr == NULL ) {
        	repr = xml_doc->createElement("svg:flowPara");
        }

        GSList *l = NULL;

        for ( SPObject* child = this->firstChild() ; child ; child = child->getNext() ) {
            Inkscape::XML::Node* c_repr = NULL;

            if ( SP_IS_FLOWTSPAN(child) ) {
                c_repr = child->updateRepr(xml_doc, NULL, flags);
            } else if ( SP_IS_FLOWPARA(child) ) {
                c_repr = child->updateRepr(xml_doc, NULL, flags);
            } else if ( SP_IS_STRING(child) ) {
                c_repr = xml_doc->createTextNode(SP_STRING(child)->string.c_str());
            }

            if ( c_repr ) {
                l = g_slist_prepend(l, c_repr);
            }
        }

        while ( l ) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove(l, l->data);
        }
    } else {
        for ( SPObject* child = this->firstChild() ; child ; child = child->getNext() ) {
            if ( SP_IS_FLOWTSPAN(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_FLOWPARA(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_STRING(child) ) {
                child->getRepr()->setContent(SP_STRING(child)->string.c_str());
            }
        }
    }

    SPItem::write(xml_doc, repr, flags);

    return repr;
}


/*
 *
 */

SPFlowline::SPFlowline() : SPObject() {
}

SPFlowline::~SPFlowline() {
}

void SPFlowline::release() {
	SPObject::release();
}

void SPFlowline::modified(unsigned int flags) {
	SPObject::modified(flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) {
		flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	}

	flags &= SP_OBJECT_MODIFIED_CASCADE;
}

Inkscape::XML::Node *SPFlowline::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ( flags & SP_OBJECT_WRITE_BUILD ) {
        if ( repr == NULL ) {
            repr = xml_doc->createElement("svg:flowLine");
        }
    }

    SPObject::write(xml_doc, repr, flags);

    return repr;
}


/*
 *
 */

SPFlowregionbreak::SPFlowregionbreak() : SPObject() {
}

SPFlowregionbreak::~SPFlowregionbreak() {
}

void SPFlowregionbreak::release() {
	SPObject::release();
}

void SPFlowregionbreak::modified(unsigned int flags) {
	SPObject::modified(flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) {
		flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	}

	flags &= SP_OBJECT_MODIFIED_CASCADE;
}

Inkscape::XML::Node *SPFlowregionbreak::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ( flags & SP_OBJECT_WRITE_BUILD ) {
        if ( repr == NULL ) {
            repr = xml_doc->createElement("svg:flowLine");
        }
    }

    SPObject::write(xml_doc, repr, flags);

    return repr;
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
