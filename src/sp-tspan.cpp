/*
 * SVG <text> and <tspan> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * fixme:
 *
 * These subcomponents should not be items, or alternately
 * we have to invent set of flags to mark, whether standard
 * attributes are applicable to given item (I even like this
 * idea somewhat - Lauris)
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>
#include <glibmm/i18n.h>

#include <livarot/Path.h>
#include "svg/stringstream.h"
#include "attributes.h"
#include "sp-use-reference.h"
#include "sp-tspan.h"
#include "sp-tref.h"
#include "sp-textpath.h"
#include "text-editing.h"
#include "style.h"
#include "xml/repr.h"
#include "document.h"
#include "2geom/transforms.h"

/*#####################################################
#  SPTSPAN
#####################################################*/
SPTSpan::SPTSpan() : SPItem() {
    this->role = SP_TSPAN_ROLE_UNSPECIFIED;
}

SPTSpan::~SPTSpan() {
}

void SPTSpan::build(SPDocument *doc, Inkscape::XML::Node *repr) {
    this->readAttr( "x" );
    this->readAttr( "y" );
    this->readAttr( "dx" );
    this->readAttr( "dy" );
    this->readAttr( "rotate" );
    this->readAttr( "sodipodi:role" );

    SPItem::build(doc, repr);
}

void SPTSpan::release() {
    SPItem::release();
}

void SPTSpan::set(unsigned int key, const gchar* value) {
    if (this->attributes.readSingleAttribute(key, value, style, &viewport)) {
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    } else {
        switch (key) {
            case SP_ATTR_SODIPODI_ROLE:
                if (value && (!strcmp(value, "line") || !strcmp(value, "paragraph"))) {
                    this->role = SP_TSPAN_ROLE_LINE;
                } else {
                    this->role = SP_TSPAN_ROLE_UNSPECIFIED;
                }
                break;
                
            default:
                SPItem::set(key, value);
                break;
        }
    }
}

void SPTSpan::update(SPCtx *ctx, guint flags) {
    unsigned childflags = flags;
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        childflags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    childflags &= SP_OBJECT_MODIFIED_CASCADE;

    for ( SPObject *ochild = this->firstChild() ; ochild ; ochild = ochild->getNext() ) {
        if ( flags || ( ochild->uflags & SP_OBJECT_MODIFIED_FLAG )) {
        	ochild->updateDisplay(ctx, childflags);
        }
    }

    SPItem::update(ctx, flags);

    if (flags & ( SP_OBJECT_STYLE_MODIFIED_FLAG |
                  SP_OBJECT_CHILD_MODIFIED_FLAG |
                  SP_TEXT_LAYOUT_MODIFIED_FLAG   ) )
    {
        SPItemCtx const *ictx = reinterpret_cast<SPItemCtx const *>(ctx);

        double const w = ictx->viewport.width();
        double const h = ictx->viewport.height();
        double const em = style->font_size.computed;
        double const ex = 0.5 * em;  // fixme: get x height from pango or libnrtype.

        attributes.update( em, ex, w, h );
    }
}

void SPTSpan::modified(unsigned int flags) {
//    SPItem::onModified(flags);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    for ( SPObject *ochild = this->firstChild() ; ochild ; ochild = ochild->getNext() ) {
        if (flags || (ochild->mflags & SP_OBJECT_MODIFIED_FLAG)) {
            ochild->emitModified(flags);
        }
    }
}

Geom::OptRect SPTSpan::bbox(Geom::Affine const &transform, SPItem::BBoxType type) const {
    Geom::OptRect bbox;
    // find out the ancestor text which holds our layout
    SPObject const *parent_text = this;
    
    while (parent_text && !SP_IS_TEXT(parent_text)) {
        parent_text = parent_text->parent;
    }
    
    if (parent_text == NULL) {
        return bbox;
    }

    // get the bbox of our portion of the layout
    bbox = SP_TEXT(parent_text)->layout.bounds(transform, sp_text_get_length_upto(parent_text, this), sp_text_get_length_upto(this, NULL) - 1);
    
    if (!bbox) {
    	return bbox;
    }

    // Add stroke width
    // FIXME this code is incorrect
    if (type == SPItem::VISUAL_BBOX && !this->style->stroke.isNone()) {
        double scale = transform.descrim();
        bbox->expandBy(0.5 * this->style->stroke_width.computed * scale);
    }
    
    return bbox;
}

