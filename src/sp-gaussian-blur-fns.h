#ifndef SP_GAUSSIANBLUR_FNS_H
#define SP_GAUSSIANBLUR_FNS_H

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

class SPGaussianBlur;

#define SP_TYPE_GAUSSIANBLUR (sp_gaussianBlur_get_type())
#define SP_GAUSSIANBLUR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_GAUSSIANBLUR, SPGaussianBlur))
#define SP_GAUSSIANBLUR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_GAUSSIANBLUR, SPGaussianBlurClass))
#define SP_IS_GAUSSIANBLUR(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_GAUSSIANBLUR))
#define SP_IS_GAUSSIANBLUR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_GAUSSIANBLUR))

GType sp_gaussianBlur_get_type();
void  sp_gaussianBlur_setDeviation(SPGaussianBlur *blur, float num);
void  sp_gaussianBlur_setDeviation(SPGaussianBlur *blur, float num, float optnum);

#endif /* !SP_GAUSSIANBLUR_FNS_H */

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
