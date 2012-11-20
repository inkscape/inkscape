/** \file
 * SPGradient, SPStop, SPLinearGradient, SPRadialGradient,
 * SPMeshGradient, SPMeshRow, SPMeshPatch
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2009 Jasper van de Gronde
 * Copyright (C) 2011 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#define noSP_GRADIENT_VERBOSE

#include <cstring>
#include <string>

#include <2geom/transforms.h>

#include <cairo.h>

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#include "display/cairo-utils.h"
#include "svg/svg.h"
#include "svg/svg-color.h"
#include "svg/css-ostringstream.h"
#include "attributes.h"
#include "document-private.h"
#include "sp-gradient.h"
#include "gradient-chemistry.h"
#include "sp-gradient-reference.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-mesh-gradient.h"
#include "sp-mesh-row.h"
#include "sp-mesh-patch.h"
#include "sp-stop.h"
#include "streq.h"
#include "uri.h"
#include "xml/repr.h"
#include "style.h"
#include "display/grayscale.h"

#define SP_MACROS_SILENT
#include "macros.h"

/// Has to be power of 2   Seems to be unused.
//#define NCOLORS NR_GRADIENT_VECTOR_LENGTH

// SPStop
static void sp_stop_class_init(SPStopClass *klass);
static void sp_stop_init(SPStop *stop);

static void sp_stop_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_stop_set(SPObject *object, unsigned key, gchar const *value);
static Inkscape::XML::Node *sp_stop_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *stop_parent_class;


// SPMeshRow
static void sp_meshrow_class_init(SPMeshRowClass *klass);
static void sp_meshrow_init(SPMeshRow *meshrow);

static void sp_meshrow_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_meshrow_set(SPObject *object, unsigned key, gchar const *value);
static Inkscape::XML::Node *sp_meshrow_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *meshrow_parent_class;


// SPMeshPatch
static void sp_meshpatch_class_init(SPMeshPatchClass *klass);
static void sp_meshpatch_init(SPMeshPatch *meshpatch);

static void sp_meshpatch_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_meshpatch_set(SPObject *object, unsigned key, gchar const *value);
static Inkscape::XML::Node *sp_meshpatch_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *meshpatch_parent_class;


class SPGradientImpl
{
    friend class SPGradient;

    static void classInit(SPGradientClass *klass);

    static void init(SPGradient *gr);
    static void build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
    static void release(SPObject *object);
    static void modified(SPObject *object, guint flags);
    static Inkscape::XML::Node *write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags);

    static void gradientRefModified(SPObject *href, guint flags, SPGradient *gradient);
    static void gradientRefChanged(SPObject *old_ref, SPObject *ref, SPGradient *gr);

    static void childAdded(SPObject *object,
                           Inkscape::XML::Node *child,
                           Inkscape::XML::Node *ref);
    static void removeChild(SPObject *object, Inkscape::XML::Node *child);

    static void setGradientAttr(SPObject *object, unsigned key, gchar const *value);
};

/**
 * Registers SPStop class and returns its type.
 */
GType
sp_stop_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPStopClass),
            NULL, NULL,
            (GClassInitFunc) sp_stop_class_init,
            NULL, NULL,
            sizeof(SPStop),
            16,
            (GInstanceInitFunc) sp_stop_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPStop", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * Callback to initialize SPStop vtable.
 */
static void sp_stop_class_init(SPStopClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    stop_parent_class = (SPObjectClass *) g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->build = sp_stop_build;
    sp_object_class->set = sp_stop_set;
    sp_object_class->write = sp_stop_write;
}

/**
 * Callback to initialize SPStop object.
 */
static void
sp_stop_init(SPStop *stop)
{
    stop->offset = 0.0;
    stop->currentColor = false;
    stop->specified_color.set( 0x000000ff );
    stop->opacity = 1.0;
}

/**
 * Virtual build: set stop attributes from its associated XML node.
 */
static void sp_stop_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) stop_parent_class)->build)
        (* ((SPObjectClass *) stop_parent_class)->build)(object, document, repr);

    object->readAttr( "offset" );
    object->readAttr( "stop-color" );
    object->readAttr( "stop-opacity" );
    object->readAttr( "style" );
    object->readAttr( "path" ); // For mesh
}

/**
 * Virtual set: set attribute to value.
 */
static void
sp_stop_set(SPObject *object, unsigned key, gchar const *value)
{
    SPStop *stop = SP_STOP(object);

    switch (key) {
        case SP_ATTR_STYLE: {
        /** \todo
         * fixme: We are reading simple values 3 times during build (Lauris).
         * \par
         * We need presentation attributes etc.
         * \par
         * remove the hackish "style reading" from here: see comments in
         * sp_object_get_style_property about the bugs in our current
         * approach.  However, note that SPStyle doesn't currently have
         * stop-color and stop-opacity properties.
         */
            {
                gchar const *p = object->getStyleProperty( "stop-color", "black");
                if (streq(p, "currentColor")) {
                    stop->currentColor = true;
                } else {
                    stop->specified_color = SPStop::readStopColor( p );
                }
            }
            {
                gchar const *p = object->getStyleProperty( "stop-opacity", "1");
                gdouble opacity = sp_svg_read_percentage(p, stop->opacity);
                stop->opacity = opacity;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            break;
        }
        case SP_PROP_STOP_COLOR: {
            {
                gchar const *p = object->getStyleProperty( "stop-color", "black");
                if (streq(p, "currentColor")) {
                    stop->currentColor = true;
                } else {
                    stop->currentColor = false;
                    stop->specified_color = SPStop::readStopColor( p );
                }
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            break;
        }
        case SP_PROP_STOP_OPACITY: {
            {
                gchar const *p = object->getStyleProperty( "stop-opacity", "1");
                gdouble opacity = sp_svg_read_percentage(p, stop->opacity);
                stop->opacity = opacity;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_OFFSET: {
            stop->offset = sp_svg_read_percentage(value, 0.0);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            break;
        }
        case SP_PROP_STOP_PATH: {
            if (value) {
                stop->path_string = new Glib::ustring( value );
                //Geom::PathVector pv = sp_svg_read_pathv(value);
                //SPCurve *curve = new SPCurve(pv);
                //if( curve ) {
                    // std::cout << "Got Curve" << std::endl;
                    //curve->unref();
                //}
            }
            break;
        }
        default: {
            if (((SPObjectClass *) stop_parent_class)->set)
                (* ((SPObjectClass *) stop_parent_class)->set)(object, key, value);
            break;
        }
    }
}

/**
 * Virtual write: write object attributes to repr.
 */
static Inkscape::XML::Node *
sp_stop_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPStop *stop = SP_STOP(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:stop");
    }

    Glib::ustring colorStr = stop->specified_color.toString();
    gfloat opacity = stop->opacity;

    if (((SPObjectClass *) stop_parent_class)->write) {
        (* ((SPObjectClass *) stop_parent_class)->write)(object, xml_doc, repr, flags);
    }

    // Since we do a hackish style setting here (because SPStyle does not support stop-color and
    // stop-opacity), we must do it AFTER calling the parent write method; otherwise
    // sp_object_write would clear our style= attribute (bug 1695287)

    Inkscape::CSSOStringStream os;
    os << "stop-color:";
    if (stop->currentColor) {
        os << "currentColor";
    } else {
        os << colorStr;
    }
    os << ";stop-opacity:" << opacity;
    repr->setAttribute("style", os.str().c_str());
    repr->setAttribute("stop-color", NULL);
    repr->setAttribute("stop-opacity", NULL);
    sp_repr_set_css_double(repr, "offset", stop->offset);
    /* strictly speaking, offset an SVG <number> rather than a CSS one, but exponents make no sense
     * for offset proportions. */

    return repr;
}


bool SPGradient::hasStops() const
{
    return has_stops;
}

bool SPGradient::hasPatches() const
{
    return has_patches;
}

bool SPGradient::isUnitsSet() const
{
    return units_set;
}

SPGradientUnits SPGradient::getUnits() const
{
    return units;
}

bool SPGradient::isSpreadSet() const
{
    return spread_set;
}

SPGradientSpread SPGradient::getSpread() const
{
    return spread;
}

void SPGradient::setSwatch( bool swatch )
{
    if ( swatch != isSwatch() ) {
        this->swatch = swatch; // to make isSolid() work, this happens first
        gchar const* paintVal = swatch ? (isSolid() ? "solid" : "gradient") : 0;
        setAttribute( "osb:paint", paintVal, 0 );

        requestModified( SP_OBJECT_MODIFIED_FLAG );
    }
}


/**
 * return true if this gradient is "equivalent" to that gradient.
 * Equivalent meaning they have the same stop count, same stop colors and same stop opacity
 * @param that - A gradient to compare this to
 */
gboolean SPGradient::isEquivalent(SPGradient *that)
{
    //TODO Make this work for mesh gradients

    if (this->getStopCount() != that->getStopCount())
        return FALSE;

    if (this->hasStops() != that->hasStops())
        return FALSE;

    if (!this->getVector() || !that->getVector())
        return FALSE;

    SPStop *as = this->getVector()->getFirstStop();
    SPStop *bs = that->getVector()->getFirstStop();

    while (as && bs) {
        if (!as->getEffectiveColor().isClose(bs->getEffectiveColor(), 0.001) ||
                as->offset != bs->offset) {
            return FALSE;
        }
        as = as->getNextStop();
        bs = bs->getNextStop();
    }

    return TRUE;
}


/**
 * Return stop's color as 32bit value.
 */
guint32
sp_stop_get_rgba32(SPStop const *const stop)
{
    guint32 rgb0 = 0;
    /* Default value: arbitrarily black.  (SVG1.1 and CSS2 both say that the initial
     * value depends on user agent, and don't give any further restrictions that I can
     * see.) */
    if (stop->currentColor) {
        char const *str = stop->getStyleProperty( "color", NULL);
        if (str) {
            rgb0 = sp_svg_read_color(str, rgb0);
        }
        unsigned const alpha = static_cast<unsigned>(stop->opacity * 0xff + 0.5);
        g_return_val_if_fail((alpha & ~0xff) == 0,
                             rgb0 | 0xff);
        return rgb0 | alpha;
    } else {
        return stop->specified_color.toRGBA32( stop->opacity );
    }
}

/*
 * Mesh Row
 */

/**
 * Registers SPMeshRow class and returns its type.
 */
GType
sp_meshrow_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPMeshRowClass),
            NULL, NULL,
            (GClassInitFunc) sp_meshrow_class_init,
            NULL, NULL,
            sizeof(SPMeshRow),
            16,
            (GInstanceInitFunc) sp_meshrow_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPMeshRow", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * Callback to initialize SPMeshRow vtable.
 */
static void sp_meshrow_class_init(SPMeshRowClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    meshrow_parent_class = (SPObjectClass *) g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->build = sp_meshrow_build;
    sp_object_class->set = sp_meshrow_set;
    sp_object_class->write = sp_meshrow_write;
}

/**
 * Callback to initialize SPMeshRow object.
 */
static void sp_meshrow_init(SPMeshRow * /*meshrow*/)
{
    // Do nothing
}

/**
 * Virtual build: set meshrow attributes from its associated XML node.
 */
static void sp_meshrow_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) meshrow_parent_class)->build)
        (* ((SPObjectClass *) meshrow_parent_class)->build)(object, document, repr);

    // No attributes
}

