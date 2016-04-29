/** \file
 * SVG <filter> implementation.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006,2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <map>
#include <string.h>
using std::map;
using std::pair;

#include <glibmm.h>
#include "attributes.h"
#include "display/nr-filter.h"
#include "document.h"
#include "sp-filter.h"
#include "sp-filter-reference.h"
#include "sp-filter-primitive.h"
#include "sp-item.h"
#include "uri.h"
#include "xml/repr.h"
#include <cstring>
#include <string>

#define SP_MACROS_SILENT
#include "macros.h"

static void filter_ref_changed(SPObject *old_ref, SPObject *ref, SPFilter *filter);
static void filter_ref_modified(SPObject *href, guint flags, SPFilter *filter);


SPFilter::SPFilter()
    : SPObject(), filterUnits(SP_FILTER_UNITS_OBJECTBOUNDINGBOX), filterUnits_set(FALSE),
      primitiveUnits(SP_FILTER_UNITS_USERSPACEONUSE), primitiveUnits_set(FALSE),
      filterRes(NumberOptNumber()),
      _renderer(NULL), _image_name(new std::map<gchar *, int, ltstr>), _image_number_next(0)
{
    this->href = new SPFilterReference(this);
    this->href->changedSignal().connect(sigc::bind(sigc::ptr_fun(filter_ref_changed), this));

    this->x = 0;
    this->y = 0;
    this->width = 0;
    this->height = 0;

    this->_image_name->clear();
}

SPFilter::~SPFilter() {
}


/**
 * Reads the Inkscape::XML::Node, and initializes SPFilter variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPFilter::build(SPDocument *document, Inkscape::XML::Node *repr) {
    //Read values of key attributes from XML nodes into object.
    this->readAttr( "style" ); // struct not derived from SPItem, we need to do this ourselves.
    this->readAttr( "filterUnits" );
    this->readAttr( "primitiveUnits" );
    this->readAttr( "x" );
    this->readAttr( "y" );
    this->readAttr( "width" );
    this->readAttr( "height" );
    this->readAttr( "filterRes" );
    this->readAttr( "xlink:href" );
    this->_refcount = 0;

	SPObject::build(document, repr);

//is this necessary?
    document->addResource("filter", this);
}

/**
 * Drops any allocated memory.
 */
void SPFilter::release() {
    if (this->document) {
        // Unregister ourselves
        this->document->removeResource("filter", this);
    }

//TODO: release resources here

    //release href
    if (this->href) {
        this->modified_connection.disconnect();
        this->href->detach();
        delete this->href;
        this->href = NULL;
    }

    for (map<gchar *, int, ltstr>::const_iterator i = this->_image_name->begin() ; i != this->_image_name->end() ; ++i) {
        g_free(i->first);
    }

    delete this->_image_name;

    SPObject::release();
}

/**
 * Sets a specific value in the SPFilter.
 */
