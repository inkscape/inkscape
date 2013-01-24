/*
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "xml/repr.h"
//#include "svg/svg.h"

//#include "style.h"

#include "sp-flowdiv.h"
#include "sp-string.h"
#include "document.h"

static void sp_flowdiv_release (SPObject *object);
static Inkscape::XML::Node *sp_flowdiv_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_flowdiv_update (SPObject *object, SPCtx *ctx, unsigned int flags);
static void sp_flowdiv_modified (SPObject *object, guint flags);
static void sp_flowdiv_build (SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr);
static void sp_flowdiv_set (SPObject *object, unsigned int key, const gchar *value);

static void sp_flowtspan_release (SPObject *object);
static Inkscape::XML::Node *sp_flowtspan_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_flowtspan_update (SPObject *object, SPCtx *ctx, unsigned int flags);
static void sp_flowtspan_modified (SPObject *object, guint flags);
static void sp_flowtspan_build (SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr);
static void sp_flowtspan_set (SPObject *object, unsigned int key, const gchar *value);

static void sp_flowpara_release (SPObject *object);
static Inkscape::XML::Node *sp_flowpara_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_flowpara_update (SPObject *object, SPCtx *ctx, unsigned int flags);
static void sp_flowpara_modified (SPObject *object, guint flags);
static void sp_flowpara_build (SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr);
static void sp_flowpara_set (SPObject *object, unsigned int key, const gchar *value);

static void sp_flowline_release (SPObject *object);
static Inkscape::XML::Node *sp_flowline_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_flowline_modified (SPObject *object, guint flags);

static void sp_flowregionbreak_release (SPObject *object);
static Inkscape::XML::Node *sp_flowregionbreak_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_flowregionbreak_modified (SPObject *object, guint flags);

G_DEFINE_TYPE(SPFlowdiv, sp_flowdiv, SP_TYPE_ITEM);

static void sp_flowdiv_class_init(SPFlowdivClass *klass)
{
    SPObjectClass *sp_object_class = reinterpret_cast<SPObjectClass *>(klass);

    sp_object_class->build = sp_flowdiv_build;
    sp_object_class->set = sp_flowdiv_set;
    sp_object_class->release = sp_flowdiv_release;
    sp_object_class->write = sp_flowdiv_write;
    sp_object_class->update = sp_flowdiv_update;
    sp_object_class->modified = sp_flowdiv_modified;
}

static void sp_flowdiv_init(SPFlowdiv */*group*/)
{
}

static void sp_flowdiv_release(SPObject *object)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowdiv_parent_class)->release) {
        reinterpret_cast<SPObjectClass *>(sp_flowdiv_parent_class)->release(object);
    }
}

static void sp_flowdiv_update(SPObject *object, SPCtx *ctx, unsigned int flags)
{
    SPItemCtx *ictx = reinterpret_cast<SPItemCtx *>(ctx);
    SPItemCtx cctx = *ictx;

    if (reinterpret_cast<SPObjectClass *>(sp_flowdiv_parent_class)->update) {
        reinterpret_cast<SPObjectClass *>(sp_flowdiv_parent_class)->update(object, ctx, flags);
    }

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList* l = NULL;
    for (SPObject *child = object->firstChild() ; child ; child = child->getNext() ) {
        g_object_ref( G_OBJECT(child) );
        l = g_slist_prepend(l, child);
    }
    l = g_slist_reverse(l);
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
        g_object_unref( G_OBJECT(child) );
    }
}

static void sp_flowdiv_modified(SPObject *object, guint flags)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowdiv_parent_class)->modified) {
        reinterpret_cast<SPObjectClass *>(sp_flowdiv_parent_class)->modified(object, flags);
    }

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList *l = NULL;
    for ( SPObject *child = object->firstChild() ; child ; child = child->getNext() ) {
        g_object_ref( G_OBJECT(child) );
        l = g_slist_prepend(l, child);
    }
    l = g_slist_reverse (l);
    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);
        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }
        g_object_unref( G_OBJECT(child) );
    }
}

