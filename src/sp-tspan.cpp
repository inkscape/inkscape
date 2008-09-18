#define __SP_TSPAN_C__

/*
 * SVG <text> and <tspan> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
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
#include "libnr/nr-matrix-fns.h"
#include "xml/repr.h"
#include "document.h"


/*#####################################################
#  SPTSPAN
#####################################################*/

static void sp_tspan_class_init(SPTSpanClass *classname);
static void sp_tspan_init(SPTSpan *tspan);

static void sp_tspan_build(SPObject * object, SPDocument * document, Inkscape::XML::Node * repr);
static void sp_tspan_release(SPObject *object);
static void sp_tspan_set(SPObject *object, unsigned key, gchar const *value);
static void sp_tspan_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_tspan_modified(SPObject *object, unsigned flags);
static void sp_tspan_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags);
static Inkscape::XML::Node *sp_tspan_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static char *sp_tspan_description (SPItem *item);

static SPItemClass *tspan_parent_class;

/**
 *
 */
GType
sp_tspan_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPTSpanClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc) sp_tspan_class_init,
            NULL,    /* class_finalize */
            NULL,    /* class_data */
            sizeof(SPTSpan),
            16,    /* n_preallocs */
            (GInstanceInitFunc) sp_tspan_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static(SP_TYPE_ITEM, "SPTSpan", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_tspan_class_init(SPTSpanClass *classname)
{
    SPObjectClass * sp_object_class;
    SPItemClass * item_class;
	
    sp_object_class = (SPObjectClass *) classname;
    item_class = (SPItemClass *) classname;
	
    tspan_parent_class = (SPItemClass*)g_type_class_ref(SP_TYPE_ITEM);
	
    sp_object_class->build = sp_tspan_build;
    sp_object_class->release = sp_tspan_release;
    sp_object_class->set = sp_tspan_set;
    sp_object_class->update = sp_tspan_update;
    sp_object_class->modified = sp_tspan_modified;
    sp_object_class->write = sp_tspan_write;

    item_class->bbox = sp_tspan_bbox;
    item_class->description = sp_tspan_description;
}

static void
sp_tspan_init(SPTSpan *tspan)
{
    tspan->role = SP_TSPAN_ROLE_UNSPECIFIED;
    new (&tspan->attributes) TextTagAttributes;
}

static void
sp_tspan_release(SPObject *object)
{
    SPTSpan *tspan = SP_TSPAN(object);

    tspan->attributes.~TextTagAttributes();
	
    if (((SPObjectClass *) tspan_parent_class)->release)
        ((SPObjectClass *) tspan_parent_class)->release(object);
}

static void
sp_tspan_build(SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr)
{
    //SPTSpan *tspan = SP_TSPAN(object);
	
    sp_object_read_attr(object, "x");
    sp_object_read_attr(object, "y");
    sp_object_read_attr(object, "dx");
    sp_object_read_attr(object, "dy");
    sp_object_read_attr(object, "rotate");
    sp_object_read_attr(object, "sodipodi:role");
	
    if (((SPObjectClass *) tspan_parent_class)->build)
        ((SPObjectClass *) tspan_parent_class)->build(object, doc, repr);
}

static void
sp_tspan_set(SPObject *object, unsigned key, gchar const *value)
{
    SPTSpan *tspan = SP_TSPAN(object);
	
    if (tspan->attributes.readSingleAttribute(key, value)) {
        object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    } else {
        switch (key) {
            case SP_ATTR_SODIPODI_ROLE:
                if (value && (!strcmp(value, "line") || !strcmp(value, "paragraph"))) {
                    tspan->role = SP_TSPAN_ROLE_LINE;
                } else {
                    tspan->role = SP_TSPAN_ROLE_UNSPECIFIED;
                }
                break;
            default:
                if (((SPObjectClass *) tspan_parent_class)->set)
                    (((SPObjectClass *) tspan_parent_class)->set)(object, key, value);
                break;
        }
    }
}

static void
sp_tspan_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (((SPObjectClass *) tspan_parent_class)->update)
        ((SPObjectClass *) tspan_parent_class)->update(object, ctx, flags);
	
    if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;
	
    SPObject *ochild;
    for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
        if ( flags || ( ochild->uflags & SP_OBJECT_MODIFIED_FLAG )) {
	    ochild->updateDisplay(ctx, flags);
        }
    }
}

