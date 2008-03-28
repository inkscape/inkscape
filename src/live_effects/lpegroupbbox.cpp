#define INKSCAPE_LPEGROUPBBOX_CPP

/*
 * Copyright (C) Steren Giannini 2008 <steren.giannini@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpegroupbbox.h"
#include "sp-shape.h"
#include "sp-item.h"
#include "sp-path.h"
#include "sp-item-group.h"
#include "display/curve.h"
#include <libnr/n-art-bpath.h>
#include <libnr/nr-matrix-fns.h>
#include "live_effects/n-art-bpath-2geom.h"
#include "svg/svg.h"
#include "ui/widget/scalar.h"

#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>

#include <algorithm>

using std::vector;

namespace Inkscape {
namespace LivePathEffect {

void
LivePathEffect_group_bbox::recursive_original_bbox(SPGroup *group, Geom::Piecewise<Geom::D2<Geom::SBasis> > & pwd2, std::vector<Geom::Path> & temppath)
{
            std::vector<Geom::Path> tempsubpath;
            GSList const *item_list = sp_item_group_item_list(group);

            for ( GSList const *iter = item_list; iter; iter = iter->next )
            {
                SPObject *subitem = static_cast<SPObject *>(iter->data);
                if (SP_IS_PATH(subitem))
                {
                    //if there is not an original-d, just take the d
                    if(SP_OBJECT_REPR(subitem)->attribute("inkscape:original-d") != NULL)      
                        tempsubpath = SVGD_to_2GeomPath(SP_OBJECT_REPR(subitem)->attribute("inkscape:original-d"));
                    else
                        tempsubpath = SVGD_to_2GeomPath(SP_OBJECT_REPR(subitem)->attribute("d")); 
                 
                    temppath.insert(temppath.end(), tempsubpath.begin(), tempsubpath.end());
                }else
                if (SP_IS_GROUP(subitem))
                {
                    recursive_original_bbox(SP_GROUP(subitem), pwd2, temppath);
                }

            }
}


} // namespace LivePathEffect
} /* namespace Inkscape */

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