static void sp_flowdiv_build(SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr)
{
    object->_requireSVGVersion(Inkscape::Version(1, 2));

    if (reinterpret_cast<SPObjectClass *>(sp_flowdiv_parent_class)->build) {
        reinterpret_cast<SPObjectClass *>(sp_flowdiv_parent_class)->build(object, doc, repr);
    }
}

static void sp_flowdiv_set(SPObject *object, unsigned int key, const gchar *value)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowdiv_parent_class)->set) {
        reinterpret_cast<SPObjectClass *>(sp_flowdiv_parent_class)->set(object, key, value);
    }
}

static Inkscape::XML::Node *sp_flowdiv_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    if ( flags & SP_OBJECT_WRITE_BUILD ) {
        if ( repr == NULL ) {
            repr = xml_doc->createElement("svg:flowDiv");
        }
        GSList *l = NULL;
        for (SPObject* child = object->firstChild() ; child ; child = child->getNext() ) {
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
        for ( SPObject* child = object->firstChild() ; child ; child = child->getNext() ) {
            if ( SP_IS_FLOWTSPAN (child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_FLOWPARA(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_STRING(child) ) {
                child->getRepr()->setContent(SP_STRING(child)->string.c_str());
            }
        }
    }

    if (((SPObjectClass *) (sp_flowdiv_parent_class))->write) {
        ((SPObjectClass *) (sp_flowdiv_parent_class))->write(object, xml_doc, repr, flags);
    }

    return repr;
}


/*
 *
 */
G_DEFINE_TYPE(SPFlowtspan, sp_flowtspan, SP_TYPE_ITEM);

static void sp_flowtspan_class_init(SPFlowtspanClass *klass)
{
    SPObjectClass *sp_object_class = reinterpret_cast<SPObjectClass *>(klass);

    sp_object_class->build = sp_flowtspan_build;
    sp_object_class->set = sp_flowtspan_set;
    sp_object_class->release = sp_flowtspan_release;
    sp_object_class->write = sp_flowtspan_write;
    sp_object_class->update = sp_flowtspan_update;
    sp_object_class->modified = sp_flowtspan_modified;
}

static void sp_flowtspan_init(SPFlowtspan */*group*/)
{
}

static void sp_flowtspan_release(SPObject *object)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowtspan_parent_class)->release) {
        reinterpret_cast<SPObjectClass *>(sp_flowtspan_parent_class)->release(object);
    }
}

static void sp_flowtspan_update(SPObject *object, SPCtx *ctx, unsigned int flags)
{
    SPItemCtx *ictx = reinterpret_cast<SPItemCtx *>(ctx);
    SPItemCtx cctx = *ictx;

    if (reinterpret_cast<SPObjectClass *>(sp_flowtspan_parent_class)->update) {
        reinterpret_cast<SPObjectClass *>(sp_flowtspan_parent_class)->update(object, ctx, flags);
    }

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList* l = NULL;
    for ( SPObject *child = object->firstChild() ; child ; child = child->getNext() ) {
        g_object_ref( G_OBJECT(child) );
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
        g_object_unref( G_OBJECT(child) );
    }
}

static void sp_flowtspan_modified(SPObject *object, guint flags)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowtspan_parent_class)->modified) {
        reinterpret_cast<SPObjectClass *>(sp_flowtspan_parent_class)->modified(object, flags);
    }

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList *l = NULL;
    for ( SPObject *child = object->firstChild() ; child ; child = child->getNext() ) {
        g_object_ref( G_OBJECT(child) );
        l = g_slist_prepend(l, child);
    }
    l = g_slist_reverse (l);
    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);
        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }
        g_object_unref( G_OBJECT(child) );
    }
}

static void sp_flowtspan_build(SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowtspan_parent_class)->build) {
        reinterpret_cast<SPObjectClass *>(sp_flowtspan_parent_class)->build(object, doc, repr);
    }
}