/**
 * Virtual set: set attribute to value.
 */
static void sp_meshrow_set(SPObject * /*object*/, unsigned /*key*/, gchar const * /*value*/)
{
    // Do nothing
}

/**
 * Virtual write: write object attributes to repr.
 */
static Inkscape::XML::Node *
sp_meshrow_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    //SPMeshRow *meshrow = SP_MESHROW(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:meshRow");
    }

    if (((SPObjectClass *) meshrow_parent_class)->write) {
        (* ((SPObjectClass *) meshrow_parent_class)->write)(object, xml_doc, repr, flags);
    }

    return repr;
}

/*
 * Mesh Patch
 */

/**
 * Registers SPMeshPatch class and returns its type.
 */
GType
sp_meshpatch_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPMeshPatchClass),
            NULL, NULL,
            (GClassInitFunc) sp_meshpatch_class_init,
            NULL, NULL,
            sizeof(SPMeshPatch),
            16,
            (GInstanceInitFunc) sp_meshpatch_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPMeshPatch", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * Callback to initialize SPMeshPatch vtable.
 */
static void sp_meshpatch_class_init(SPMeshPatchClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    meshpatch_parent_class = (SPObjectClass *) g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->build = sp_meshpatch_build;
    sp_object_class->set = sp_meshpatch_set;
    sp_object_class->write = sp_meshpatch_write;
}

/**
 * Callback to initialize SPMeshPatch object.
 */
static void sp_meshpatch_init(SPMeshPatch * /*meshpatch*/)
{
    // Do nothing
}

/**
 * Virtual build: set meshpatch attributes from its associated XML node.
 */
static void sp_meshpatch_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) meshpatch_parent_class)->build)
        (* ((SPObjectClass *) meshpatch_parent_class)->build)(object, document, repr);

    object->readAttr( "tensor" );
}

/**
 * Virtual set: set attribute to value.
 */
static void
sp_meshpatch_set(SPObject *object, unsigned key, gchar const *value)
{
    SPMeshPatch *patch = SP_MESHPATCH(object);

    switch (key) {
        case SP_ATTR_TENSOR: {
            if (value) {
                patch->tensor_string = new Glib::ustring( value );
                // std::cout << "sp_meshpatch_set: Tensor string: " << patch->tensor_string->c_str() << std::endl;
            }
            break;
        }
        default: {
            // Do nothing
        }
    }
}

/**
 * Virtual write: write object attributes to repr.
 */
static Inkscape::XML::Node *
sp_meshpatch_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    //SPMeshPatch *meshpatch = SP_MESHPATCH(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:meshPatch");
    }

    if (((SPObjectClass *) meshpatch_parent_class)->write) {
        (* ((SPObjectClass *) meshpatch_parent_class)->write)(object, xml_doc, repr, flags);
    }

    return repr;
}


/*
 * Gradient
 */

static SPPaintServerClass *gradient_parent_class;

/**
 * Registers SPGradient class and returns its type.
 */
GType SPGradient::getType()
{
    static GType gradient_type = 0;
    if (!gradient_type) {

        GTypeInfo gradient_info = {
            sizeof(SPGradientClass),
            NULL, NULL,
            (GClassInitFunc) SPGradientImpl::classInit,
            NULL, NULL,
            sizeof(SPGradient),
            16,
            (GInstanceInitFunc) SPGradientImpl::init,
            NULL,   /* value_table */
        };
        gradient_type = g_type_register_static(SP_TYPE_PAINT_SERVER, "SPGradient",
                                               &gradient_info, (GTypeFlags)0);
    }
    return gradient_type;
}

/**
 * SPGradient vtable initialization.
 */
void SPGradientImpl::classInit(SPGradientClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    gradient_parent_class = (SPPaintServerClass *)g_type_class_ref(SP_TYPE_PAINT_SERVER);

    sp_object_class->build = SPGradientImpl::build;
    sp_object_class->release = SPGradientImpl::release;
    sp_object_class->set = SPGradientImpl::setGradientAttr;
    sp_object_class->child_added = SPGradientImpl::childAdded;
    sp_object_class->remove_child = SPGradientImpl::removeChild;
    sp_object_class->modified = SPGradientImpl::modified;
    sp_object_class->write = SPGradientImpl::write;
}

/**
 * Callback for SPGradient object initialization.
 */
