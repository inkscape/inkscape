#define __SP_FLOWDIV_C__

/*
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "libnr/nr-matrix-ops.h"
#include "xml/repr.h"
//#include "svg/svg.h"

//#include "style.h"

#include "sp-flowdiv.h"
#include "sp-string.h"
#include "document.h"

static void sp_flowdiv_class_init (SPFlowdivClass *klass);
static void sp_flowdiv_init (SPFlowdiv *group);
static void sp_flowdiv_release (SPObject *object);
static Inkscape::XML::Node *sp_flowdiv_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_flowdiv_update (SPObject *object, SPCtx *ctx, unsigned int flags);
static void sp_flowdiv_modified (SPObject *object, guint flags);
static void sp_flowdiv_build (SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr);
static void sp_flowdiv_set (SPObject *object, unsigned int key, const gchar *value);

static void sp_flowtspan_class_init (SPFlowtspanClass *klass);
static void sp_flowtspan_init (SPFlowtspan *group);
static void sp_flowtspan_release (SPObject *object);
static Inkscape::XML::Node *sp_flowtspan_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_flowtspan_update (SPObject *object, SPCtx *ctx, unsigned int flags);
static void sp_flowtspan_modified (SPObject *object, guint flags);
static void sp_flowtspan_build (SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr);
static void sp_flowtspan_set (SPObject *object, unsigned int key, const gchar *value);

static void sp_flowpara_class_init (SPFlowparaClass *klass);
static void sp_flowpara_init (SPFlowpara *group);
static void sp_flowpara_release (SPObject *object);
static Inkscape::XML::Node *sp_flowpara_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_flowpara_update (SPObject *object, SPCtx *ctx, unsigned int flags);
static void sp_flowpara_modified (SPObject *object, guint flags);
static void sp_flowpara_build (SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr);
static void sp_flowpara_set (SPObject *object, unsigned int key, const gchar *value);

static void sp_flowline_class_init (SPFlowlineClass *klass);
static void sp_flowline_release (SPObject *object);
static void sp_flowline_init (SPFlowline *group);
static Inkscape::XML::Node *sp_flowline_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_flowline_modified (SPObject *object, guint flags);

static void sp_flowregionbreak_class_init (SPFlowregionbreakClass *klass);
static void sp_flowregionbreak_release (SPObject *object);
static void sp_flowregionbreak_init (SPFlowregionbreak *group);
static Inkscape::XML::Node *sp_flowregionbreak_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_flowregionbreak_modified (SPObject *object, guint flags);

static SPItemClass * flowdiv_parent_class;
static SPItemClass * flowtspan_parent_class;
static SPItemClass * flowpara_parent_class;
static SPObjectClass * flowline_parent_class;
static SPObjectClass * flowregionbreak_parent_class;

GType
sp_flowdiv_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPFlowdivClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_flowdiv_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPFlowdiv),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_flowdiv_init,
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_ITEM, "SPFlowdiv", &group_info, (GTypeFlags)0);
	}
	return group_type;
}

static void
sp_flowdiv_class_init (SPFlowdivClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;

	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;

	flowdiv_parent_class = (SPItemClass *)g_type_class_ref (SP_TYPE_ITEM);

	sp_object_class->build = sp_flowdiv_build;
	sp_object_class->set = sp_flowdiv_set;
	sp_object_class->release = sp_flowdiv_release;
	sp_object_class->write = sp_flowdiv_write;
	sp_object_class->update = sp_flowdiv_update;
	sp_object_class->modified = sp_flowdiv_modified;
}

static void
sp_flowdiv_init (SPFlowdiv */*group*/)
{
}

static void
sp_flowdiv_release (SPObject *object)
{
	if (((SPObjectClass *) flowdiv_parent_class)->release)
		((SPObjectClass *) flowdiv_parent_class)->release (object);
}
static void
sp_flowdiv_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPItemCtx *ictx=(SPItemCtx *) ctx;
	SPItemCtx cctx=*ictx;

	if (((SPObjectClass *) (flowdiv_parent_class))->update)
		((SPObjectClass *) (flowdiv_parent_class))->update (object, ctx, flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	GSList* l = NULL;
	for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		SPObject *child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
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
		g_object_unref (G_OBJECT (child));
	}
}
static void
sp_flowdiv_modified (SPObject *object, guint flags)
{
	SPObject *child;
	GSList *l;

	if (((SPObjectClass *) (flowdiv_parent_class))->modified)
		((SPObjectClass *) (flowdiv_parent_class))->modified (object, flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	l = NULL;
	for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			child->emitModified(flags);
		}
		g_object_unref (G_OBJECT (child));
	}
}