static void sp_flowtspan_set(SPObject *object, unsigned int key, const gchar *value)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowtspan_parent_class)->set) {
        reinterpret_cast<SPObjectClass *>(sp_flowtspan_parent_class)->set(object, key, value);
    }
}

static Inkscape::XML::Node *sp_flowtspan_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    if ( flags&SP_OBJECT_WRITE_BUILD ) {
        if ( repr == NULL ) {
            repr = xml_doc->createElement("svg:flowSpan");
        }
        GSList *l = NULL;
        for ( SPObject* child = object->firstChild() ; child ; child = child->getNext() ) {
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
        for ( SPObject* child = object->firstChild() ; child ; child = child->getNext() ) {
            if ( SP_IS_FLOWTSPAN(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_FLOWPARA(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_STRING(child) ) {
                child->getRepr()->setContent(SP_STRING(child)->string.c_str());
            }
        }
    }

    if (((SPObjectClass *) (sp_flowtspan_parent_class))->write) {
        ((SPObjectClass *) (sp_flowtspan_parent_class))->write(object, xml_doc, repr, flags);
    }

    return repr;
}



/*
 *
 */
G_DEFINE_TYPE(SPFlowpara, sp_flowpara, SP_TYPE_ITEM);

static void sp_flowpara_class_init(SPFlowparaClass *klass)
{
    SPObjectClass *sp_object_class = reinterpret_cast<SPObjectClass *>(klass);

    sp_object_class->build = sp_flowpara_build;
    sp_object_class->set = sp_flowpara_set;
    sp_object_class->release = sp_flowpara_release;
    sp_object_class->write = sp_flowpara_write;
    sp_object_class->update = sp_flowpara_update;
    sp_object_class->modified = sp_flowpara_modified;
}

static void sp_flowpara_init (SPFlowpara */*group*/)
{
}

static void sp_flowpara_release(SPObject *object)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowpara_parent_class)->release) {
        reinterpret_cast<SPObjectClass *>(sp_flowpara_parent_class)->release(object);
    }
}

static void sp_flowpara_update(SPObject *object, SPCtx *ctx, unsigned int flags)
{
    SPItemCtx *ictx = reinterpret_cast<SPItemCtx *>(ctx);
    SPItemCtx cctx = *ictx;

    if (reinterpret_cast<SPObjectClass *>(sp_flowpara_parent_class)->update) {
        reinterpret_cast<SPObjectClass *>(sp_flowpara_parent_class)->update(object, ctx, flags);
    }

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList* l = NULL;
    for ( SPObject *child = object->firstChild() ; child ; child = child->getNext() ) {
        g_object_ref( G_OBJECT(child) );
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
        g_object_unref( G_OBJECT(child) );
    }
}

static void sp_flowpara_modified(SPObject *object, guint flags)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowpara_parent_class)->modified) {
        reinterpret_cast<SPObjectClass *>(sp_flowpara_parent_class)->modified(object, flags);
    }

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList *l = NULL;
    for ( SPObject *child = object->firstChild() ; child ; child = child->getNext() ) {
        g_object_ref( G_OBJECT(child) );
        l = g_slist_prepend(l, child);
    }
    l = g_slist_reverse (l);
    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);
        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }
        g_object_unref( G_OBJECT(child) );
    }
}

static void sp_flowpara_build(SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowpara_parent_class)->build) {
        reinterpret_cast<SPObjectClass *>(sp_flowpara_parent_class)->build(object, doc, repr);
    }
}

static void sp_flowpara_set(SPObject *object, unsigned int key, const gchar *value)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowpara_parent_class)->set) {
        reinterpret_cast<SPObjectClass *>(sp_flowpara_parent_class)->set(object, key, value);
    }
}