Inkscape::XML::Node* SPTSpan::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:tspan");
    }

    this->attributes.writeTo(repr);

    if ( flags&SP_OBJECT_WRITE_BUILD ) {
        GSList *l = NULL;

        for (SPObject* child = this->firstChild() ; child ; child = child->getNext() ) {
            Inkscape::XML::Node* c_repr=NULL;

            if ( SP_IS_TSPAN(child) || SP_IS_TREF(child) ) {
                c_repr = child->updateRepr(xml_doc, NULL, flags);
            } else if ( SP_IS_TEXTPATH(child) ) {
                //c_repr = child->updateRepr(xml_doc, NULL, flags); // shouldn't happen
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
        for (SPObject* child = this->firstChild() ; child ; child = child->getNext() ) {
            if ( SP_IS_TSPAN(child) || SP_IS_TREF(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_TEXTPATH(child) ) {
                //c_repr = child->updateRepr(xml_doc, NULL, flags); // shouldn't happen
            } else if ( SP_IS_STRING(child) ) {
                child->getRepr()->setContent(SP_STRING(child)->string.c_str());
            }
        }
    }

    SPItem::write(xml_doc, repr, flags);

    return repr;
}

const char* SPTSpan::displayName() const {
    return _("Text Span");
}


/*#####################################################
#  SPTEXTPATH
#####################################################*/
void   refresh_textpath_source(SPTextPath* offset);

SPTextPath::SPTextPath() : SPItem() {
    this->startOffset._set = false;
    this->originalPath = NULL;
    this->isUpdating=false;

    // set up the uri reference
    this->sourcePath = new SPUsePath(this);
    this->sourcePath->user_unlink = sp_textpath_to_text;
}

SPTextPath::~SPTextPath() {
	delete this->sourcePath;
}

void SPTextPath::build(SPDocument *doc, Inkscape::XML::Node *repr) {
    this->readAttr( "x" );
    this->readAttr( "y" );
    this->readAttr( "dx" );
    this->readAttr( "dy" );
    this->readAttr( "rotate" );
    this->readAttr( "startOffset" );
    this->readAttr( "xlink:href" );

    bool  no_content = true;

    for (Inkscape::XML::Node* rch = repr->firstChild() ; rch != NULL; rch = rch->next()) {
        if ( rch->type() == Inkscape::XML::TEXT_NODE )
        {
            no_content = false;
            break;
        }
    }

    if ( no_content ) {
        Inkscape::XML::Document *xml_doc = doc->getReprDoc();
        Inkscape::XML::Node* rch = xml_doc->createTextNode("");
        repr->addChild(rch, NULL);
    }

    SPItem::build(doc, repr);
}

void SPTextPath::release() {
    //this->attributes.~TextTagAttributes();

    if (this->originalPath) {
    	delete this->originalPath;
    }

    this->originalPath = NULL;

    SPItem::release();
}

void SPTextPath::set(unsigned int key, const gchar* value) {
    if (this->attributes.readSingleAttribute(key, value, style, &viewport)) {
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    } else {
        switch (key) {
            case SP_ATTR_XLINK_HREF:
                this->sourcePath->link((char*)value);
                break;
            case SP_ATTR_STARTOFFSET:
                this->startOffset.readOrUnset(value);
                this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
                break;
            default:
                SPItem::set(key, value);
                break;
        }
    }
}

void SPTextPath::update(SPCtx *ctx, guint flags) {
    this->isUpdating = true;

    if ( this->sourcePath->sourceDirty ) {
        refresh_textpath_source(this);
    }

    this->isUpdating = false;

    SPItem::update(ctx, flags);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    for ( SPObject *ochild = this->firstChild() ; ochild ; ochild = ochild->getNext() ) {
        if ( flags || ( ochild->uflags & SP_OBJECT_MODIFIED_FLAG )) {
            ochild->updateDisplay(ctx, flags);
        }
    }

    if (flags & ( SP_OBJECT_STYLE_MODIFIED_FLAG |
                  SP_OBJECT_CHILD_MODIFIED_FLAG |
                  SP_TEXT_LAYOUT_MODIFIED_FLAG   ) )
    {
        SPItemCtx const *ictx = reinterpret_cast<SPItemCtx const *>(ctx);

        double const w = ictx->viewport.width();
        double const h = ictx->viewport.height();
        double const em = style->font_size.computed;
        double const ex = 0.5 * em;  // fixme: get x height from pango or libnrtype.

        attributes.update( em, ex, w, h );
    }
}


void refresh_textpath_source(SPTextPath* tp)
{
    if ( tp == NULL ) {
    	return;
    }

    tp->sourcePath->refresh_source();
    tp->sourcePath->sourceDirty=false;

    // finalisons
    if ( tp->sourcePath->originalPath ) {
        if (tp->originalPath) {
            delete tp->originalPath;
        }

        tp->originalPath = NULL;

        tp->originalPath = new Path;
        tp->originalPath->Copy(tp->sourcePath->originalPath);
        tp->originalPath->ConvertWithBackData(0.01);
    }
}

void SPTextPath::modified(unsigned int flags) {
//    SPItem::onModified(flags);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    for ( SPObject *ochild = this->firstChild() ; ochild ; ochild = ochild->getNext() ) {
        if (flags || (ochild->mflags & SP_OBJECT_MODIFIED_FLAG)) {
            ochild->emitModified(flags);
        }
    }
}

Inkscape::XML::Node* SPTextPath::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:textPath");
    }

    this->attributes.writeTo(repr);
    if (this->startOffset._set) {
        if (this->startOffset.unit == SVGLength::PERCENT) {
	        Inkscape::SVGOStringStream os;
            os << (this->startOffset.computed * 100.0) << "%";
            this->getRepr()->setAttribute("startOffset", os.str().c_str());
        } else {
            /* FIXME: This logic looks rather undesirable if e.g. startOffset is to be
               in ems. */
            sp_repr_set_svg_double(repr, "startOffset", this->startOffset.computed);
        }
    }

    if ( this->sourcePath->sourceHref ) {
    	repr->setAttribute("xlink:href", this->sourcePath->sourceHref);
    }

    if ( flags & SP_OBJECT_WRITE_BUILD ) {
        GSList *l = NULL;

        for (SPObject* child = this->firstChild() ; child ; child = child->getNext() ) {
            Inkscape::XML::Node* c_repr=NULL;

            if ( SP_IS_TSPAN(child) || SP_IS_TREF(child) ) {
                c_repr = child->updateRepr(xml_doc, NULL, flags);
            } else if ( SP_IS_TEXTPATH(child) ) {
                //c_repr = child->updateRepr(xml_doc, NULL, flags); // shouldn't happen
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
        for (SPObject* child = this->firstChild() ; child ; child = child->getNext() ) {
            if ( SP_IS_TSPAN(child) || SP_IS_TREF(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_TEXTPATH(child) ) {
                //c_repr = child->updateRepr(xml_doc, NULL, flags); // shouldn't happen
            } else if ( SP_IS_STRING(child) ) {
                child->getRepr()->setContent(SP_STRING(child)->string.c_str());
            }
        }
    }

    SPItem::write(xml_doc, repr, flags);

    return repr;
}


SPItem *sp_textpath_get_path_item(SPTextPath *tp)
{
    if (tp && tp->sourcePath) {
        SPItem *refobj = tp->sourcePath->getObject();

        if (SP_IS_ITEM(refobj)) {
            return refobj;
        }
    }
    return NULL;
}

void sp_textpath_to_text(SPObject *tp)
{
    SPObject *text = tp->parent;

    Geom::OptRect bbox = SP_ITEM(text)->geometricBounds(SP_ITEM(text)->i2doc_affine());

    if (!bbox) {
    	return;
    }

    Geom::Point xy = bbox->min();
    xy *= tp->document->getDocumentScale().inverse(); // Convert to user-units.
    
    // make a list of textpath children
    GSList *tp_reprs = NULL;

    for (SPObject *o = tp->firstChild() ; o != NULL; o = o->next) {
        tp_reprs = g_slist_prepend(tp_reprs, o->getRepr());
    }

    for ( GSList *i = tp_reprs ; i ; i = i->next ) {
        // make a copy of each textpath child
        Inkscape::XML::Node *copy = ((Inkscape::XML::Node *) i->data)->duplicate(text->getRepr()->document());
        // remove the old repr from under textpath
        tp->getRepr()->removeChild((Inkscape::XML::Node *) i->data);
        // put its copy under text
        text->getRepr()->addChild(copy, NULL); // fixme: copy id
    }

    //remove textpath
    tp->deleteObject();
    g_slist_free(tp_reprs);

    // set x/y on text (to be near where it was when on path)
    /* fixme: Yuck, is this really the right test? */
    if (xy[Geom::X] != 1e18 && xy[Geom::Y] != 1e18) {
        sp_repr_set_svg_double(text->getRepr(), "x", xy[Geom::X]);
        sp_repr_set_svg_double(text->getRepr(), "y", xy[Geom::Y]);
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