static void
sp_tspan_modified(SPObject *object, unsigned flags)
{
    if (((SPObjectClass *) tspan_parent_class)->modified)
        ((SPObjectClass *) tspan_parent_class)->modified(object, flags);
	
    if (flags & SP_OBJECT_MODIFIED_FLAG)
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;
	
    SPObject *ochild;
    for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
        if (flags || (ochild->mflags & SP_OBJECT_MODIFIED_FLAG)) {
            ochild->emitModified(flags);
        }
    }
}

static void sp_tspan_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const /*flags*/)
{
    // find out the ancestor text which holds our layout
    SPObject *parent_text = SP_OBJECT(item);
    for (; parent_text != NULL && !SP_IS_TEXT(parent_text); parent_text = SP_OBJECT_PARENT (parent_text));
    if (parent_text == NULL) return;

    // get the bbox of our portion of the layout
    SP_TEXT(parent_text)->layout.getBoundingBox(bbox, transform, sp_text_get_length_upto(parent_text, item), sp_text_get_length_upto(item, NULL) - 1);

    // Add stroke width
    SPStyle* style=SP_OBJECT_STYLE (item);
    if (!style->stroke.isNone()) {
        double const scale = transform.descrim();
        if ( fabs(style->stroke_width.computed * scale) > 0.01 ) { // sinon c'est 0=oon veut pas de bord
            double const width = MAX(0.125, style->stroke_width.computed * scale);
            if ( fabs(bbox->x1 - bbox->x0) > -0.00001 && fabs(bbox->y1 - bbox->y0) > -0.00001 ) {
                bbox->x0-=0.5*width;
                bbox->x1+=0.5*width;
                bbox->y0-=0.5*width;
                bbox->y1+=0.5*width;
            }
        }
    }
}

static Inkscape::XML::Node *
sp_tspan_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPTSpan *tspan = SP_TSPAN(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:tspan");
    }
	
    tspan->attributes.writeTo(repr);
	
    if ( flags&SP_OBJECT_WRITE_BUILD ) {
        GSList *l = NULL;
        for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            Inkscape::XML::Node* c_repr=NULL;
            if ( SP_IS_TSPAN(child) || SP_IS_TREF(child) ) {
                c_repr = child->updateRepr(xml_doc, NULL, flags);
            } else if ( SP_IS_TEXTPATH(child) ) {
                //c_repr = child->updateRepr(xml_doc, NULL, flags); // shouldn't happen
            } else if ( SP_IS_STRING(child) ) {
                c_repr = xml_doc->createTextNode(SP_STRING(child)->string.c_str());
            }
            if ( c_repr ) l = g_slist_prepend(l, c_repr);
        }
        while ( l ) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove(l, l->data);
        }
    } else {
        for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            if ( SP_IS_TSPAN(child) || SP_IS_TREF(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_TEXTPATH(child) ) {
                //c_repr = child->updateRepr(xml_doc, NULL, flags); // shouldn't happen
            } else if ( SP_IS_STRING(child) ) {
                SP_OBJECT_REPR(child)->setContent(SP_STRING(child)->string.c_str());
            }
        }
    }
	
    if (((SPObjectClass *) tspan_parent_class)->write)
        ((SPObjectClass *) tspan_parent_class)->write(object, xml_doc, repr, flags);
	
    return repr;
}

static char *
sp_tspan_description(SPItem *item)
{
    g_return_val_if_fail(SP_IS_TSPAN(item), NULL);

    return g_strdup(_("<b>Text span</b>"));
}


/*#####################################################
#  SPTEXTPATH
#####################################################*/

static void sp_textpath_class_init(SPTextPathClass *classname);
static void sp_textpath_init(SPTextPath *textpath);
static void sp_textpath_finalize(GObject *obj);

static void sp_textpath_build(SPObject * object, SPDocument * document, Inkscape::XML::Node * repr);
static void sp_textpath_release(SPObject *object);
static void sp_textpath_set(SPObject *object, unsigned key, gchar const *value);
static void sp_textpath_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_textpath_modified(SPObject *object, unsigned flags);
static Inkscape::XML::Node *sp_textpath_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static SPItemClass *textpath_parent_class;

void   refresh_textpath_source(SPTextPath* offset);


/**
 *
 */
