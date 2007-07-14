#ifndef __SP_FILTER_ENUMS_H__
#define __SP_FILTER_ENUMS_H__

/*
 * Conversion data for filter and filter primitive enumerations
 *
 * Authors:
 *   Nicholas Bishop
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-blend.h"
#include "display/nr-filter-composite.h"
#include "display/nr-filter-types.h"
#include "util/enums.h"

// Filter primitives
extern const Inkscape::Util::EnumData<NR::FilterPrimitiveType> FPData[NR::NR_FILTER_ENDPRIMITIVETYPE];
extern const Inkscape::Util::EnumDataConverter<NR::FilterPrimitiveType> FPConverter;
// Blend mode
extern const Inkscape::Util::EnumData<NR::FilterBlendMode> BlendModeData[NR::BLEND_ENDMODE];
extern const Inkscape::Util::EnumDataConverter<NR::FilterBlendMode> BlendModeConverter;
// Composite operator
extern const Inkscape::Util::EnumData<FeCompositeOperator> CompositeOperatorData[COMPOSITE_ENDOPERATOR];
extern const Inkscape::Util::EnumDataConverter<FeCompositeOperator> CompositeOperatorConverter;

#endif

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