void SPGradientImpl::init(SPGradient *gr)
{
    gr->ref = new SPGradientReference(gr);
    gr->ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(SPGradientImpl::gradientRefChanged), gr));

    /** \todo
     * Fixme: reprs being rearranged (e.g. via the XML editor)
     * may require us to clear the state.
     */
    gr->state = SP_GRADIENT_STATE_UNKNOWN;

    gr->units = SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX;
    gr->units_set = FALSE;

    gr->gradientTransform = Geom::identity();
    gr->gradientTransform_set = FALSE;

    gr->spread = SP_GRADIENT_SPREAD_PAD;
    gr->spread_set = FALSE;

    gr->has_stops = FALSE;

    gr->vector.built = false;
    gr->vector.stops.clear();

    new (&gr->modified_connection) sigc::connection();
}

/**
 * Virtual build: set gradient attributes from its associated repr.
 */
void SPGradientImpl::build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    SPGradient *gradient = SP_GRADIENT(object);

    // Work-around in case a swatch had been marked for immediate collection:
    if ( repr->attribute("osb:paint") && repr->attribute("inkscape:collect") ) {
        repr->setAttribute("inkscape:collect", 0);
    }

    if (((SPObjectClass *) gradient_parent_class)->build) {
        (* ((SPObjectClass *) gradient_parent_class)->build)(object, document, repr);
    }

    for ( SPObject *ochild = object->firstChild() ; ochild ; ochild = ochild->getNext() ) {
        if (SP_IS_STOP(ochild)) {
            gradient->has_stops = TRUE;
            break;
        }
    }

    object->readAttr( "gradientUnits" );
    object->readAttr( "gradientTransform" );
    object->readAttr( "spreadMethod" );
    object->readAttr( "xlink:href" );
    object->readAttr( "osb:paint" );

    // Register ourselves
    document->addResource("gradient", object);
}

/**
 * Virtual release of SPGradient members before destruction.
 */
void SPGradientImpl::release(SPObject *object)
{
    SPGradient *gradient = (SPGradient *) object;

#ifdef SP_GRADIENT_VERBOSE
    g_print("Releasing gradient %s\n", object->getId());
#endif

    if (object->document) {
        // Unregister ourselves
        object->document->removeResource("gradient", object);
    }

    if (gradient->ref) {
        gradient->modified_connection.disconnect();
        gradient->ref->detach();
        delete gradient->ref;
        gradient->ref = NULL;
    }

    gradient->modified_connection.~connection();

    if (((SPObjectClass *) gradient_parent_class)->release)
        ((SPObjectClass *) gradient_parent_class)->release(object);
}

/**
 * Set gradient attribute to value.
 */