GType
sp_textpath_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPTextPathClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc) sp_textpath_class_init,
            NULL,    /* class_finalize */
            NULL,    /* class_data */
            sizeof(SPTextPath),
            16,    /* n_preallocs */
            (GInstanceInitFunc) sp_textpath_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static(SP_TYPE_ITEM, "SPTextPath", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_textpath_class_init(SPTextPathClass *classname)
{
    GObjectClass  *gobject_class = (GObjectClass *) classname;
    SPObjectClass * sp_object_class;
    SPItemClass * item_class;
	
    sp_object_class = (SPObjectClass *) classname;
    item_class = (SPItemClass *) classname;
	
    textpath_parent_class = (SPItemClass*)g_type_class_ref(SP_TYPE_ITEM);
	
    gobject_class->finalize = sp_textpath_finalize;
		
    sp_object_class->build = sp_textpath_build;
    sp_object_class->release = sp_textpath_release;
    sp_object_class->set = sp_textpath_set;
    sp_object_class->update = sp_textpath_update;
    sp_object_class->modified = sp_textpath_modified;
    sp_object_class->write = sp_textpath_write;
}

static void
sp_textpath_init(SPTextPath *textpath)
{
    new (&textpath->attributes) TextTagAttributes;
	
    textpath->startOffset._set = false;
    textpath->originalPath = NULL;
    textpath->isUpdating=false;
    // set up the uri reference
    textpath->sourcePath = new SPUsePath(SP_OBJECT(textpath));
    textpath->sourcePath->user_unlink = sp_textpath_to_text;
}

static void
sp_textpath_finalize(GObject *obj)
{
    SPTextPath *textpath = (SPTextPath *) obj;
	
    delete textpath->sourcePath;
}

static void
sp_textpath_release(SPObject *object)
{
    SPTextPath *textpath = SP_TEXTPATH(object);
	
    textpath->attributes.~TextTagAttributes();
	
    if (textpath->originalPath) delete textpath->originalPath;
    textpath->originalPath = NULL;
		
    if (((SPObjectClass *) textpath_parent_class)->release)
        ((SPObjectClass *) textpath_parent_class)->release(object);
}

static void
sp_textpath_build(SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr)
{
    //SPTextPath *textpath = SP_TEXTPATH(object);
	
    sp_object_read_attr(object, "x");
    sp_object_read_attr(object, "y");
    sp_object_read_attr(object, "dx");
    sp_object_read_attr(object, "dy");
    sp_object_read_attr(object, "rotate");
    sp_object_read_attr(object, "startOffset");
    sp_object_read_attr(object, "xlink:href");
	
    bool  no_content=true;
    for (Inkscape::XML::Node* rch = repr->firstChild() ; rch != NULL; rch = rch->next()) {
        if ( rch->type() == Inkscape::XML::TEXT_NODE ) {no_content=false;break;}
    }
	
    if ( no_content ) {
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);
        Inkscape::XML::Node* rch = xml_doc->createTextNode("");
        repr->addChild(rch, NULL);
    }
	
    if (((SPObjectClass *) textpath_parent_class)->build)
        ((SPObjectClass *) textpath_parent_class)->build(object, doc, repr);
}

static void
sp_textpath_set(SPObject *object, unsigned key, gchar const *value)
{
    SPTextPath *textpath = SP_TEXTPATH(object);
	
    if (textpath->attributes.readSingleAttribute(key, value)) {
        object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    } else {
        switch (key) {
            case SP_ATTR_XLINK_HREF:
                textpath->sourcePath->link((char*)value);
                break;
            case SP_ATTR_STARTOFFSET:
                textpath->startOffset.readOrUnset(value);
                object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
                break;
            default:
                if (((SPObjectClass *) textpath_parent_class)->set)
                    (((SPObjectClass *) textpath_parent_class)->set)(object, key, value);
                break;
        }
    }
}

static void
sp_textpath_update(SPObject *object, SPCtx *ctx, guint flags)
{
    SPTextPath *textpath = SP_TEXTPATH(object);
	
    textpath->isUpdating=true;
    if ( textpath->sourcePath->sourceDirty ) refresh_textpath_source(textpath);
    textpath->isUpdating=false;
		
    if (((SPObjectClass *) textpath_parent_class)->update)
        ((SPObjectClass *) textpath_parent_class)->update(object, ctx, flags);
		
    if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;
			
    SPObject *ochild;
    for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
        if ( flags || ( ochild->uflags & SP_OBJECT_MODIFIED_FLAG )) {
            ochild->updateDisplay(ctx, flags);
        }
    }
}