void SPFilter::set(unsigned int key, gchar const *value) {
    switch (key) {
        case SP_ATTR_FILTERUNITS:
            if (value) {
                if (!strcmp(value, "userSpaceOnUse")) {
                    this->filterUnits = SP_FILTER_UNITS_USERSPACEONUSE;
                } else {
                    this->filterUnits = SP_FILTER_UNITS_OBJECTBOUNDINGBOX;
                }

                this->filterUnits_set = TRUE;
            } else {
                this->filterUnits = SP_FILTER_UNITS_OBJECTBOUNDINGBOX;
                this->filterUnits_set = FALSE;
            }

            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_PRIMITIVEUNITS:
            if (value) {
                if (!strcmp(value, "objectBoundingBox")) {
                    this->primitiveUnits = SP_FILTER_UNITS_OBJECTBOUNDINGBOX;
                } else {
                    this->primitiveUnits = SP_FILTER_UNITS_USERSPACEONUSE;
                }

                this->primitiveUnits_set = TRUE;
            } else {
                this->primitiveUnits = SP_FILTER_UNITS_USERSPACEONUSE;
                this->primitiveUnits_set = FALSE;
            }

            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_X:
            this->x.readOrUnset(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y:
            this->y.readOrUnset(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_WIDTH:
            this->width.readOrUnset(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_HEIGHT:
            this->height.readOrUnset(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_FILTERRES:
            this->filterRes.set(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_XLINK_HREF:
            if (value) {
                try {
                    this->href->attach(Inkscape::URI(value));
                } catch (Inkscape::BadURIException &e) {
                    g_warning("%s", e.what());
                    this->href->detach();
                }
            } else {
                this->href->detach();
            }
            break;
        default:
            // See if any parents need this value.
        	SPObject::set(key, value);
            break;
    }
}


/**
 * Returns the number of references to the filter.
 */
guint SPFilter::getRefCount() {
	// NOTE: this is currently updated by sp_style_filter_ref_changed() in style.cpp
	return _refcount;
}

/**
 * Receives update notifications.
 */
void SPFilter::update(SPCtx *ctx, guint flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        SPItemCtx *ictx = (SPItemCtx *) ctx;

        // Do here since we know viewport (Bounding box case handled during rendering)
        // Note: This only works for root viewport since this routine is not called after
        // setting a new viewport. A true fix requires a strategy like SPItemView or SPMarkerView.
        if(this->filterUnits == SP_FILTER_UNITS_USERSPACEONUSE) {
            if (this->x.unit == SVGLength::PERCENT) {
                this->x._set = true;
                this->x.computed = this->x.value * ictx->viewport.width();
            }

            if (this->y.unit == SVGLength::PERCENT) {
                this->y._set = true;
                this->y.computed = this->y.value * ictx->viewport.height();
            }

            if (this->width.unit == SVGLength::PERCENT) {
                this->width._set = true;
                this->width.computed = this->width.value * ictx->viewport.width();
            }

            if (this->height.unit == SVGLength::PERCENT) {
                this->height._set = true;
                this->height.computed = this->height.value * ictx->viewport.height();
            }
        }
        /* do something to trigger redisplay, updates? */

    }

    // Update filter primitives in order to update filter primitive area
    // (SPObject::ActionUpdate is not actually used)
    unsigned childflags = flags;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
      childflags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    childflags &= SP_OBJECT_MODIFIED_CASCADE;
    std::vector<SPObject*> l(this->childList(true, SPObject::ActionUpdate));
    for(std::vector<SPObject*>::const_iterator i=l.begin();i!=l.end();++i){
        SPObject *child = *i;
        if( SP_IS_FILTER_PRIMITIVE( child ) ) {
            child->updateDisplay(ctx, childflags);
        }
        sp_object_unref(child);
    }

    SPObject::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPFilter::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    // Original from sp-item-group.cpp
    if (flags & SP_OBJECT_WRITE_BUILD) {
        if (!repr) {
            repr = doc->createElement("svg:filter");
        }

        GSList *l = NULL;
        for ( SPObject *child = this->firstChild(); child; child = child->getNext() ) {
            Inkscape::XML::Node *crepr = child->updateRepr(doc, NULL, flags);

            if (crepr) {
                l = g_slist_prepend (l, crepr);
            }
        }

        while (l) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove (l, l->data);
        }
    } else {
        for ( SPObject *child = this->firstChild() ; child; child = child->getNext() ) {
            child->updateRepr(flags);
        }
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || this->filterUnits_set) {
        switch (this->filterUnits) {
            case SP_FILTER_UNITS_USERSPACEONUSE:
                repr->setAttribute("filterUnits", "userSpaceOnUse");
                break;
            default:
                repr->setAttribute("filterUnits", "objectBoundingBox");
                break;
        }
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || this->primitiveUnits_set) {
        switch (this->primitiveUnits) {
            case SP_FILTER_UNITS_OBJECTBOUNDINGBOX:
                repr->setAttribute("primitiveUnits", "objectBoundingBox");
                break;
            default:
                repr->setAttribute("primitiveUnits", "userSpaceOnUse");
                break;
        }
    }

    if (this->x._set) {
        sp_repr_set_svg_double(repr, "x", this->x.computed);
    } else {
        repr->setAttribute("x", NULL);
    }

    if (this->y._set) {
        sp_repr_set_svg_double(repr, "y", this->y.computed);
    } else {
        repr->setAttribute("y", NULL);
    }

    if (this->width._set) {
        sp_repr_set_svg_double(repr, "width", this->width.computed);
    } else {
        repr->setAttribute("width", NULL);
    }

    if (this->height._set) {
        sp_repr_set_svg_double(repr, "height", this->height.computed);
    } else {
        repr->setAttribute("height", NULL);
    }

    if (this->filterRes.getNumber()>=0) {
        gchar *tmp = this->filterRes.getValueString();
        repr->setAttribute("filterRes", tmp);
        g_free(tmp);
    } else {
        repr->setAttribute("filterRes", NULL);
    }

    if (this->href->getURI()) {
        gchar *uri_string = this->href->getURI()->toString();
        repr->setAttribute("xlink:href", uri_string);
        g_free(uri_string);
    }

    SPObject::write(doc, repr, flags);

    return repr;
}


/**
 * Gets called when the filter is (re)attached to another filter.
 */
static void
filter_ref_changed(SPObject *old_ref, SPObject *ref, SPFilter *filter)
{
    if (old_ref) {
        filter->modified_connection.disconnect();
    }

    if ( SP_IS_FILTER(ref)
         && ref != filter )
    {
        filter->modified_connection =
            ref->connectModified(sigc::bind(sigc::ptr_fun(&filter_ref_modified), filter));
    }

    filter_ref_modified(ref, 0, filter);
}

static void filter_ref_modified(SPObject */*href*/, guint /*flags*/, SPFilter *filter)
{
    filter->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Callback for child_added event.
 */
void SPFilter::child_added(Inkscape::XML::Node *child, Inkscape::XML::Node *ref) {
	SPObject::child_added(child, ref);

    this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Callback for remove_child event.
 */
void SPFilter::remove_child(Inkscape::XML::Node *child) {
	SPObject::remove_child(child);

	this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

void sp_filter_build_renderer(SPFilter *sp_filter, Inkscape::Filters::Filter *nr_filter)
{
    g_assert(sp_filter != NULL);
    g_assert(nr_filter != NULL);

    sp_filter->_renderer = nr_filter;

    nr_filter->set_filter_units(sp_filter->filterUnits);
    nr_filter->set_primitive_units(sp_filter->primitiveUnits);
    nr_filter->set_x(sp_filter->x);
    nr_filter->set_y(sp_filter->y);
    nr_filter->set_width(sp_filter->width);
    nr_filter->set_height(sp_filter->height);

    if (sp_filter->filterRes.getNumber() >= 0) {
        if (sp_filter->filterRes.getOptNumber() >= 0) {
            nr_filter->set_resolution(sp_filter->filterRes.getNumber(),
                                      sp_filter->filterRes.getOptNumber());
        } else {
            nr_filter->set_resolution(sp_filter->filterRes.getNumber());
        }
    }

    nr_filter->clear_primitives();
    SPObject *primitive_obj = sp_filter->children;
    while (primitive_obj) {
        if (SP_IS_FILTER_PRIMITIVE(primitive_obj)) {
            SPFilterPrimitive *primitive = SP_FILTER_PRIMITIVE(primitive_obj);
            g_assert(primitive != NULL);

//            if (((SPFilterPrimitiveClass*) G_OBJECT_GET_CLASS(primitive))->build_renderer) {
//                ((SPFilterPrimitiveClass *) G_OBJECT_GET_CLASS(primitive))->build_renderer(primitive, nr_filter);
//            } else {
//                g_warning("Cannot build filter renderer: missing builder");
//            }  // CPPIFY: => FilterPrimitive should be abstract.
            primitive->build_renderer(nr_filter);
        }
        primitive_obj = primitive_obj->next;
    }
}

int sp_filter_primitive_count(SPFilter *filter) {
    g_assert(filter != NULL);
    int count = 0;

    SPObject *primitive_obj = filter->children;
    while (primitive_obj) {
        if (SP_IS_FILTER_PRIMITIVE(primitive_obj)) count++;
        primitive_obj = primitive_obj->next;
    }
    return count;
}

int sp_filter_get_image_name(SPFilter *filter, gchar const *name) {
    gchar *name_copy = strdup(name);
    map<gchar *, int, ltstr>::iterator result = filter->_image_name->find(name_copy);
    free(name_copy);
    if (result == filter->_image_name->end()) return -1;
    else return (*result).second;
}

int sp_filter_set_image_name(SPFilter *filter, gchar const *name) {
    int value = filter->_image_number_next;
    filter->_image_number_next++;
    gchar *name_copy = strdup(name);
    pair<gchar*,int> new_pair(name_copy, value);
    pair<map<gchar*,int,ltstr>::iterator,bool> ret = filter->_image_name->insert(new_pair);
    if (ret.second == false) {
        // The element is not inserted (because an element with the same key was already in the map) 
        // Therefore, free the memory allocated for the new entry:
        free(name_copy);

        return (*ret.first).second;
    }
    return value;
}

gchar const *sp_filter_name_for_image(SPFilter const *filter, int const image) {
    switch (image) {
        case Inkscape::Filters::NR_FILTER_SOURCEGRAPHIC:
            return "SourceGraphic";
            break;
        case Inkscape::Filters::NR_FILTER_SOURCEALPHA:
            return "SourceAlpha";
            break;
        case Inkscape::Filters::NR_FILTER_BACKGROUNDIMAGE:
            return "BackgroundImage";
            break;
        case Inkscape::Filters::NR_FILTER_BACKGROUNDALPHA:
            return "BackgroundAlpha";
            break;
        case Inkscape::Filters::NR_FILTER_STROKEPAINT:
            return "StrokePaint";
            break;
        case Inkscape::Filters::NR_FILTER_FILLPAINT:
            return "FillPaint";
            break;
        case Inkscape::Filters::NR_FILTER_SLOT_NOT_SET:
        case Inkscape::Filters::NR_FILTER_UNNAMED_SLOT:
            return 0;
            break;
        default:
            for (map<gchar *, int, ltstr>::const_iterator i
                     = filter->_image_name->begin() ;
                 i != filter->_image_name->end() ; ++i) {
                if (i->second == image) {
                    return i->first;
                }
            }
    }
    return 0;
}

Glib::ustring sp_filter_get_new_result_name(SPFilter *filter) {
    g_assert(filter != NULL);
    int largest = 0;

    SPObject *primitive_obj = filter->children;
    while (primitive_obj) {
        if (SP_IS_FILTER_PRIMITIVE(primitive_obj)) {
            Inkscape::XML::Node *repr = primitive_obj->getRepr();
            char const *result = repr->attribute("result");
            int index;
            if (result)
            {
                if (sscanf(result, "result%5d", &index) == 1)
                {
                    if (index > largest)
                    {
                        largest = index;
                    }
                }
            }
        }
        primitive_obj = primitive_obj->next;
    }

    return "result" + Glib::Ascii::dtostr(largest + 1);
}

bool ltstr::operator()(const char* s1, const char* s2) const
{
    return strcmp(s1, s2) < 0;
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