void SPGradientImpl::setGradientAttr(SPObject *object, unsigned key, gchar const *value)
{
    SPGradient *gr = SP_GRADIENT(object);

    switch (key) {
        case SP_ATTR_GRADIENTUNITS:
            if (value) {
                if (!strcmp(value, "userSpaceOnUse")) {
                    gr->units = SP_GRADIENT_UNITS_USERSPACEONUSE;
                } else {
                    gr->units = SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX;
                }
                gr->units_set = TRUE;
            } else {
                gr->units = SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX;
                gr->units_set = FALSE;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_GRADIENTTRANSFORM: {
            Geom::Affine t;
            if (value && sp_svg_transform_read(value, &t)) {
                gr->gradientTransform = t;
                gr->gradientTransform_set = TRUE;
            } else {
                gr->gradientTransform = Geom::identity();
                gr->gradientTransform_set = FALSE;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_SPREADMETHOD:
            if (value) {
                if (!strcmp(value, "reflect")) {
                    gr->spread = SP_GRADIENT_SPREAD_REFLECT;
                } else if (!strcmp(value, "repeat")) {
                    gr->spread = SP_GRADIENT_SPREAD_REPEAT;
                } else {
                    gr->spread = SP_GRADIENT_SPREAD_PAD;
                }
                gr->spread_set = TRUE;
            } else {
                gr->spread_set = FALSE;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_XLINK_HREF:
            if (value) {
                try {
                    gr->ref->attach(Inkscape::URI(value));
                } catch (Inkscape::BadURIException &e) {
                    g_warning("%s", e.what());
                    gr->ref->detach();
                }
            } else {
                gr->ref->detach();
            }
            break;
        case SP_ATTR_OSB_SWATCH:
        {
            bool newVal = (value != 0);
            bool modified = false;
            if (newVal != gr->swatch) {
                gr->swatch = newVal;
                modified = true;
            }
            if (newVal) {
                // Might need to flip solid/gradient
                Glib::ustring paintVal = ( gr->hasStops() && (gr->getStopCount() == 0) ) ? "solid" : "gradient";
                if ( paintVal != value ) {
                    gr->setAttribute( "osb:paint", paintVal.c_str(), 0 );
                    modified = true;
                }
            }
            if (modified) {
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
        }
            break;
        default:
            if (((SPObjectClass *) gradient_parent_class)->set) {
                ((SPObjectClass *) gradient_parent_class)->set(object, key, value);
            }
            break;
    }
}

/**
 * Gets called when the gradient is (re)attached to another gradient.
 */
void SPGradientImpl::gradientRefChanged(SPObject *old_ref, SPObject *ref, SPGradient *gr)
{
    if (old_ref) {
        gr->modified_connection.disconnect();
    }
    if ( SP_IS_GRADIENT(ref)
         && ref != gr )
    {
        gr->modified_connection = ref->connectModified(sigc::bind<2>(sigc::ptr_fun(&SPGradientImpl::gradientRefModified), gr));
    }

    // Per SVG, all unset attributes must be inherited from linked gradient.
    // So, as we're now (re)linked, we assign linkee's values to this gradient if they are not yet set -
    // but without setting the _set flags.
    // FIXME: do the same for gradientTransform too
    if (!gr->units_set) {
        gr->units = gr->fetchUnits();
    }
    if (!gr->spread_set) {
        gr->spread = gr->fetchSpread();
    }

    /// \todo Fixme: what should the flags (second) argument be? */
    gradientRefModified(ref, 0, gr);
}

/**
 * Callback for child_added event.
 */
void SPGradientImpl::childAdded(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    SPGradient *gr = SP_GRADIENT(object);

    gr->invalidateVector();

    if (((SPObjectClass *) gradient_parent_class)->child_added) {
        (* ((SPObjectClass *) gradient_parent_class)->child_added)(object, child, ref);
    }

    SPObject *ochild = object->get_child_by_repr(child);
    if ( ochild && SP_IS_STOP(ochild) ) {
        gr->has_stops = TRUE;
        if ( gr->getStopCount() > 0 ) {
            gchar const * attr = gr->getAttribute("osb:paint");
            if ( attr && strcmp(attr, "gradient") ) {
                gr->setAttribute( "osb:paint", "gradient", 0 );
            }
        }
    }

    /// \todo Fixme: should we schedule "modified" here?
    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Callback for remove_child event.
 */
void SPGradientImpl::removeChild(SPObject *object, Inkscape::XML::Node *child)
{
    SPGradient *gr = SP_GRADIENT(object);

    gr->invalidateVector();

    if (((SPObjectClass *) gradient_parent_class)->remove_child) {
        (* ((SPObjectClass *) gradient_parent_class)->remove_child)(object, child);
    }

    gr->has_stops = FALSE;
    for ( SPObject *ochild = object->firstChild() ; ochild ; ochild = ochild->getNext() ) {
        if (SP_IS_STOP(ochild)) {
            gr->has_stops = TRUE;
            break;
        }
    }

    if ( gr->getStopCount() == 0 ) {
        gchar const * attr = gr->getAttribute("osb:paint");
        if ( attr && strcmp(attr, "solid") ) {
            gr->setAttribute( "osb:paint", "solid", 0 );
        }
    }

    /* Fixme: should we schedule "modified" here? */
    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Callback for modified event.
 */
void SPGradientImpl::modified(SPObject *object, guint flags)
{
    SPGradient *gr = SP_GRADIENT(object);

    if (flags & SP_OBJECT_CHILD_MODIFIED_FLAG) {
        if( gr->get_type() != SP_GRADIENT_TYPE_MESH ) {
            gr->invalidateVector();
        } else {
            gr->invalidateArray();
        }
    }

    if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
        if( gr->get_type() != SP_GRADIENT_TYPE_MESH ) {
            gr->ensureVector();
        } else {
            gr->ensureArray();
        }
    }

    if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    // FIXME: climb up the ladder of hrefs
    GSList *l = NULL;
    for (SPObject *child = object->firstChild() ; child; child = child->getNext() ) {
        g_object_ref(G_OBJECT(child));
        l = g_slist_prepend(l, child);
    }
    l = g_slist_reverse(l);
    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);
        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }
        g_object_unref(G_OBJECT(child));
    }
}

SPStop* SPGradient::getFirstStop()
{
    SPStop* first = 0;
    for (SPObject *ochild = firstChild(); ochild && !first; ochild = ochild->getNext()) {
        if (SP_IS_STOP(ochild)) {
            first = SP_STOP(ochild);
        }
    }
    return first;
}

int SPGradient::getStopCount() const
{
    int count = 0;

    for (SPStop *stop = const_cast<SPGradient*>(this)->getFirstStop(); stop && stop->getNextStop(); stop = stop->getNextStop()) {
        count++;
    }

    return count;
}

/**
 * Write gradient attributes to repr.
 */
Inkscape::XML::Node *SPGradientImpl::write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPGradient *gr = SP_GRADIENT(object);

    if (((SPObjectClass *) gradient_parent_class)->write) {
        (* ((SPObjectClass *) gradient_parent_class)->write)(object, xml_doc, repr, flags);
    }

    if (flags & SP_OBJECT_WRITE_BUILD) {
        GSList *l = NULL;
        for (SPObject *child = object->firstChild(); child; child = child->getNext()) {
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
    }

    if (gr->ref->getURI()) {
        gchar *uri_string = gr->ref->getURI()->toString();
        repr->setAttribute("xlink:href", uri_string);
        g_free(uri_string);
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || gr->units_set) {
        switch (gr->units) {
            case SP_GRADIENT_UNITS_USERSPACEONUSE:
                repr->setAttribute("gradientUnits", "userSpaceOnUse");
                break;
            default:
                repr->setAttribute("gradientUnits", "objectBoundingBox");
                break;
        }
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || gr->gradientTransform_set) {
        gchar *c=sp_svg_transform_write(gr->gradientTransform);
        repr->setAttribute("gradientTransform", c);
        g_free(c);
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || gr->spread_set) {
        /* FIXME: Ensure that gr->spread is the inherited value
         * if !gr->spread_set.  Not currently happening: see SPGradient::modified.
         */
        switch (gr->spread) {
            case SP_GRADIENT_SPREAD_REFLECT:
                repr->setAttribute("spreadMethod", "reflect");
                break;
            case SP_GRADIENT_SPREAD_REPEAT:
                repr->setAttribute("spreadMethod", "repeat");
                break;
            default:
                repr->setAttribute("spreadMethod", "pad");
                break;
        }
    }

    if ( (flags & SP_OBJECT_WRITE_EXT) && gr->isSwatch() ) {
        if ( gr->isSolid() ) {
            repr->setAttribute( "osb:paint", "solid" );
        } else {
            repr->setAttribute( "osb:paint", "gradient" );
        }
    } else {
        repr->setAttribute( "osb:paint", 0 );
    }

    return repr;
}

/**
 * Forces the vector to be built, if not present (i.e., changed).
 *
 * \pre SP_IS_GRADIENT(gradient).
 */
void SPGradient::ensureVector()
{
    if ( !vector.built ) {
        rebuildVector();
    }
}

/**
 * Forces the array to be built, if not present (i.e., changed).
 *
 * \pre SP_IS_GRADIENT(gradient).
 */
void SPGradient::ensureArray()
{
    //std::cout << "SPGradient::ensureArray()" << std::endl;
    if ( !array.built ) {
        rebuildArray();
    }
}

/**
 * Set units property of gradient and emit modified.
 */
void SPGradient::setUnits(SPGradientUnits units)
{
    if (units != this->units) {
        this->units = units;
        units_set = TRUE;
        requestModified(SP_OBJECT_MODIFIED_FLAG);
    }
}

/**
 * Set spread property of gradient and emit modified.
 */
void SPGradient::setSpread(SPGradientSpread spread)
{
    if (spread != this->spread) {
        this->spread = spread;
        spread_set = TRUE;
        requestModified(SP_OBJECT_MODIFIED_FLAG);
    }
}

/**
 * Returns the first of {src, src-\>ref-\>getObject(),
 * src-\>ref-\>getObject()-\>ref-\>getObject(),...}
 * for which \a match is true, or NULL if none found.
 *
 * The raison d'être of this routine is that it correctly handles cycles in the href chain (e.g., if
 * a gradient gives itself as its href, or if each of two gradients gives the other as its href).
 *
 * \pre SP_IS_GRADIENT(src).
 */
static SPGradient *
chase_hrefs(SPGradient *const src, bool (*match)(SPGradient const *))
{
    g_return_val_if_fail(SP_IS_GRADIENT(src), NULL);

    /* Use a pair of pointers for detecting loops: p1 advances half as fast as p2.  If there is a
       loop, then once p1 has entered the loop, we'll detect it the next time the distance between
       p1 and p2 is a multiple of the loop size. */
    SPGradient *p1 = src, *p2 = src;
    bool do1 = false;
    for (;;) {
        if (match(p2)) {
            return p2;
        }

        p2 = p2->ref->getObject();
        if (!p2) {
            return p2;
        }
        if (do1) {
            p1 = p1->ref->getObject();
        }
        do1 = !do1;

        if ( p2 == p1 ) {
            /* We've been here before, so return NULL to indicate that no matching gradient found
             * in the chain. */
            return NULL;
        }
    }
}

/**
 * True if gradient has stops.
 */
static bool has_stopsFN(SPGradient const *gr)
{
    return gr->hasStops();
}

/**
 * True if gradient has spread set.
 */
static bool has_spread_set(SPGradient const *gr)
{
    return gr->isSpreadSet();
}

/**
 * True if gradient has units set.
 */
static bool
has_units_set(SPGradient const *gr)
{
    return gr->isUnitsSet();
}


SPGradient *SPGradient::getVector(bool force_vector)
{
    SPGradient * src = chase_hrefs(this, has_stopsFN);

    if (force_vector) {
        src = sp_gradient_ensure_vector_normalized(src);
    }
    return src;
}

/**
 * Returns the effective spread of given gradient (climbing up the refs chain if needed).
 *
 * \pre SP_IS_GRADIENT(gradient).
 */
SPGradientSpread SPGradient::fetchSpread()
{
    SPGradient const *src = chase_hrefs(this, has_spread_set);
    return ( src
             ? src->spread
             : SP_GRADIENT_SPREAD_PAD ); // pad is the default
}

/**
 * Returns the effective units of given gradient (climbing up the refs chain if needed).
 *
 * \pre SP_IS_GRADIENT(gradient).
 */
SPGradientUnits SPGradient::fetchUnits()
{
    SPGradient const *src = chase_hrefs(this, has_units_set);
    return ( src
             ? src->units
             : SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX ); // bbox is the default
}


/**
 * Clears the gradient's svg:stop children from its repr.
 */
void
sp_gradient_repr_clear_vector(SPGradient *gr)
{
    Inkscape::XML::Node *repr = gr->getRepr();

    /* Collect stops from original repr */
    GSList *sl = NULL;
    for (Inkscape::XML::Node *child = repr->firstChild() ; child != NULL; child = child->next() ) {
        if (!strcmp(child->name(), "svg:stop")) {
            sl = g_slist_prepend(sl, child);
        }
    }
    /* Remove all stops */
    while (sl) {
        /** \todo
         * fixme: This should work, unless we make gradient
         * into generic group.
         */
        sp_repr_unparent((Inkscape::XML::Node *)sl->data);
        sl = g_slist_remove(sl, sl->data);
    }
}

/**
 * Writes the gradient's internal vector (whether from its own stops, or
 * inherited from refs) into the gradient repr as svg:stop elements.
 */
void
sp_gradient_repr_write_vector(SPGradient *gr)
{
    g_return_if_fail(gr != NULL);
    g_return_if_fail(SP_IS_GRADIENT(gr));

    Inkscape::XML::Document *xml_doc = gr->document->getReprDoc();
    Inkscape::XML::Node *repr = gr->getRepr();

    /* We have to be careful, as vector may be our own, so construct repr list at first */
    GSList *cl = NULL;

    for (guint i = 0; i < gr->vector.stops.size(); i++) {
        Inkscape::CSSOStringStream os;
        Inkscape::XML::Node *child = xml_doc->createElement("svg:stop");
        sp_repr_set_css_double(child, "offset", gr->vector.stops[i].offset);
        /* strictly speaking, offset an SVG <number> rather than a CSS one, but exponents make no
         * sense for offset proportions. */
        os << "stop-color:" << gr->vector.stops[i].color.toString() << ";stop-opacity:" << gr->vector.stops[i].opacity;
        child->setAttribute("style", os.str().c_str());
        /* Order will be reversed here */
        cl = g_slist_prepend(cl, child);
    }

    sp_gradient_repr_clear_vector(gr);

    /* And insert new children from list */
    while (cl) {
        Inkscape::XML::Node *child = static_cast<Inkscape::XML::Node *>(cl->data);
        repr->addChild(child, NULL);
        Inkscape::GC::release(child);
        cl = g_slist_remove(cl, child);
    }
}


void SPGradientImpl::gradientRefModified(SPObject */*href*/, guint /*flags*/, SPGradient *gradient)
{
    if ( gradient->invalidateVector() ) {
        gradient->requestModified(SP_OBJECT_MODIFIED_FLAG);
        // Conditional to avoid causing infinite loop if there's a cycle in the href chain.
    }
}

/** Return true if change made. */
bool SPGradient::invalidateVector()
{
    bool ret = false;

    if (vector.built) {
        vector.built = false;
        vector.stops.clear();
        ret = true;
    }

    return ret;
}

/** Return true if change made. */
bool SPGradient::invalidateArray()
{
    bool ret = false;

    if (array.built) {
        array.built = false;
        array.clear();
        ret = true;
    }

    return ret;
}

/** Creates normalized color vector */
void SPGradient::rebuildVector()
{
    gint len = 0;
    for ( SPObject *child = firstChild() ; child ; child = child->getNext() ) {
        if (SP_IS_STOP(child)) {
            len ++;
        }
    }

    has_stops = (len != 0);

    vector.stops.clear();

    SPGradient *reffed = ref ? ref->getObject() : NULL;
    if ( !hasStops() && reffed ) {
        /* Copy vector from referenced gradient */
        vector.built = true;   // Prevent infinite recursion.
        reffed->ensureVector();
        if (!reffed->vector.stops.empty()) {
            vector.built = reffed->vector.built;
            vector.stops.assign(reffed->vector.stops.begin(), reffed->vector.stops.end());
            return;
        }
    }

    for ( SPObject *child = firstChild(); child; child = child->getNext() ) {
        if (SP_IS_STOP(child)) {
            SPStop *stop = SP_STOP(child);

            SPGradientStop gstop;
            if (!vector.stops.empty()) {
                // "Each gradient offset value is required to be equal to or greater than the
                // previous gradient stop's offset value. If a given gradient stop's offset
                // value is not equal to or greater than all previous offset values, then the
                // offset value is adjusted to be equal to the largest of all previous offset
                // values."
                gstop.offset = MAX(stop->offset, vector.stops.back().offset);
            } else {
                gstop.offset = stop->offset;
            }

            // "Gradient offset values less than 0 (or less than 0%) are rounded up to
            // 0%. Gradient offset values greater than 1 (or greater than 100%) are rounded
            // down to 100%."
            gstop.offset = CLAMP(gstop.offset, 0, 1);

            gstop.color = stop->getEffectiveColor();
            gstop.opacity = stop->opacity;

            vector.stops.push_back(gstop);
        }
    }

    // Normalize per section 13.2.4 of SVG 1.1.
    if (vector.stops.empty()) {
        /* "If no stops are defined, then painting shall occur as if 'none' were specified as the
         * paint style."
         */
        {
            SPGradientStop gstop;
            gstop.offset = 0.0;
            gstop.color.set( 0x00000000 );
            gstop.opacity = 0.0;
            vector.stops.push_back(gstop);
        }
        {
            SPGradientStop gstop;
            gstop.offset = 1.0;
            gstop.color.set( 0x00000000 );
            gstop.opacity = 0.0;
            vector.stops.push_back(gstop);
        }
    } else {
        /* "If one stop is defined, then paint with the solid color fill using the color defined
         * for that gradient stop."
         */
        if (vector.stops.front().offset > 0.0) {
            // If the first one is not at 0, then insert a copy of the first at 0.
            SPGradientStop gstop;
            gstop.offset = 0.0;
            gstop.color = vector.stops.front().color;
            gstop.opacity = vector.stops.front().opacity;
            vector.stops.insert(vector.stops.begin(), gstop);
        }
        if (vector.stops.back().offset < 1.0) {
            // If the last one is not at 1, then insert a copy of the last at 1.
            SPGradientStop gstop;
            gstop.offset = 1.0;
            gstop.color = vector.stops.back().color;
            gstop.opacity = vector.stops.back().opacity;
            vector.stops.push_back(gstop);
        }
    }

    vector.built = true;
}

/** Creates normalized color mesh patch array */
void SPGradient::rebuildArray()
{
    // std::cout << "SPGradient::rebuildArray()" << std::endl;

    if( !SP_IS_MESHGRADIENT(this) ) {
        g_warning( "SPGradient::rebuildArray() called for non-mesh gradient" );
        return;
    }

    array.read( SP_MESHGRADIENT( this ) );

    has_patches = false;
    for ( SPObject *ro = firstChild() ; ro ; ro = ro->getNext() ) {
        if (SP_IS_MESHROW(ro)) {
            has_patches = true;
            // std::cout << "  Has Patches" << std::endl;
            break;
        }
    }

    // MESH_FIXME: TO PROPERLY COPY
    SPGradient *reffed = ref->getObject();
    if ( !hasPatches() && reffed ) {
        std::cout << "SPGradient::rebuildArray(): reffed array    NOT IMPLEMENTED!!!" << std::endl;
        /* Copy array from referenced gradient */
        array.built = true;   // Prevent infinite recursion.
        reffed->ensureArray();
        // if (!reffed->array.nodes.empty()) {
        //     array.built = reffed->array.built;
        //     for( uint i = 0; i < reffed->array.nodes.size(); ++i ) {
        //         array.nodes[i].assign(reffed->array.nodes[i].begin(), reffed->array.nodes[i].end());

        //         // FILL ME
        //     }
        //     return;
        // }
    }
}

Geom::Affine
sp_gradient_get_g2d_matrix(SPGradient const *gr, Geom::Affine const &ctm, Geom::Rect const &bbox)
{
    if (gr->getUnits() == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
        return ( Geom::Scale(bbox.dimensions())
                 * Geom::Translate(bbox.min())
                 * Geom::Affine(ctm) );
    } else {
        return ctm;
    }
}

Geom::Affine
sp_gradient_get_gs2d_matrix(SPGradient const *gr, Geom::Affine const &ctm, Geom::Rect const &bbox)
{
    if (gr->getUnits() == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
        return ( gr->gradientTransform
                 * Geom::Scale(bbox.dimensions())
                 * Geom::Translate(bbox.min())
                 * Geom::Affine(ctm) );
    } else {
        return gr->gradientTransform * ctm;
    }
}

void
sp_gradient_set_gs2d_matrix(SPGradient *gr, Geom::Affine const &ctm,
                            Geom::Rect const &bbox, Geom::Affine const &gs2d)
{
    gr->gradientTransform = gs2d * ctm.inverse();
    if (gr->getUnits() == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX ) {
        gr->gradientTransform = ( gr->gradientTransform
                                  * Geom::Translate(-bbox.min())
                                  * Geom::Scale(bbox.dimensions()).inverse() );
    }
    gr->gradientTransform_set = TRUE;

    gr->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/*
 * Linear Gradient
 */

static void sp_lineargradient_class_init(SPLinearGradientClass *klass);
static void sp_lineargradient_init(SPLinearGradient *lg);

static void sp_lineargradient_build(SPObject *object,
                                    SPDocument *document,
                                    Inkscape::XML::Node *repr);
static void sp_lineargradient_set(SPObject *object, unsigned key, gchar const *value);
static Inkscape::XML::Node *sp_lineargradient_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr,
                                                    guint flags);
static cairo_pattern_t *sp_lineargradient_create_pattern(SPPaintServer *ps, cairo_t *ct, Geom::OptRect const &bbox, double opacity);

static SPGradientClass *lg_parent_class;

/**
 * Register SPLinearGradient class and return its type.
 */
GType
sp_lineargradient_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPLinearGradientClass),
            NULL, NULL,
            (GClassInitFunc) sp_lineargradient_class_init,
            NULL, NULL,
            sizeof(SPLinearGradient),
            16,
            (GInstanceInitFunc) sp_lineargradient_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_GRADIENT, "SPLinearGradient", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * SPLinearGradient vtable initialization.
 */
static void sp_lineargradient_class_init(SPLinearGradientClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPPaintServerClass *ps_class = (SPPaintServerClass *) klass;

    lg_parent_class = (SPGradientClass*)g_type_class_ref(SP_TYPE_GRADIENT);

    sp_object_class->build = sp_lineargradient_build;
    sp_object_class->set = sp_lineargradient_set;
    sp_object_class->write = sp_lineargradient_write;

    ps_class->pattern_new = sp_lineargradient_create_pattern;
}

/**
 * Callback for SPLinearGradient object initialization.
 */
static void sp_lineargradient_init(SPLinearGradient *lg)
{
    lg->x1.unset(SVGLength::PERCENT, 0.0, 0.0);
    lg->y1.unset(SVGLength::PERCENT, 0.0, 0.0);
    lg->x2.unset(SVGLength::PERCENT, 1.0, 1.0);
    lg->y2.unset(SVGLength::PERCENT, 0.0, 0.0);
}

/**
 * Callback: set attributes from associated repr.
 */
static void sp_lineargradient_build(SPObject *object,
                                    SPDocument *document,
                                    Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) lg_parent_class)->build)
        (* ((SPObjectClass *) lg_parent_class)->build)(object, document, repr);

    object->readAttr( "x1" );
    object->readAttr( "y1" );
    object->readAttr( "x2" );
    object->readAttr( "y2" );
}

