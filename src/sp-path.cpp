/*
 * SVG <path> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   David Turner <novalis@gnu.org>
 *   Abhishek Sharma
 *   Johan Engelen
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 1999-2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>

#include "live_effects/effect.h"
#include "live_effects/lpeobject.h"
#include "live_effects/lpeobject-reference.h"
#include "sp-lpe-item.h"

#include "display/curve.h"
#include <2geom/pathvector.h>
#include <2geom/bezier-curve.h>
#include <2geom/hvlinesegment.h>
#include "helper/geom-curves.h"

#include "svg/svg.h"
#include "xml/repr.h"
#include "attributes.h"

#include "sp-path.h"
#include "sp-guide.h"

#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "ui/tools/tool-base.h"
#include "inkscape.h"
#include "style.h"
#include "message-stack.h"
#include "selection.h"

#define noPATH_VERBOSE

#include "sp-factory.h"

namespace {
	SPObject* createPath() {
		return new SPPath();
	}

	bool pathRegistered = SPFactory::instance().registerObject("svg:path", createPath);
}

gint SPPath::nodesInPath() const
{
    return _curve ? _curve->nodes_in_path() : 0;
}

const char* SPPath::displayName() const {
    return _("Path");
}

gchar* SPPath::description() const {
    int count = this->nodesInPath();
    char *lpe_desc = g_strdup("");
    
    if (hasPathEffect()) {
        Glib::ustring s;
        PathEffectList effect_list =  this->getEffectList();
        
        for (PathEffectList::iterator it = effect_list.begin(); it != effect_list.end(); ++it)
        {
            LivePathEffectObject *lpeobj = (*it)->lpeobject;
            
            if (!lpeobj || !lpeobj->get_lpe()) {
                break;
            }
            
            if (s.empty()) {
                s = lpeobj->get_lpe()->getName();
            } else {
                s = s + ", " + lpeobj->get_lpe()->getName();
            }
        }
        lpe_desc = g_strdup_printf(_(", path effect: %s"), s.c_str());
    }
    char *ret = g_strdup_printf(ngettext(
                _("%i node%s"), _("%i nodes%s"), count), count, lpe_desc);
    g_free(lpe_desc);
    return ret;
}

void SPPath::convert_to_guides() const {
    if (!this->_curve) {
        return;
    }

    std::list<std::pair<Geom::Point, Geom::Point> > pts;

    Geom::Affine const i2dt(this->i2dt_affine());
    Geom::PathVector const & pv = this->_curve->get_pathvector();
    
    for(Geom::PathVector::const_iterator pit = pv.begin(); pit != pv.end(); ++pit) {
        for(Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_default(); ++cit) {
            // only add curves for straight line segments
            if( is_straight_curve(*cit) )
            {
                pts.push_back(std::make_pair(cit->initialPoint() * i2dt, cit->finalPoint() * i2dt));
            }
        }
    }

    sp_guide_pt_pairs_to_guides(this->document, pts);
}

SPPath::SPPath() : SPShape(), connEndPair(this) {
}

SPPath::~SPPath() {
}

void SPPath::build(SPDocument *document, Inkscape::XML::Node *repr) {
    /* Are these calls actually necessary? */
    this->readAttr( "marker" );
    this->readAttr( "marker-start" );
    this->readAttr( "marker-mid" );
    this->readAttr( "marker-end" );

    sp_conn_end_pair_build(this);

    SPShape::build(document, repr);

    this->readAttr( "inkscape:original-d" );
    this->readAttr( "d" );

    /* d is a required attribute */
    gchar const *d = this->getAttribute("d", NULL);

    if (d == NULL) {
        this->setKeyValue( sp_attribute_lookup("d"), "");
    }
}

void SPPath::release() {
    this->connEndPair.release();

    SPShape::release();
}

void SPPath::set(unsigned int key, const gchar* value) {
    switch (key) {
        case SP_ATTR_INKSCAPE_ORIGINAL_D:
			if (value) {
				Geom::PathVector pv = sp_svg_read_pathv(value);
				SPCurve *curve = new SPCurve(pv);

				if (curve) {
					this->set_original_curve(curve, TRUE, true);
					curve->unref();
				}
			} else {
				this->set_original_curve(NULL, TRUE, true);
			}

			this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

       case SP_ATTR_D:
			if (value) {
				Geom::PathVector pv = sp_svg_read_pathv(value);
				SPCurve *curve = new SPCurve(pv);

				if (curve) {
					this->setCurve(curve, TRUE);
					curve->unref();
				}
			} else {
				this->setCurve(NULL, TRUE);
			}

			this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_PROP_MARKER:
        case SP_PROP_MARKER_START:
        case SP_PROP_MARKER_MID:
        case SP_PROP_MARKER_END:
            sp_shape_set_marker(this, key, value);
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_CONNECTOR_TYPE:
        case SP_ATTR_CONNECTOR_CURVATURE:
        case SP_ATTR_CONNECTION_START:
        case SP_ATTR_CONNECTION_END:
        case SP_ATTR_CONNECTION_START_POINT:
        case SP_ATTR_CONNECTION_END_POINT:
            this->connEndPair.setAttr(key, value);
            break;

        default:
            SPShape::set(key, value);
            break;
    }
}

Inkscape::XML::Node* SPPath::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:path");
    }

