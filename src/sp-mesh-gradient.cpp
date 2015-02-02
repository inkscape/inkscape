#include <glibmm.h>

#include "attributes.h"
#include "display/cairo-utils.h"
#include "xml/repr.h"

#include "sp-mesh-gradient.h"

#include "sp-factory.h"

namespace {
	SPObject* createMeshGradient() {
		return new SPMeshGradient();
	}

	bool meshGradientRegistered = SPFactory::instance().registerObject("svg:meshGradient", createMeshGradient);
}


/*
 * Mesh Gradient
 */
//#define MESH_DEBUG
SPMeshGradient::SPMeshGradient() : SPGradient(), smooth(SP_MESH_SMOOTH_NONE), smooth_set(false) {
    // Start coordinate of mesh
    this->x.unset(SVGLength::NONE, 0.0, 0.0);
    this->y.unset(SVGLength::NONE, 0.0, 0.0);
}

SPMeshGradient::~SPMeshGradient() {
}

void SPMeshGradient::build(SPDocument *document, Inkscape::XML::Node *repr) {
    SPGradient::build(document, repr);

    // Start coordinate of mesh
    this->readAttr( "x" );
    this->readAttr( "y" );

    this->readAttr( "smooth" );
}


void SPMeshGradient::set(unsigned key, gchar const *value) {
    switch (key) {
        case SP_ATTR_X:
            if (!this->x.read(value)) {
                this->x.unset(SVGLength::NONE, 0.0, 0.0);
            }

            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_Y:
            if (!this->y.read(value)) {
                this->y.unset(SVGLength::NONE, 0.0, 0.0);
            }

            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_SMOOTH:
	    if (value) {
	      if (!strcmp(value, "none")) {
		this->smooth = SP_MESH_SMOOTH_NONE;
	      } else if (!strcmp(value, "smooth")) {
		this->smooth = SP_MESH_SMOOTH_SMOOTH;
	      } else if (!strcmp(value, "smooth1")) {
		this->smooth = SP_MESH_SMOOTH_SMOOTH1;
	      } else if (!strcmp(value, "smooth2")) {
		this->smooth = SP_MESH_SMOOTH_SMOOTH2;
	      } else if (!strcmp(value, "smooth3")) {
		this->smooth = SP_MESH_SMOOTH_SMOOTH3;
	      } else if (!strcmp(value, "smooth4")) {
		this->smooth = SP_MESH_SMOOTH_SMOOTH4;
	      } else if (!strcmp(value, "smooth5")) {
		this->smooth = SP_MESH_SMOOTH_SMOOTH5;
	      } else if (!strcmp(value, "smooth6")) {
		this->smooth = SP_MESH_SMOOTH_SMOOTH6;
	      } else if (!strcmp(value, "smooth7")) {
		this->smooth = SP_MESH_SMOOTH_SMOOTH7;
	      } else {
		std::cout << "SPMeshGradient::set(): invalid value " << value << std::endl;
	      }
	      this->smooth_set = TRUE;
	    } else {
	      std::cout << "SPMeshGradient::set() No value " << std::endl;
	      this->smooth = SP_MESH_SMOOTH_NONE;
	      this->smooth_set = FALSE;
	    }

            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        default:
            SPGradient::set(key, value);
            break;
    }
}

/**
 * Write mesh gradient attributes to associated repr.
 */
Inkscape::XML::Node* SPMeshGradient::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
#ifdef MESH_DEBUG
    std::cout << "sp_meshgradient_write() ***************************" << std::endl;
#endif

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:meshGradient");
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || this->x._set) {
    	sp_repr_set_svg_double(repr, "x", this->x.computed);
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || this->y._set) {
    	sp_repr_set_svg_double(repr, "y", this->y.computed);
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || this->smooth_set) {
        switch (this->smooth) {
	    case SP_MESH_SMOOTH_SMOOTH:
	      repr->setAttribute("smooth", "smooth");
	      break;
	    case SP_MESH_SMOOTH_SMOOTH1:
	      repr->setAttribute("smooth", "smooth1");
	      break;
	    case SP_MESH_SMOOTH_SMOOTH2:
	      repr->setAttribute("smooth", "smooth2");
	      break;
	    case SP_MESH_SMOOTH_SMOOTH3:
	      repr->setAttribute("smooth", "smooth3");
	      break;
	    case SP_MESH_SMOOTH_SMOOTH4:
	      repr->setAttribute("smooth", "smooth4");
	      break;
	    case SP_MESH_SMOOTH_SMOOTH5:
	      repr->setAttribute("smooth", "smooth5");
	      break;
	    case SP_MESH_SMOOTH_SMOOTH6:
	      repr->setAttribute("smooth", "smooth6");
	      break;
	    case SP_MESH_SMOOTH_SMOOTH7:
	      repr->setAttribute("smooth", "smooth7");
	      break;
	    case SP_MESH_SMOOTH_NONE:
	      repr->setAttribute("smooth", "none");
	      break;
	    default:
	      // Do nothing
	      break;
	}
    }

    SPGradient::write(xml_doc, repr, flags);

    return repr;
}

void
sp_meshgradient_repr_write(SPMeshGradient *mg)
{
    mg->array.write( mg );
}


cairo_pattern_t* SPMeshGradient::pattern_new(cairo_t * /*ct*/,
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

    this->ensureArray();

    cairo_pattern_t *cp = NULL;

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 11, 4)
    SPMeshNodeArray* my_array = &array;

    if( smooth_set ) {
      switch (smooth) {
        case SP_MESH_SMOOTH_NONE:
	  // std::cout << "SPMeshGradient::pattern_new: no smoothing" << std::endl;
	  break;
	case SP_MESH_SMOOTH_SMOOTH1:
	case SP_MESH_SMOOTH_SMOOTH2:
	case SP_MESH_SMOOTH_SMOOTH3:
	case SP_MESH_SMOOTH_SMOOTH4:
	case SP_MESH_SMOOTH_SMOOTH5:
	  // std::cout << "SPMeshGradient::pattern_new: calling array.smooth" << std::endl;
	  array.smooth( &array_smoothed, smooth );
	  my_array = &array_smoothed;
	  break;
	case SP_MESH_SMOOTH_SMOOTH:
	case SP_MESH_SMOOTH_SMOOTH6:
	case SP_MESH_SMOOTH_SMOOTH7:
	  // std::cout << "SPMeshGradient::pattern_new: calling array.smooth2" << std::endl;
	  array.smooth2( &array_smoothed, smooth );
	  my_array = &array_smoothed;
	  break;
	}
    }

    cp = cairo_pattern_create_mesh();

    for( unsigned int i = 0; i < my_array->patch_rows(); ++i ) {
        for( unsigned int j = 0; j < my_array->patch_columns(); ++j ) {

            SPMeshPatchI patch( &(my_array->nodes), i, j );

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
    Geom::Affine gs2user = this->gradientTransform;
    if (this->getUnits() == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
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