/**
 * Callback: set attribute.
 */
static void
sp_lineargradient_set(SPObject *object, unsigned key, gchar const *value)
{
    SPLinearGradient *lg = SP_LINEARGRADIENT(object);

    switch (key) {
        case SP_ATTR_X1:
            lg->x1.readOrUnset(value, SVGLength::PERCENT, 0.0, 0.0);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y1:
            lg->y1.readOrUnset(value, SVGLength::PERCENT, 0.0, 0.0);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_X2:
            lg->x2.readOrUnset(value, SVGLength::PERCENT, 1.0, 1.0);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y2:
            lg->y2.readOrUnset(value, SVGLength::PERCENT, 0.0, 0.0);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) lg_parent_class)->set)
                (* ((SPObjectClass *) lg_parent_class)->set)(object, key, value);
            break;
    }
}

/**
 * Callback: write attributes to associated repr.
 */
static Inkscape::XML::Node *
sp_lineargradient_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPLinearGradient *lg = SP_LINEARGRADIENT(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:linearGradient");
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || lg->x1._set)
        sp_repr_set_svg_double(repr, "x1", lg->x1.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || lg->y1._set)
        sp_repr_set_svg_double(repr, "y1", lg->y1.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || lg->x2._set)
        sp_repr_set_svg_double(repr, "x2", lg->x2.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || lg->y2._set)
        sp_repr_set_svg_double(repr, "y2", lg->y2.computed);

    if (((SPObjectClass *) lg_parent_class)->write)
        (* ((SPObjectClass *) lg_parent_class)->write)(object, xml_doc, repr, flags);

    return repr;
}

