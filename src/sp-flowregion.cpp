/*
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <glibmm/i18n.h>

#include <xml/repr.h>
#include "display/curve.h"
#include "sp-shape.h"
#include "sp-text.h"
#include "sp-use.h"
#include "style.h"
#include "document.h"
#include "sp-title.h"
#include "sp-desc.h"

#include "sp-flowregion.h"

#include "display/canvas-bpath.h"

#include "livarot/Path.h"
#include "livarot/Shape.h"

#include "sp-factory.h"

namespace {
	SPObject* createFlowregion() {
		return new SPFlowregion();
	}

	SPObject* createFlowregionExclude() {
		return new SPFlowregionExclude();
	}

	bool flowregionRegistered = SPFactory::instance().registerObject("svg:flowRegion", createFlowregion);
	bool flowregionExcludeRegistered = SPFactory::instance().registerObject("svg:flowRegionExclude", createFlowregionExclude);
}

static void         GetDest(SPObject* child,Shape **computed);


SPFlowregion::SPFlowregion() : SPItem() {
	//new (&this->computed) std::vector<Shape*>;
}

SPFlowregion::~SPFlowregion() {
	for (std::vector<Shape*>::iterator it = this->computed.begin() ; it != this->computed.end() ; ++it) {
        delete *it;
	}

    //this->computed.~vector<Shape*>();
}

void SPFlowregion::child_added(Inkscape::XML::Node *child, Inkscape::XML::Node *ref) {
	SPItem::child_added(child, ref);

	this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/* fixme: hide (Lauris) */

void SPFlowregion::remove_child(Inkscape::XML::Node * child) {
	SPItem::remove_child(child);

	this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}


void SPFlowregion::update(SPCtx *ctx, unsigned int flags) {
    SPItemCtx *ictx = reinterpret_cast<SPItemCtx *>(ctx);
    SPItemCtx cctx = *ictx;

    unsigned childflags = flags;
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        childflags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    childflags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList *l = NULL;

    for ( SPObject *child = this->firstChild() ; child ; child = child->getNext() ) {
        sp_object_ref(child);
        l = g_slist_prepend(l, child);
    }

    l = g_slist_reverse(l);

    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);

        if (childflags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            if (SP_IS_ITEM (child)) {
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

    this->UpdateComputed();
}

void SPFlowregion::UpdateComputed(void)
{
    for (std::vector<Shape*>::iterator it = computed.begin() ; it != computed.end() ; ++it) {
        delete *it;
    }
    computed.clear();

    for (SPObject* child = firstChild() ; child ; child = child->getNext() ) {
        Shape *shape = 0;
        GetDest(child, &shape);
        computed.push_back(shape);
    }
}

void SPFlowregion::modified(guint flags) {
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList *l = NULL;

    for ( SPObject *child = this->firstChild() ; child ; child = child->getNext() ) {
        sp_object_ref(child);
        l = g_slist_prepend(l, child);
    }

    l = g_slist_reverse(l);

    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }

        sp_object_unref(child);
    }
}

Inkscape::XML::Node *SPFlowregion::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if (flags & SP_OBJECT_WRITE_BUILD) {
        if ( repr == NULL ) {
            repr = xml_doc->createElement("svg:flowRegion");
        }

        GSList *l = NULL;
        for ( SPObject *child = this->firstChild() ; child; child = child->getNext() ) {
            if ( !SP_IS_TITLE(child) && !SP_IS_DESC(child) ) {
                Inkscape::XML::Node *crepr = child->updateRepr(xml_doc, NULL, flags);

                if (crepr) {
                    l = g_slist_prepend(l, crepr);
                }
            }
        }

        while (l) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove(l, l->data);
        }

    } else {
        for ( SPObject *child = this->firstChild() ; child; child = child->getNext() ) {
            if ( !SP_IS_TITLE(child) && !SP_IS_DESC(child) ) {
                child->updateRepr(flags);
            }
        }
    }

    SPItem::write(xml_doc, repr, flags);

    this->UpdateComputed();  // copied from update(), see LP Bug 1339305

    return repr;
}

const char* SPFlowregion::displayName() const {
	// TRANSLATORS: "Flow region" is an area where text is allowed to flow
	return _("Flow Region");
}

SPFlowregionExclude::SPFlowregionExclude() : SPItem() {
	this->computed = NULL;
}

SPFlowregionExclude::~SPFlowregionExclude() {
    if (this->computed) {
        delete this->computed;
        this->computed = NULL;
    }
}