static Inkscape::XML::Node *sp_flowpara_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    if ( flags&SP_OBJECT_WRITE_BUILD ) {
        if ( repr == NULL ) repr = xml_doc->createElement("svg:flowPara");
        GSList *l = NULL;
        for ( SPObject* child = object->firstChild() ; child ; child = child->getNext() ) {
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
        for ( SPObject* child = object->firstChild() ; child ; child = child->getNext() ) {
            if ( SP_IS_FLOWTSPAN(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_FLOWPARA(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_STRING(child) ) {
                child->getRepr()->setContent(SP_STRING(child)->string.c_str());
            }
        }
    }

    if (((SPObjectClass *) (sp_flowpara_parent_class))->write) {
        ((SPObjectClass *) (sp_flowpara_parent_class))->write(object, xml_doc, repr, flags);
    }

    return repr;
}

/*
 *
 */
G_DEFINE_TYPE(SPFlowline, sp_flowline, SP_TYPE_OBJECT);

static void sp_flowline_class_init(SPFlowlineClass *klass)
{
    SPObjectClass *sp_object_class = reinterpret_cast<SPObjectClass *>(klass);

    sp_object_class->release = sp_flowline_release;
    sp_object_class->write = sp_flowline_write;
    sp_object_class->modified = sp_flowline_modified;
}

static void sp_flowline_init(SPFlowline */*group*/)
{
}

static void sp_flowline_release(SPObject *object)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowline_parent_class)->release) {
        reinterpret_cast<SPObjectClass *>(sp_flowline_parent_class)->release(object);
    }
}

static void sp_flowline_modified(SPObject *object, guint flags)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowline_parent_class)->modified) {
        reinterpret_cast<SPObjectClass *>(sp_flowline_parent_class)->modified(object, flags);
    }

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    flags &= SP_OBJECT_MODIFIED_CASCADE;
}

static Inkscape::XML::Node *sp_flowline_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    if ( flags & SP_OBJECT_WRITE_BUILD ) {
        if ( repr == NULL ) {
            repr = xml_doc->createElement("svg:flowLine");
        }
    } else {
    }

    if (reinterpret_cast<SPObjectClass *>(sp_flowline_parent_class)->write) {
        reinterpret_cast<SPObjectClass *>(sp_flowline_parent_class)->write(object, xml_doc, repr, flags);
    }

    return repr;
}

/*
 *
 */
G_DEFINE_TYPE(SPFlowregionbreak, sp_flowregionbreak, SP_TYPE_OBJECT);

static void sp_flowregionbreak_class_init(SPFlowregionbreakClass *klass)
{
    SPObjectClass *sp_object_class = reinterpret_cast<SPObjectClass *>(klass);

    sp_object_class->release = sp_flowregionbreak_release;
    sp_object_class->write = sp_flowregionbreak_write;
    sp_object_class->modified = sp_flowregionbreak_modified;
}

static void sp_flowregionbreak_init(SPFlowregionbreak */*group*/)
{
}

static void sp_flowregionbreak_release(SPObject *object)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowregionbreak_parent_class)->release) {
        reinterpret_cast<SPObjectClass *>(sp_flowregionbreak_parent_class)->release(object);
    }
}

static void sp_flowregionbreak_modified(SPObject *object, guint flags)
{
    if (reinterpret_cast<SPObjectClass *>(sp_flowregionbreak_parent_class)->modified) {
        reinterpret_cast<SPObjectClass *>(sp_flowregionbreak_parent_class)->modified(object, flags);
    }

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    flags &= SP_OBJECT_MODIFIED_CASCADE;
}

static Inkscape::XML::Node *sp_flowregionbreak_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    if ( flags & SP_OBJECT_WRITE_BUILD ) {
        if ( repr == NULL ) {
            repr = xml_doc->createElement("svg:flowLine");
        }
    } else {
    }

    if (reinterpret_cast<SPObjectClass *>(sp_flowregionbreak_parent_class)->write) {
        reinterpret_cast<SPObjectClass *>(sp_flowregionbreak_parent_class)->write(object, xml_doc, repr, flags);
    }

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