static void
sp_flowdiv_build (SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr)
{
	object->_requireSVGVersion(Inkscape::Version(1, 2));

	if (((SPObjectClass *) flowdiv_parent_class)->build)
		((SPObjectClass *) flowdiv_parent_class)->build (object, doc, repr);
}

static void
sp_flowdiv_set (SPObject *object, unsigned int key, const gchar *value)
{
	if (((SPObjectClass *) flowdiv_parent_class)->set)
		(((SPObjectClass *) flowdiv_parent_class)->set) (object, key, value);
}

static Inkscape::XML::Node *
sp_flowdiv_write (SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
//	SPFlowdiv *group = SP_FLOWDIV (object);

	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		if ( repr == NULL ) repr = xml_doc->createElement("svg:flowDiv");
		GSList *l = NULL;
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			Inkscape::XML::Node* c_repr=NULL;
			if ( SP_IS_FLOWTSPAN (child) ) {
				c_repr = child->updateRepr(xml_doc, NULL, flags);
			} else if ( SP_IS_FLOWPARA(child) ) {
				c_repr = child->updateRepr(xml_doc, NULL, flags);
			} else if ( SP_IS_STRING(child) ) {
				c_repr = xml_doc->createTextNode(SP_STRING(child)->string.c_str());
			}
			if ( c_repr ) l = g_slist_prepend (l, c_repr);
		}
		while ( l ) {
			repr->addChild((Inkscape::XML::Node *) l->data, NULL);
			Inkscape::GC::release((Inkscape::XML::Node *) l->data);
			l = g_slist_remove (l, l->data);
		}
	} else {
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			if ( SP_IS_FLOWTSPAN (child) ) {
				child->updateRepr(flags);
			} else if ( SP_IS_FLOWPARA(child) ) {
				child->updateRepr(flags);
			} else if ( SP_IS_STRING(child) ) {
				SP_OBJECT_REPR (child)->setContent(SP_STRING(child)->string.c_str());
			}
		}
	}

	if (((SPObjectClass *) (flowdiv_parent_class))->write)
		((SPObjectClass *) (flowdiv_parent_class))->write (object, xml_doc, repr, flags);

	return repr;
}


/*
 *
 */

GType
sp_flowtspan_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPFlowtspanClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_flowtspan_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPFlowtspan),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_flowtspan_init,
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_ITEM, "SPFlowtspan", &group_info, (GTypeFlags)0);
	}
	return group_type;
}

static void
sp_flowtspan_class_init (SPFlowtspanClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;

	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;

	flowtspan_parent_class = (SPItemClass *)g_type_class_ref (SP_TYPE_ITEM);

	sp_object_class->build = sp_flowtspan_build;
	sp_object_class->set = sp_flowtspan_set;
	sp_object_class->release = sp_flowtspan_release;
	sp_object_class->write = sp_flowtspan_write;
	sp_object_class->update = sp_flowtspan_update;
	sp_object_class->modified = sp_flowtspan_modified;
}

static void
sp_flowtspan_init (SPFlowtspan */*group*/)
{
}