/**
 * Directly set properties of linear gradient and request modified.
 */
void
sp_lineargradient_set_position(SPLinearGradient *lg,
                               gdouble x1, gdouble y1,
                               gdouble x2, gdouble y2)
{
    g_return_if_fail(lg != NULL);
    g_return_if_fail(SP_IS_LINEARGRADIENT(lg));

    /* fixme: units? (Lauris)  */
    lg->x1.set(SVGLength::NONE, x1, x1);
    lg->y1.set(SVGLength::NONE, y1, y1);
    lg->x2.set(SVGLength::NONE, x2, x2);
    lg->y2.set(SVGLength::NONE, y2, y2);

    lg->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/*
 * Radial Gradient
 */

static void sp_radialgradient_class_init(SPRadialGradientClass *klass);
static void sp_radialgradient_init(SPRadialGradient *rg);

static void sp_radialgradient_build(SPObject *object,
                                    SPDocument *document,
                                    Inkscape::XML::Node *repr);
static void sp_radialgradient_set(SPObject *object, unsigned key, gchar const *value);
static Inkscape::XML::Node *sp_radialgradient_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr,
                                                    guint flags);
static cairo_pattern_t *sp_radialgradient_create_pattern(SPPaintServer *ps, cairo_t *ct, Geom::OptRect const &bbox, double opacity);

static SPGradientClass *rg_parent_class;

/**
 * Register SPRadialGradient class and return its type.
 */
GType
sp_radialgradient_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPRadialGradientClass),
            NULL, NULL,
            (GClassInitFunc) sp_radialgradient_class_init,
            NULL, NULL,
            sizeof(SPRadialGradient),
            16,
            (GInstanceInitFunc) sp_radialgradient_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_GRADIENT, "SPRadialGradient", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * SPRadialGradient vtable initialization.
 */
static void sp_radialgradient_class_init(SPRadialGradientClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPPaintServerClass *ps_class = (SPPaintServerClass *) klass;

    rg_parent_class = (SPGradientClass*)g_type_class_ref(SP_TYPE_GRADIENT);

    sp_object_class->build = sp_radialgradient_build;
    sp_object_class->set = sp_radialgradient_set;
    sp_object_class->write = sp_radialgradient_write;

    ps_class->pattern_new = sp_radialgradient_create_pattern;
}

/**
 * Callback for SPRadialGradient object initialization.
 */
static void
sp_radialgradient_init(SPRadialGradient *rg)
{
    rg->cx.unset(SVGLength::PERCENT, 0.5, 0.5);
    rg->cy.unset(SVGLength::PERCENT, 0.5, 0.5);
    rg->r.unset(SVGLength::PERCENT, 0.5, 0.5);
    rg->fx.unset(SVGLength::PERCENT, 0.5, 0.5);
    rg->fy.unset(SVGLength::PERCENT, 0.5, 0.5);
}

/**
 * Set radial gradient attributes from associated repr.
 */
static void
sp_radialgradient_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) rg_parent_class)->build)
        (* ((SPObjectClass *) rg_parent_class)->build)(object, document, repr);

    object->readAttr( "cx" );
    object->readAttr( "cy" );
    object->readAttr( "r" );
    object->readAttr( "fx" );
    object->readAttr( "fy" );
}

/**
 * Set radial gradient attribute.
 */
