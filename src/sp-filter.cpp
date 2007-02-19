#define __SP_FILTER_CPP__

/** \file
 * SVG <filter> implementation.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "attributes.h"
#include "document.h"
#include "sp-filter.h"
#include "sp-filter-reference.h"
#include "uri.h"
#include "xml/repr.h"

#define SP_MACROS_SILENT
#include "macros.h"


/* Filter base class */

static void sp_filter_class_init(SPFilterClass *klass);
static void sp_filter_init(SPFilter *filter);

static void sp_filter_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_filter_release(SPObject *object);
static void sp_filter_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_filter_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_filter_child_added(SPObject *object,
                                    Inkscape::XML::Node *child,
                                    Inkscape::XML::Node *ref);
static void sp_filter_remove_child(SPObject *object, Inkscape::XML::Node *child);
static Inkscape::XML::Node *sp_filter_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static void filter_ref_changed(SPObject *old_ref, SPObject *ref, SPFilter *filter);
static void filter_ref_modified(SPObject *href, guint flags, SPFilter *filter);

static SPObjectClass *filter_parent_class;

GType
sp_filter_get_type()
{
    static GType filter_type = 0;

    if (!filter_type) {
        GTypeInfo filter_info = {
            sizeof(SPFilterClass),
            NULL, NULL,
            (GClassInitFunc) sp_filter_class_init,
            NULL, NULL,
            sizeof(SPFilter),
            16,
            (GInstanceInitFunc) sp_filter_init,
            NULL,    /* value_table */
        };
        filter_type = g_type_register_static(SP_TYPE_OBJECT, "SPFilter", &filter_info, (GTypeFlags)0);
    }
    return filter_type;
}

static void
sp_filter_class_init(SPFilterClass *klass)
{

    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    filter_parent_class = (SPObjectClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_filter_build;
    sp_object_class->release = sp_filter_release;
    sp_object_class->write = sp_filter_write;
    sp_object_class->set = sp_filter_set;
    sp_object_class->update = sp_filter_update;
    sp_object_class->child_added = sp_filter_child_added;
    sp_object_class->remove_child = sp_filter_remove_child;
}

static void
sp_filter_init(SPFilter *filter)
{
    filter->href = new SPFilterReference(SP_OBJECT(filter));
    filter->href->changedSignal().connect(sigc::bind(sigc::ptr_fun(filter_ref_changed), filter));

    filter->x = 0;
    filter->y = 0;
    filter->width = 0;
    filter->height = 0;

    filter->filterUnits = SP_FILTER_UNITS_OBJECTBOUNDINGBOX;
    filter->primitiveUnits = SP_FILTER_UNITS_OBJECTBOUNDINGBOX;
    filter->filterUnits_set = FALSE;
    filter->primitiveUnits_set = FALSE;
    filter->_primitive_count=0;
    
    filter->_primitive_table_size = 1;
    filter->_primitives = new SPFilterPrimitive*[1];
    filter->_primitives[0] = NULL;

}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFilter variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_filter_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) filter_parent_class)->build) {
        ((SPObjectClass *) filter_parent_class)->build(object, document, repr);
    }

    //Read values of key attributes from XML nodes into object.
    sp_object_read_attr(object, "filterUnits");
    sp_object_read_attr(object, "primitiveUnits");
    sp_object_read_attr(object, "x");
    sp_object_read_attr(object, "y");
    sp_object_read_attr(object, "width");
    sp_object_read_attr(object, "height");
    sp_object_read_attr(object, "filterRes");
    sp_object_read_attr(object, "xlink:href");

//is this necessary?
    sp_document_add_resource(document, "filter", object);
}

/**
 * Drops any allocated memory.
 */