void   refresh_textpath_source(SPTextPath* tp)
{
    if ( tp == NULL ) return;
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

static void
sp_textpath_modified(SPObject *object, unsigned flags)
{
    if (((SPObjectClass *) textpath_parent_class)->modified)
        ((SPObjectClass *) textpath_parent_class)->modified(object, flags);
	
    if (flags & SP_OBJECT_MODIFIED_FLAG)
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;
	
    SPObject *ochild;
    for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
        if (flags || (ochild->mflags & SP_OBJECT_MODIFIED_FLAG)) {
            ochild->emitModified(flags);
        }
    }
}
static Inkscape::XML::Node *
sp_textpath_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPTextPath *textpath = SP_TEXTPATH(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:textPath");
    }
	
    textpath->attributes.writeTo(repr);
    if (textpath->startOffset._set) {
        if (textpath->startOffset.unit == SVGLength::PERCENT) {
	        Inkscape::SVGOStringStream os;
            os << (textpath->startOffset.computed * 100.0) << "%";
            SP_OBJECT_REPR(textpath)->setAttribute("startOffset", os.str().c_str());
        } else {
            /* FIXME: This logic looks rather undesirable if e.g. startOffset is to be
               in ems. */
            sp_repr_set_svg_double(repr, "startOffset", textpath->startOffset.computed);
        }
    }

    if ( textpath->sourcePath->sourceHref ) repr->setAttribute("xlink:href", textpath->sourcePath->sourceHref);
	
    if ( flags&SP_OBJECT_WRITE_BUILD ) {
        GSList *l = NULL;
        for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            Inkscape::XML::Node* c_repr=NULL;
            if ( SP_IS_TSPAN(child) || SP_IS_TREF(child) ) {
                c_repr = child->updateRepr(xml_doc, NULL, flags);
            } else if ( SP_IS_TEXTPATH(child) ) {
                //c_repr = child->updateRepr(xml_doc, NULL, flags); // shouldn't happen
            } else if ( SP_IS_STRING(child) ) {
                c_repr = xml_doc->createTextNode(SP_STRING(child)->string.c_str());
            }
            if ( c_repr ) l = g_slist_prepend(l, c_repr);
        }
        while ( l ) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove(l, l->data);
        }
    } else {
        for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            if ( SP_IS_TSPAN(child) || SP_IS_TREF(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_TEXTPATH(child) ) {
                //c_repr = child->updateRepr(xml_doc, NULL, flags); // shouldn't happen
            } else if ( SP_IS_STRING(child) ) {
                SP_OBJECT_REPR(child)->setContent(SP_STRING(child)->string.c_str());
            }
        }
    }
	
    if (((SPObjectClass *) textpath_parent_class)->write)
        ((SPObjectClass *) textpath_parent_class)->write(object, xml_doc, repr, flags);
	
    return repr;
}


SPItem *
sp_textpath_get_path_item(SPTextPath *tp)
{
    if (tp && tp->sourcePath) {
        SPItem *refobj = tp->sourcePath->getObject();
        if (SP_IS_ITEM(refobj))
            return (SPItem *) refobj;
    }
    return NULL;
}

void
sp_textpath_to_text(SPObject *tp)
{
    SPObject *text = SP_OBJECT_PARENT(tp);

    NRRect bbox;
    sp_item_invoke_bbox(SP_ITEM(text), &bbox, sp_item_i2doc_affine(SP_ITEM(text)), TRUE);
    NR::Point xy(bbox.x0, bbox.y0);

    // make a list of textpath children
    GSList *tp_reprs = NULL;
    for (SPObject *o = SP_OBJECT(tp)->firstChild() ; o != NULL; o = o->next) {
        tp_reprs = g_slist_prepend(tp_reprs, SP_OBJECT_REPR(o));
    }

    for ( GSList *i = tp_reprs ; i ; i = i->next ) {
        // make a copy of each textpath child
        Inkscape::XML::Node *copy = ((Inkscape::XML::Node *) i->data)->duplicate(SP_OBJECT_REPR(text)->document());
        // remove the old repr from under textpath
        SP_OBJECT_REPR(tp)->removeChild((Inkscape::XML::Node *) i->data);
        // put its copy under text
        SP_OBJECT_REPR(text)->addChild(copy, NULL); // fixme: copy id
    }

    //remove textpath
    tp->deleteObject();
    g_slist_free(tp_reprs);

    // set x/y on text
    /* fixme: Yuck, is this really the right test? */
    if (xy[NR::X] != 1e18 && xy[NR::Y] != 1e18) {
        sp_repr_set_svg_double(SP_OBJECT_REPR(text), "x", xy[NR::X]);
        sp_repr_set_svg_double(SP_OBJECT_REPR(text), "y", xy[NR::Y]);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