static void
sp_radialgradient_set(SPObject *object, unsigned key, gchar const *value)
{
    SPRadialGradient *rg = SP_RADIALGRADIENT(object);

    switch (key) {
        case SP_ATTR_CX:
            if (!rg->cx.read(value)) {
                rg->cx.unset(SVGLength::PERCENT, 0.5, 0.5);
            }
            if (!rg->fx._set) {
                rg->fx.value = rg->cx.value;
                rg->fx.computed = rg->cx.computed;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_CY:
            if (!rg->cy.read(value)) {
                rg->cy.unset(SVGLength::PERCENT, 0.5, 0.5);
            }
            if (!rg->fy._set) {
                rg->fy.value = rg->cy.value;
                rg->fy.computed = rg->cy.computed;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_R:
            if (!rg->r.read(value)) {
                rg->r.unset(SVGLength::PERCENT, 0.5, 0.5);
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_FX:
            if (!rg->fx.read(value)) {
                rg->fx.unset(rg->cx.unit, rg->cx.value, rg->cx.computed);
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_FY:
            if (!rg->fy.read(value)) {
                rg->fy.unset(rg->cy.unit, rg->cy.value, rg->cy.computed);
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) rg_parent_class)->set)
                ((SPObjectClass *) rg_parent_class)->set(object, key, value);
            break;
    }
}

/**
 * Write radial gradient attributes to associated repr.
 */
static Inkscape::XML::Node *
sp_radialgradient_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPRadialGradient *rg = SP_RADIALGRADIENT(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:radialGradient");
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || rg->cx._set) sp_repr_set_svg_double(repr, "cx", rg->cx.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || rg->cy._set) sp_repr_set_svg_double(repr, "cy", rg->cy.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || rg->r._set) sp_repr_set_svg_double(repr, "r", rg->r.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || rg->fx._set) sp_repr_set_svg_double(repr, "fx", rg->fx.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || rg->fy._set) sp_repr_set_svg_double(repr, "fy", rg->fy.computed);

    if (((SPObjectClass *) rg_parent_class)->write)
        (* ((SPObjectClass *) rg_parent_class)->write)(object, xml_doc, repr, flags);

    return repr;
}

/**
 * Directly set properties of radial gradient and request modified.
 */
void
sp_radialgradient_set_position(SPRadialGradient *rg,
                               gdouble cx, gdouble cy, gdouble fx, gdouble fy, gdouble r)
{
    g_return_if_fail(rg != NULL);
    g_return_if_fail(SP_IS_RADIALGRADIENT(rg));

    /* fixme: units? (Lauris)  */
    rg->cx.set(SVGLength::NONE, cx, cx);
    rg->cy.set(SVGLength::NONE, cy, cy);
    rg->fx.set(SVGLength::NONE, fx, fx);
    rg->fy.set(SVGLength::NONE, fy, fy);
    rg->r.set(SVGLength::NONE, r, r);

    rg->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/*
 * Mesh Gradient
 */

//#define MESH_DEBUG

static void sp_meshgradient_class_init(SPMeshGradientClass *klass);
static void sp_meshgradient_init(SPMeshGradient *mg);

static void sp_meshgradient_build(SPObject *object,
                                    SPDocument *document,
                                    Inkscape::XML::Node *repr);
static void sp_meshgradient_set(SPObject *object, unsigned key, gchar const *value);
static Inkscape::XML::Node *sp_meshgradient_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr,
                                                    guint flags);
static cairo_pattern_t *sp_meshgradient_create_pattern(SPPaintServer *ps, cairo_t *ct, Geom::OptRect const &bbox, double opacity);

static SPGradientClass *mg_parent_class;

/**
 * Register SPMeshGradient class and return its type.
 */
GType
sp_meshgradient_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPMeshGradientClass),
            NULL, NULL,
            (GClassInitFunc) sp_meshgradient_class_init,
            NULL, NULL,
            sizeof(SPMeshGradient),
            16,
            (GInstanceInitFunc) sp_meshgradient_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_GRADIENT, "SPMeshGradient", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * SPMeshGradient vtable initialization.
 */
static void sp_meshgradient_class_init(SPMeshGradientClass *klass)
{
#ifdef MESH_DEBUG
    std::cout << "sp_meshgradient_class_init()" << std::endl;
#endif
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPPaintServerClass *ps_class = (SPPaintServerClass *) klass;

    mg_parent_class = (SPGradientClass*)g_type_class_ref(SP_TYPE_GRADIENT);

    sp_object_class->build = sp_meshgradient_build;
    sp_object_class->set = sp_meshgradient_set;
    sp_object_class->write = sp_meshgradient_write;

    ps_class->pattern_new = sp_meshgradient_create_pattern;
}

/**
 * Callback for SPMeshGradient object initialization.
 */
static void
sp_meshgradient_init(SPMeshGradient *mg)
{
    // Start coordinate of mesh
    mg->x.unset(SVGLength::NONE, 0.0, 0.0);
    mg->y.unset(SVGLength::NONE, 0.0, 0.0);
}

/**
 * Set mesh gradient attributes from associated repr.
 */
static void
sp_meshgradient_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) mg_parent_class)->build)
        (* ((SPObjectClass *) mg_parent_class)->build)(object, document, repr);

    // Start coordinate of mesh
    object->readAttr( "x" );
    object->readAttr( "y" );
}

/**
 * Set mesh gradient attribute.
 */
static void
sp_meshgradient_set(SPObject *object, unsigned key, gchar const *value)
{
    SPMeshGradient *mg = SP_MESHGRADIENT(object);

    switch (key) {
        case SP_ATTR_X:
            if (!mg->x.read(value)) {
                mg->x.unset(SVGLength::NONE, 0.0, 0.0);
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y:
            if (!mg->y.read(value)) {
                mg->y.unset(SVGLength::NONE, 0.0, 0.0);
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) mg_parent_class)->set)
                ((SPObjectClass *) mg_parent_class)->set(object, key, value);
            break;
    }
}

/**
 * Write mesh gradient attributes to associated repr.
 */
static Inkscape::XML::Node *
sp_meshgradient_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{

#ifdef MESH_DEBUG
    std::cout << "sp_meshgradient_write() ***************************" << std::endl;
#endif
    SPMeshGradient *mg = SP_MESHGRADIENT(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:meshGradient");
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || mg->x._set) sp_repr_set_svg_double(repr, "x", mg->x.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || mg->y._set) sp_repr_set_svg_double(repr, "y", mg->y.computed);

    if (((SPObjectClass *) mg_parent_class)->write)
        (* ((SPObjectClass *) mg_parent_class)->write)(object, xml_doc, repr, flags);

    return repr;
}

/**
 * Directly set properties of mesh gradient and request modified.
 */
