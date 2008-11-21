#ifndef SP_FECONVOLVEMATRIX_FNS_H
#define SP_FECONVOLVEMATRIX_FNS_H

/** \file 
 * Macros and fn declarations related to gaussian blur filter.
 */

#include <glib-object.h>
#include <glib/gtypes.h>

namespace Inkscape {
namespace XML {
class Node;
}
}

class SPFeConvolveMatrix;

#define SP_TYPE_FECONVOLVEMATRIX (sp_feConvolveMatrix_get_type())
#define SP_FECONVOLVEMATRIX(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FECONVOLVEMATRIX, SPFeConvolveMatrix))
#define SP_FECONVOLVEMATRIX_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FECONVOLVEMATRIX, SPFeConvolveMatrixClass))
#define SP_IS_FECONVOLVEMATRIX(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FECONVOLVEMATRIX))
#define SP_IS_FECONVOLVEMATRIX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FECONVOLVEMATRIX))

GType sp_feConvolveMatrix_get_type();

#endif /* !SP_FECONVOLVEMATRIX_FNS_H */

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