static void
sp_flowtspan_release (SPObject *object)
{
	if (((SPObjectClass *) flowtspan_parent_class)->release)
		((SPObjectClass *) flowtspan_parent_class)->release (object);
}
static void
sp_flowtspan_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
//	SPFlowtspan *group=SP_FLOWTSPAN (object);
	SPItemCtx *ictx=(SPItemCtx *) ctx;
	SPItemCtx cctx=*ictx;

	if (((SPObjectClass *) (flowtspan_parent_class))->update)
		((SPObjectClass *) (flowtspan_parent_class))->update (object, ctx, flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	GSList* l = NULL;
	for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		SPObject *child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
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
		g_object_unref (G_OBJECT (child));
	}
}
static void
sp_flowtspan_modified (SPObject *object, guint flags)
{
	SPObject *child;
	GSList *l;

	if (((SPObjectClass *) (flowtspan_parent_class))->modified)
		((SPObjectClass *) (flowtspan_parent_class))->modified (object, flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	l = NULL;
	for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			child->emitModified(flags);
		}
		g_object_unref (G_OBJECT (child));
	}
}
static void
sp_flowtspan_build (SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr)
{
	if (((SPObjectClass *) flowtspan_parent_class)->build)
		((SPObjectClass *) flowtspan_parent_class)->build (object, doc, repr);
}
static void
sp_flowtspan_set (SPObject *object, unsigned int key, const gchar *value)
{
	if (((SPObjectClass *) flowtspan_parent_class)->set)
		(((SPObjectClass *) flowtspan_parent_class)->set) (object, key, value);
}
static Inkscape::XML::Node *
sp_flowtspan_write (SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		if ( repr == NULL ) repr = xml_doc->createElement("svg:flowSpan");
		GSList *l = NULL;
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			Inkscape::XML::Node* c_repr=NULL;
			if ( SP_IS_FLOWTSPAN (child) ) {
				c_repr = child->updateRepr(xml_doc, NULL, flags);
			} else if ( SP_IS_FLOWPARA (child) ) {
					c_repr = child->updateRepr(xml_doc, NULL, flags);
			} else if ( SP_IS_STRING(child) ) {
				c_repr = xml_doc->createTextNode(SP_STRING(child)->string.c_str());
			}
			if ( c_repr ) l = g_slist_prepend (l, c_repr);
		}
		while ( l ) {
			repr->addChild((Inkscape::XML::Node *) l->data, NULL);
			Inkscape::GC::release((Inkscape::XML::Node *) l->data);
			l = g_slist_remove (l, l->data);
		}
	} else {
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			if ( SP_IS_FLOWTSPAN (child) ) {
				child->updateRepr(flags);
			} else if ( SP_IS_FLOWPARA (child) ) {
					child->updateRepr(flags);
			} else if ( SP_IS_STRING(child) ) {
				SP_OBJECT_REPR (child)->setContent(SP_STRING(child)->string.c_str());
			}
		}
	}

	if (((SPObjectClass *) (flowtspan_parent_class))->write)
		((SPObjectClass *) (flowtspan_parent_class))->write (object, xml_doc, repr, flags);

	return repr;
}



/*
 *
 */

GType
sp_flowpara_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPFlowparaClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_flowpara_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPFlowpara),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_flowpara_init,
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_ITEM, "SPFlowpara", &group_info, (GTypeFlags)0);
	}
	return group_type;
}

static void
sp_flowpara_class_init (SPFlowparaClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;

	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;

	flowpara_parent_class = (SPItemClass *)g_type_class_ref (SP_TYPE_ITEM);

	sp_object_class->build = sp_flowpara_build;
	sp_object_class->set = sp_flowpara_set;
	sp_object_class->release = sp_flowpara_release;
	sp_object_class->write = sp_flowpara_write;
	sp_object_class->update = sp_flowpara_update;
	sp_object_class->modified = sp_flowpara_modified;
}

static void
sp_flowpara_init (SPFlowpara */*group*/)
{
}
static void
sp_flowpara_release (SPObject *object)
{
	if (((SPObjectClass *) flowpara_parent_class)->release)
		((SPObjectClass *) flowpara_parent_class)->release (object);
}

static void
sp_flowpara_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPItemCtx *ictx=(SPItemCtx *) ctx;
	SPItemCtx cctx=*ictx;

	if (((SPObjectClass *) (flowpara_parent_class))->update)
		((SPObjectClass *) (flowpara_parent_class))->update (object, ctx, flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	GSList* l = NULL;
	for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		SPObject *child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
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
		g_object_unref (G_OBJECT (child));
	}
}
static void
sp_flowpara_modified (SPObject *object, guint flags)
{
	SPObject *child;
	GSList *l;

	if (((SPObjectClass *) (flowpara_parent_class))->modified)
		((SPObjectClass *) (flowpara_parent_class))->modified (object, flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	l = NULL;
	for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			child->emitModified(flags);
		}
		g_object_unref (G_OBJECT (child));
	}
}
static void
sp_flowpara_build (SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr)
{
	if (((SPObjectClass *) flowpara_parent_class)->build)
		((SPObjectClass *) flowpara_parent_class)->build (object, doc, repr);
}
static void
sp_flowpara_set (SPObject *object, unsigned int key, const gchar *value)
{
	if (((SPObjectClass *) flowpara_parent_class)->set)
		(((SPObjectClass *) flowpara_parent_class)->set) (object, key, value);
}
static Inkscape::XML::Node *
sp_flowpara_write (SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
	//	SPFlowpara *group = SP_FLOWPARA (object);

	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		if ( repr == NULL ) repr = xml_doc->createElement("svg:flowPara");
		GSList *l = NULL;
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			Inkscape::XML::Node* c_repr=NULL;
			if ( SP_IS_FLOWTSPAN (child) ) {
				c_repr = child->updateRepr(xml_doc, NULL, flags);
			} else if ( SP_IS_FLOWPARA (child) ) {
				c_repr = child->updateRepr(xml_doc, NULL, flags);
			} else if ( SP_IS_STRING(child) ) {
				c_repr = xml_doc->createTextNode(SP_STRING(child)->string.c_str());
			}
			if ( c_repr ) l = g_slist_prepend (l, c_repr);
		}
		while ( l ) {
			repr->addChild((Inkscape::XML::Node *) l->data, NULL);
			Inkscape::GC::release((Inkscape::XML::Node *) l->data);
			l = g_slist_remove (l, l->data);
		}
	} else {
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			if ( SP_IS_FLOWTSPAN (child) ) {
				child->updateRepr(flags);
			} else if ( SP_IS_FLOWPARA (child) ) {
				child->updateRepr(flags);
			} else if ( SP_IS_STRING(child) ) {
				SP_OBJECT_REPR (child)->setContent(SP_STRING(child)->string.c_str());
			}
		}
	}

	if (((SPObjectClass *) (flowpara_parent_class))->write)
		((SPObjectClass *) (flowpara_parent_class))->write (object, xml_doc, repr, flags);

	return repr;
}