static void
sp_filter_release(SPObject *object)
{
    SPFilter *filter = SP_FILTER(object);

    if (SP_OBJECT_DOCUMENT(object)) {
        /* Unregister ourselves */
        sp_document_remove_resource(SP_OBJECT_DOCUMENT(object), "filter", SP_OBJECT(object));
    }

//TODO: release resources here

    //release href
    if (filter->href) {
        if (filter->href->getObject()) {
            sp_signal_disconnect_by_data(filter->href->getObject(), filter);
        }
        filter->href->detach();
        delete filter->href;
        filter->href = NULL;
    }

    if (((SPObjectClass *) filter_parent_class)->release)
        ((SPObjectClass *) filter_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPFilter.
 */
static void
sp_filter_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFilter *filter = SP_FILTER(object);

    switch (key) {
        case SP_ATTR_FILTERUNITS:
            if (value) {
                if (!strcmp(value, "userSpaceOnUse")) {
                    filter->filterUnits = SP_FILTER_UNITS_USERSPACEONUSE;
                } else {
                    filter->filterUnits = SP_FILTER_UNITS_OBJECTBOUNDINGBOX;
                }
                filter->filterUnits_set = TRUE;
            } else {
                filter->filterUnits = SP_FILTER_UNITS_OBJECTBOUNDINGBOX;
                filter->filterUnits_set = FALSE;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_PRIMITIVEUNITS:
            if (value) {
                if (!strcmp(value, "userSpaceOnUse")) {
                    filter->primitiveUnits = SP_FILTER_UNITS_USERSPACEONUSE;
                } else {
                    filter->primitiveUnits = SP_FILTER_UNITS_OBJECTBOUNDINGBOX;
                }
                filter->primitiveUnits_set = TRUE;
            } else {
                filter->primitiveUnits = SP_FILTER_UNITS_OBJECTBOUNDINGBOX;
                filter->primitiveUnits_set = FALSE;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
    case SP_ATTR_X:
            filter->x.readOrUnset(value);
        object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
    case SP_ATTR_Y:
        filter->y.readOrUnset(value);
        object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
    case SP_ATTR_WIDTH:
        filter->width.readOrUnset(value);
        object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
    case SP_ATTR_HEIGHT:
        filter->height.readOrUnset(value);
        object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
    case SP_ATTR_FILTERRES:
        filter->filterRes.set(value);
            break;
        case SP_ATTR_XLINK_HREF:
            if (value) {
                try {
                    filter->href->attach(Inkscape::URI(value));
                } catch (Inkscape::BadURIException &e) {
                    g_warning("%s", e.what());
                    filter->href->detach();
                }
            } else {
                filter->href->detach();
            }
            break;
        default:
            // See if any parents need this value. 
            if (((SPObjectClass *) filter_parent_class)->set) {
                ((SPObjectClass *) filter_parent_class)->set(object, key, value);
            }
            break;
    }
}

/**
 * Receives update notifications.
 */
static void
sp_filter_update(SPObject *object, SPCtx *ctx, guint flags)
{
    //SPFilter *filter = SP_FILTER(object);

    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) filter_parent_class)->update) {
        ((SPObjectClass *) filter_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_filter_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SPFilter *filter = SP_FILTER(object);

    if (!repr) {
        repr = SP_OBJECT_REPR(object)->duplicate();
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || filter->filterUnits_set) {
        switch (filter->filterUnits) {
            case SP_FILTER_UNITS_USERSPACEONUSE:
                repr->setAttribute("filterUnits", "userSpaceOnUse");
                break;
            default:
                repr->setAttribute("filterUnits", "objectBoundingBox");
                break;
        }
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || filter->primitiveUnits_set) {
        switch (filter->primitiveUnits) {
            case SP_FILTER_UNITS_USERSPACEONUSE:
                repr->setAttribute("primitiveUnits", "userSpaceOnUse");
                break;
            default:
                repr->setAttribute("primitiveUnits", "objectBoundingBox");
                break;
        }
    }

    if (filter->x._set) {
        sp_repr_set_svg_double(repr, "x", filter->x.computed);
    } else {
        repr->setAttribute("x", NULL);
    }

    if (filter->y._set) {
        sp_repr_set_svg_double(repr, "y", filter->y.computed);
    } else {
        repr->setAttribute("y", NULL);
    }

    if (filter->width._set) {
        sp_repr_set_svg_double(repr, "width", filter->width.computed);
    } else {
        repr->setAttribute("width", NULL);
    }

    if (filter->height._set) {
        sp_repr_set_svg_double(repr, "height", filter->height.computed);
    } else {
        repr->setAttribute("height", NULL);
    }

    if (filter->filterRes.getNumber()>=0) {
        gchar *tmp = filter->filterRes.getValueString();
        repr->setAttribute("filterRes", tmp);
        g_free(tmp);
    } else {
        repr->setAttribute("filterRes", NULL);
    }

    if (filter->href->getURI()) {
        gchar *uri_string = filter->href->getURI()->toString();
        repr->setAttribute("xlink:href", uri_string);
        g_free(uri_string);
    }

    if (((SPObjectClass *) filter_parent_class)->write) {
        ((SPObjectClass *) filter_parent_class)->write(object, repr, flags);
    }

    return repr;
}


/**
 * Gets called when the filter is (re)attached to another filter.
 */
static void
filter_ref_changed(SPObject *old_ref, SPObject *ref, SPFilter *filter)
{
    if (old_ref) {
        sp_signal_disconnect_by_data(old_ref, filter);
    }
    if ( SP_IS_FILTER(ref)
         && ref != filter )
    {
        ref->connectModified(sigc::bind(sigc::ptr_fun(&filter_ref_modified), filter));
        //g_signal_connect(G_OBJECT(ref), "modified", G_CALLBACK(filter_ref_modified), filter);
    }

    filter_ref_modified(ref, 0, filter);
}

static void
filter_ref_modified(SPObject *href, guint flags, SPFilter *filter)
{
    SP_OBJECT(filter)->requestModified(SP_OBJECT_MODIFIED_FLAG);
}


void _enlarge_primitive_table(SPFilter * filter) {
    SPFilterPrimitive **new_tbl = new SPFilterPrimitive*[filter->_primitive_table_size * 2];
    for (int i = 0 ; i < filter->_primitive_count ; i++) {
        new_tbl[i] = filter->_primitives[i];
    }
    filter->_primitive_table_size *= 2;
    for (int i = filter->_primitive_count ; i < filter->_primitive_table_size ; i++) {
        new_tbl[i] = NULL;
    }
    delete[] filter->_primitives;
    filter->_primitives = new_tbl;
}

SPFilterPrimitive *add_primitive(SPFilter *filter, SPFilterPrimitive *primitive)
{
    if (filter->_primitive_count >= filter->_primitive_table_size) {
        _enlarge_primitive_table(filter);
    }
    filter->_primitives[filter->_primitive_count] = primitive;
    filter->_primitive_count++;
    return primitive;
}

/**
 * Callback for child_added event.
 */
static void
sp_filter_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{/*
    SPFilter *f = SP_FILTER(object);

    if (((SPObjectClass *) filter_parent_class)->child_added)
        (* ((SPObjectClass *) filter_parent_class)->child_added)(object, child, ref);

    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
	*/
}

/**
 * Callback for remove_child event.
 */
static void
sp_filter_remove_child(SPObject *object, Inkscape::XML::Node *child)
{/*
    SPFilter *f = SP_FILTER(object);

    if (((SPObjectClass *) filter_parent_class)->remove_child)
        (* ((SPObjectClass *) filter_parent_class)->remove_child)(object, child);

    SPObject *ochild;

    
    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
	*/
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