#ifdef PATH_VERBOSE
g_message("sp_path_write writes 'd' attribute");
#endif

    if ( this->_curve != NULL ) {
        gchar *str = sp_svg_write_path(this->_curve->get_pathvector());
        repr->setAttribute("d", str);
        g_free(str);
    } else {
        repr->setAttribute("d", NULL);
    }

    if (flags & SP_OBJECT_WRITE_EXT) {
        if ( this->_curve_before_lpe != NULL ) {
            gchar *str = sp_svg_write_path(this->_curve_before_lpe->get_pathvector());
            repr->setAttribute("inkscape:original-d", str);
            g_free(str);
        } else {
            repr->setAttribute("inkscape:original-d", NULL);
        }
    }

    this->connEndPair.writeRepr(repr);

    SPShape::write(xml_doc, repr, flags);

    return repr;
}

void SPPath::update(SPCtx *ctx, guint flags) {
	if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
		flags &= ~SP_OBJECT_USER_MODIFIED_FLAG_B; // since we change the description, it's not a "just translation" anymore
	}

	SPShape::update(ctx, flags);

	this->connEndPair.update();
}

Geom::Affine SPPath::set_transform(Geom::Affine const &transform) {
    if (!_curve) { // 0 nodes, nothing to transform
        return Geom::identity();
    }

    // Transform the original-d path if this is a valid LPE this, other else the (ordinary) path
    if (_curve_before_lpe && hasPathEffectRecursive()) {
        if (this->hasPathEffectOfType(Inkscape::LivePathEffect::CLONE_ORIGINAL)) {
            // if path has the CLONE_ORIGINAL LPE applied, don't write the transform to the pathdata, but write it 'unoptimized'
            return transform;
        } else {
            _curve_before_lpe->transform(transform);
        }
    } else {
        _curve->transform(transform);
    }

    // Adjust stroke
    this->adjust_stroke(transform.descrim());

    // Adjust pattern fill
    this->adjust_pattern(transform);

    // Adjust gradient fill
    this->adjust_gradient(transform);

    // Adjust LPE
    this->adjust_livepatheffect(transform);

    this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

    // nothing remains - we've written all of the transform, so return identity
    return Geom::identity();
}


void SPPath::update_patheffect(bool write) {
    Inkscape::XML::Node *repr = this->getRepr();

#ifdef PATH_VERBOSE
g_message("sp_path_update_patheffect");
#endif

    if (_curve_before_lpe && hasPathEffectRecursive()) {
        SPCurve *curve = _curve_before_lpe->copy();
        /* if a path has an lpeitem applied, then reset the curve to the _curve_before_lpe.
         * This is very important for LPEs to work properly! (the bbox might be recalculated depending on the curve in shape)*/
        this->setCurveInsync(curve, TRUE);

        bool success = this->performPathEffect(curve);

        if (success && write) {
            sp_lpe_item_apply_to_mask(this);
            sp_lpe_item_apply_to_clippath(this);
            // could also do this->getRepr()->updateRepr();  but only the d attribute needs updating.
#ifdef PATH_VERBOSE
g_message("sp_path_update_patheffect writes 'd' attribute");
#endif

            if (_curve) {
                gchar *str = sp_svg_write_path(this->_curve->get_pathvector());
                repr->setAttribute("d", str);
                g_free(str);
            } else {
                repr->setAttribute("d", NULL);
            }
        } else if (!success) {
            // LPE was unsuccesfull. Read the old 'd'-attribute.
            if (gchar const * value = repr->attribute("d")) {
                Geom::PathVector pv = sp_svg_read_pathv(value);
                SPCurve *oldcurve = new SPCurve(pv);

                if (oldcurve) {
                    this->setCurve(oldcurve, TRUE);
                    oldcurve->unref();
                }
            }
        }

        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        curve->unref();
    }
}


/**
 * Adds a original_curve to the path.  If owner is specified, a reference
 * will be made, otherwise the curve will be copied into the path.
 * Any existing curve in the path will be unreferenced first.
 * This routine triggers reapplication of an effect if present
 * and also triggers a request to update the display. Does not write
 * result to XML when write=false.
 */
void SPPath::set_original_curve (SPCurve *new_curve, unsigned int owner, bool write)
{
    if (_curve_before_lpe) {
        _curve_before_lpe = _curve_before_lpe->unref();
    }

    if (new_curve) {
        if (owner) {
            _curve_before_lpe = new_curve->ref();
        } else {
            _curve_before_lpe = new_curve->copy();
        }
    }

    sp_lpe_item_update_patheffect(this, true, write);
    requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Return duplicate of _curve_before_lpe (if any exists) or NULL if there is no curve
 */
SPCurve * SPPath::get_original_curve () const
{
    if (_curve_before_lpe) {
        return _curve_before_lpe->copy();
    }

    return NULL;
}

/**
 * Return duplicate of edittable curve which is _curve_before_lpe if it exists or
 * shape->curve if not.
 */
SPCurve* SPPath::get_curve_for_edit () const
{
    if (_curve_before_lpe && hasPathEffectRecursive()) {
        return get_original_curve();
    } else {
        return getCurve();
    }
}

/**
 * Returns \c _curve_before_lpe if it is not NULL and a valid LPE is applied or
 * \c curve if not.
 */
const SPCurve* SPPath::get_curve_reference () const
{
    if (_curve_before_lpe && hasPathEffectRecursive()) {
        return _curve_before_lpe;
    } else {
        return _curve;
    }
}

/**
 * Returns \c _curve_before_lpe if it is not NULL and a valid LPE is applied or \c curve if not.
 * \todo should only be available to class friends!
 */
SPCurve* SPPath::get_curve ()
{
    if (_curve_before_lpe && hasPathEffectRecursive()) {
        return _curve_before_lpe;
    } else {
        return _curve;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