/*
 *
 */

GType
sp_flowline_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPFlowlineClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_flowline_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPFlowline),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_flowline_init,
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_OBJECT, "SPFlowline", &group_info, (GTypeFlags)0);
	}
	return group_type;
}

static void
sp_flowline_class_init (SPFlowlineClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;

	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;

	flowline_parent_class = (SPObjectClass *)g_type_class_ref (SP_TYPE_OBJECT);

	sp_object_class->release = sp_flowline_release;
	sp_object_class->write = sp_flowline_write;
	sp_object_class->modified = sp_flowline_modified;
}

static void
sp_flowline_init (SPFlowline */*group*/)
{
}
static void
sp_flowline_release (SPObject *object)
{
	if (((SPObjectClass *) flowline_parent_class)->release)
		((SPObjectClass *) flowline_parent_class)->release (object);
}

static void
sp_flowline_modified (SPObject *object, guint flags)
{
	if (((SPObjectClass *) (flowline_parent_class))->modified)
		((SPObjectClass *) (flowline_parent_class))->modified (object, flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
}
static Inkscape::XML::Node *
sp_flowline_write (SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		if ( repr == NULL ) repr = xml_doc->createElement("svg:flowLine");
	} else {
	}

	if (((SPObjectClass *) (flowline_parent_class))->write)
		((SPObjectClass *) (flowline_parent_class))->write (object, xml_doc, repr, flags);

	return repr;
}

/*
 *
 */

GType
sp_flowregionbreak_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPFlowregionbreakClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_flowregionbreak_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPFlowregionbreak),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_flowregionbreak_init,
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_OBJECT, "SPFlowregionbreak", &group_info, (GTypeFlags)0);
	}
	return group_type;
}

static void
sp_flowregionbreak_class_init (SPFlowregionbreakClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;

	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;

	flowregionbreak_parent_class = (SPObjectClass *)g_type_class_ref (SP_TYPE_OBJECT);

	sp_object_class->release = sp_flowregionbreak_release;
	sp_object_class->write = sp_flowregionbreak_write;
	sp_object_class->modified = sp_flowregionbreak_modified;
}

static void
sp_flowregionbreak_init (SPFlowregionbreak */*group*/)
{
}
static void
sp_flowregionbreak_release (SPObject *object)
{
	if (((SPObjectClass *) flowregionbreak_parent_class)->release)
		((SPObjectClass *) flowregionbreak_parent_class)->release (object);
}

static void
sp_flowregionbreak_modified (SPObject *object, guint flags)
{
	if (((SPObjectClass *) (flowregionbreak_parent_class))->modified)
		((SPObjectClass *) (flowregionbreak_parent_class))->modified (object, flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
}
static Inkscape::XML::Node *
sp_flowregionbreak_write (SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		if ( repr == NULL ) repr = xml_doc->createElement("svg:flowLine");
	} else {
	}

	if (((SPObjectClass *) (flowregionbreak_parent_class))->write)
		((SPObjectClass *) (flowregionbreak_parent_class))->write (object, xml_doc, repr, flags);

	return repr;
}