void SPFlowregionExclude::child_added(Inkscape::XML::Node *child, Inkscape::XML::Node *ref) {
	SPItem::child_added(child, ref);

	this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/* fixme: hide (Lauris) */

void SPFlowregionExclude::remove_child(Inkscape::XML::Node * child) {
	SPItem::remove_child(child);

	this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}


void SPFlowregionExclude::update(SPCtx *ctx, unsigned int flags) {
    SPItemCtx *ictx = reinterpret_cast<SPItemCtx *>(ctx);
    SPItemCtx cctx = *ictx;

    SPItem::update(ctx, flags);

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

        if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            if (SP_IS_ITEM (child)) {
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

    this->UpdateComputed();
}


void SPFlowregionExclude::UpdateComputed(void)
{
    if (computed) {
        delete computed;
        computed = NULL;
    }

    for ( SPObject* child = firstChild() ; child ; child = child->getNext() ) {
        GetDest(child, &computed);
    }
}

void SPFlowregionExclude::modified(guint flags) {
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

Inkscape::XML::Node *SPFlowregionExclude::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if (flags & SP_OBJECT_WRITE_BUILD) {
        if ( repr == NULL ) {
            repr = xml_doc->createElement("svg:flowRegionExclude");
        }

        GSList *l = NULL;

        for ( SPObject *child = this->firstChild() ; child; child = child->getNext() ) {
            Inkscape::XML::Node *crepr = child->updateRepr(xml_doc, NULL, flags);

            if (crepr) {
                l = g_slist_prepend(l, crepr);
            }
        }

        while (l) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove(l, l->data);
        }

    } else {
        for ( SPObject *child = this->firstChild() ; child; child = child->getNext() ) {
            child->updateRepr(flags);
        }
    }

    SPItem::write(xml_doc, repr, flags);

    return repr;
}

const char* SPFlowregionExclude::displayName() const {
	/* TRANSLATORS: A region "cut out of" a flow region; text is not allowed to flow inside the
	 * flow excluded region.  flowRegionExclude in SVG 1.2: see
	 * http://www.w3.org/TR/2004/WD-SVG12-20041027/flow.html#flowRegion-elem and
	 * http://www.w3.org/TR/2004/WD-SVG12-20041027/flow.html#flowRegionExclude-elem. */
	return _("Flow Excluded Region");
}

static void         UnionShape(Shape **base_shape, Shape const *add_shape)
{
    if (*base_shape == NULL)
        *base_shape = new Shape;
	if ( (*base_shape)->hasEdges() == false ) {
		(*base_shape)->Copy(const_cast<Shape*>(add_shape));
	} else if ( add_shape->hasEdges() ) {
		Shape* temp=new Shape;
		temp->Booleen(const_cast<Shape*>(add_shape), *base_shape, bool_op_union);
		delete *base_shape;
		*base_shape = temp;
	}
}

static void         GetDest(SPObject* child,Shape **computed)
{
	if ( child == NULL ) return;

	SPCurve *curve=NULL;
	Geom::Affine tr_mat;

	SPObject* u_child=child;
	if ( SP_IS_USE(u_child) ) {
		u_child=SP_USE(u_child)->child;
		tr_mat = SP_ITEM(u_child)->getRelativeTransform(child->parent);
	} else {
		tr_mat = SP_ITEM(u_child)->transform;
	}
	if ( SP_IS_SHAPE (u_child) ) {
        if (!(SP_SHAPE(u_child)->_curve))
            SP_SHAPE (u_child)->set_shape ();
		curve = SP_SHAPE (u_child)->getCurve ();
	} else if ( SP_IS_TEXT (u_child) ) {
	curve = SP_TEXT (u_child)->getNormalizedBpath ();
	}

	if ( curve ) {
		Path*   temp=new Path;
        temp->LoadPathVector(curve->get_pathvector(), tr_mat, true);
		Shape*  n_shp=new Shape;
		temp->Convert(0.25);
		temp->Fill(n_shp,0);
		Shape*  uncross=new Shape;
		SPStyle* style = u_child->style;
		if ( style && style->fill_rule.computed == SP_WIND_RULE_EVENODD ) {
			uncross->ConvertToShape(n_shp,fill_oddEven);
		} else {
			uncross->ConvertToShape(n_shp,fill_nonZero);
		}
		UnionShape(computed, uncross);
		delete uncross;
		delete n_shp;
		delete temp;
		curve->unref();
	} else {
//		printf("no curve\n");
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