void
sp_meshgradient_set_position(SPMeshGradient *mg, gdouble x, gdouble y)
{
    g_return_if_fail(mg != NULL);
    g_return_if_fail(SP_IS_MESHGRADIENT(mg));

    mg->x.set(SVGLength::NONE, x, x);
    mg->y.set(SVGLength::NONE, y, y);

    mg->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/* CAIRO RENDERING STUFF */

static void
sp_gradient_pattern_common_setup(cairo_pattern_t *cp,
                                 SPGradient *gr,
                                 Geom::OptRect const &bbox,
                                 double opacity)
{
    // set spread type
    switch (gr->getSpread()) {
    case SP_GRADIENT_SPREAD_REFLECT:
        cairo_pattern_set_extend(cp, CAIRO_EXTEND_REFLECT);
        break;
    case SP_GRADIENT_SPREAD_REPEAT:
        cairo_pattern_set_extend(cp, CAIRO_EXTEND_REPEAT);
        break;
    case SP_GRADIENT_SPREAD_PAD:
    default:
        cairo_pattern_set_extend(cp, CAIRO_EXTEND_PAD);
        break;
    }

    // add stops
    for (std::vector<SPGradientStop>::iterator i = gr->vector.stops.begin();
         i != gr->vector.stops.end(); ++i)
    {
        // multiply stop opacity by paint opacity
        cairo_pattern_add_color_stop_rgba(cp, i->offset,
            i->color.v.c[0], i->color.v.c[1], i->color.v.c[2], i->opacity * opacity);
    }

    // set pattern matrix
    Geom::Affine gs2user = gr->gradientTransform;
    if (gr->getUnits() == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX && bbox) {
        Geom::Affine bbox2user(bbox->width(), 0, 0, bbox->height(), bbox->left(), bbox->top());
        gs2user *= bbox2user;
    }
    ink_cairo_pattern_set_matrix(cp, gs2user.inverse());
}

static cairo_pattern_t *
sp_radialgradient_create_pattern(SPPaintServer *ps,
                                 cairo_t *ct,
                                 Geom::OptRect const &bbox,
                                 double opacity)
{
    SPRadialGradient *rg = SP_RADIALGRADIENT(ps);
    SPGradient *gr = SP_GRADIENT(ps);

    gr->ensureVector();

    Geom::Point focus(rg->fx.computed, rg->fy.computed);
    Geom::Point center(rg->cx.computed, rg->cy.computed);
    double radius = rg->r.computed;
    double scale = 1.0;
    double tolerance = cairo_get_tolerance(ct);

    // NOTE: SVG2 will allow the use of a focus circle which can
    // have its center outside the first circle.

    // code below suggested by Cairo devs to overcome tolerance problems
    // more: https://bugs.freedesktop.org/show_bug.cgi?id=40918

    // Corrected for
    // https://bugs.launchpad.net/inkscape/+bug/970355

    Geom::Affine gs2user = gr->gradientTransform;
    Geom::Scale  gs2user_scale;

    if (gr->getUnits() == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX && bbox) {
        Geom::Affine bbox2user(bbox->width(), 0, 0, bbox->height(), bbox->left(), bbox->top());
        gs2user *= bbox2user;
        gs2user_scale = Geom::Scale( gs2user[0], gs2user[3] );
    }

    Geom::Point d = focus - center;
    Geom::Point d_user = d * gs2user_scale;
    Geom::Point r_user( radius, 0 );
    r_user *= gs2user_scale;

    if (d_user.length() + tolerance > r_user.length()) {
        scale = r_user.length() / d_user.length();
        double dx = d_user.x(), dy = d_user.y();
        cairo_user_to_device_distance(ct, &dx, &dy);
        if (!Geom::are_near(dx, 0, tolerance) ||
            !Geom::are_near(dy, 0, tolerance))
        {
            scale *= 1.0 - 2.0 * tolerance / hypot(dx, dy);
        }
    }

    cairo_pattern_t *cp = cairo_pattern_create_radial(
        scale * d.x() + center.x(), scale * d.y() + center.y(), 0,
        center.x(), center.y(), radius);

    sp_gradient_pattern_common_setup(cp, gr, bbox, opacity);

    return cp;
}

static cairo_pattern_t *sp_meshgradient_create_pattern(SPPaintServer *ps,
                                                       cairo_t * /* ct */,
#if defined(MESH_DEBUG) || (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 11, 4))
                                                       Geom::OptRect const &bbox,
                                                       double opacity
#else
                                                       Geom::OptRect const & /*bbox*/,
                                                       double /*opacity*/
#endif
    )
{
    using Geom::X;
    using Geom::Y;

#ifdef MESH_DEBUG
    std::cout << "sp_meshgradient_create_pattern: (" << bbox->x0 << "," <<  bbox->y0 << ") (" <<  bbox->x1 << "," << bbox->y1 << ")  " << opacity << std::endl;
#endif
    //SPMeshGradient *mg = SP_MESHGRADIENT(ps);
    SPGradient *gr = SP_GRADIENT(ps);

    gr->ensureArray();

    cairo_pattern_t *cp = NULL;

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 11, 4)
    SPMeshNodeArray* array = &(gr->array);

    cp = cairo_pattern_create_mesh();

    for( unsigned int i = 0; i < array->patch_rows(); ++i ) {
        for( unsigned int j = 0; j < array->patch_columns(); ++j ) {

            SPMeshPatchI patch( &(array->nodes), i, j );

            cairo_mesh_pattern_begin_patch( cp );
            cairo_mesh_pattern_move_to( cp, patch.getPoint( 0, 0 )[X], patch.getPoint( 0, 0 )[Y] ); 

            for( unsigned int k = 0; k < 4; ++k ) {
#ifdef DEBUG_MESH
                std::cout << i << " " << j << " "
                          << patch.getPathType( k ) << "  (";
                for( int p = 0; p < 4; ++p ) {
                    std::cout << patch.getPoint( k, p );
                }
                std::cout << ") "
                          << patch.getColor( k ).toString() << std::endl;
#endif

                switch ( patch.getPathType( k ) ) {
                    case 'l':
                    case 'L':
                    case 'z':
                    case 'Z':
                        cairo_mesh_pattern_line_to( cp,
                                                    patch.getPoint( k, 3 )[X],
                                                    patch.getPoint( k, 3 )[Y] );
                        break;
                    case 'c':
                    case 'C':
                    {
                        std::vector< Geom::Point > pts = patch.getPointsForSide( k );
                        cairo_mesh_pattern_curve_to( cp,
                                                     pts[1][X], pts[1][Y],
                                                     pts[2][X], pts[2][Y],
                                                     pts[3][X], pts[3][Y] );
                        break;
                    }
                    default:
                        // Shouldn't happen
                        std::cout << "sp_meshgradient_create_pattern: path error" << std::endl;
                }

                if( patch.tensorIsSet(k) ) {
                    // Tensor point defined relative to corner.
                    Geom::Point t = patch.getTensorPoint(k);
                    cairo_mesh_pattern_set_control_point( cp, k, t[X], t[Y] );
                    //std::cout << "  sp_meshgradient_create_pattern: tensor " << k
                    //          << " set to " << t << "." << std::endl;
                } else {
                    // Geom::Point t = patch.coonsTensorPoint(k);
                    //std::cout << "  sp_meshgradient_create_pattern: tensor " << k
                    //          << " calculated as " << t << "." <<std::endl;
                }

                cairo_mesh_pattern_set_corner_color_rgba(
                    cp, k,
                    patch.getColor( k ).v.c[0], 
                    patch.getColor( k ).v.c[1], 
                    patch.getColor( k ).v.c[2], 
                    patch.getOpacity( k ) * opacity );
            }

            cairo_mesh_pattern_end_patch( cp );
        }
    }

    // set pattern matrix
    Geom::Affine gs2user = gr->gradientTransform;
    if (gr->getUnits() == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
        Geom::Affine bbox2user(bbox->width(), 0, 0, bbox->height(), bbox->left(), bbox->top());
        gs2user *= bbox2user;
    }
    ink_cairo_pattern_set_matrix(cp, gs2user.inverse());

#else
    static bool shown = false;
    if( !shown ) {
        std::cout << "sp_meshgradient_create_pattern: needs cairo >= 1.11.4, using "
                  << cairo_version_string() << std::endl;
        shown = true;
    }
#endif

/*
    cairo_pattern_t *cp = cairo_pattern_create_radial(
        rg->fx.computed, rg->fy.computed, 0,
        rg->cx.computed, rg->cy.computed, rg->r.computed);
    sp_gradient_pattern_common_setup(cp, gr, bbox, opacity);
*/

    return cp;
}

static cairo_pattern_t *
sp_lineargradient_create_pattern(SPPaintServer *ps,
                                 cairo_t */* ct */,
                                 Geom::OptRect const &bbox,
                                 double opacity)
{
    SPLinearGradient *lg = SP_LINEARGRADIENT(ps);
    SPGradient *gr = SP_GRADIENT(ps);

    gr->ensureVector();

    cairo_pattern_t *cp = cairo_pattern_create_linear(
        lg->x1.computed, lg->y1.computed,
        lg->x2.computed, lg->y2.computed);

    sp_gradient_pattern_common_setup(cp, gr, bbox, opacity);

    return cp;
}

cairo_pattern_t *
sp_gradient_create_preview_pattern(SPGradient *gr, double width)
{
    cairo_pattern_t *pat = NULL;

    if( gr->get_type() != SP_GRADIENT_TYPE_MESH ) { 

        gr->ensureVector();

        pat = cairo_pattern_create_linear(0, 0, width, 0);

        for (std::vector<SPGradientStop>::iterator i = gr->vector.stops.begin();
             i != gr->vector.stops.end(); ++i)
        {
            cairo_pattern_add_color_stop_rgba(pat, i->offset,
              i->color.v.c[0], i->color.v.c[1], i->color.v.c[2], i->opacity);
        }
    }

    return pat;
}

void
sp_meshgradient_repr_write(SPMeshGradient *mg)
{
    mg->array.write( mg );
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
