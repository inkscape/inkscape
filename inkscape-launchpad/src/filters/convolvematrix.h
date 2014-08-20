/** @file
 * @brief SVG matrix convolution filter effect
 */
/*
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SP_FECONVOLVEMATRIX_H_SEEN
#define SP_FECONVOLVEMATRIX_H_SEEN

#include <vector>
#include "sp-filter-primitive.h"
#include "number-opt-number.h"
#include "display/nr-filter-convolve-matrix.h"

#define SP_FECONVOLVEMATRIX(obj) (dynamic_cast<SPFeConvolveMatrix*>((SPObject*)obj))
#define SP_IS_FECONVOLVEMATRIX(obj) (dynamic_cast<const SPFeConvolveMatrix*>((SPObject*)obj) != NULL)

class SPFeConvolveMatrix : public SPFilterPrimitive {
public:
	SPFeConvolveMatrix();
	virtual ~SPFeConvolveMatrix();

    NumberOptNumber order;
    std::vector<gdouble> kernelMatrix;
    double divisor, bias;
    int targetX, targetY;
    Inkscape::Filters::FilterConvolveMatrixEdgeMode edgeMode;
    NumberOptNumber kernelUnitLength;
    bool preserveAlpha;

    bool targetXIsSet;
    bool targetYIsSet;
    bool divisorIsSet;
    bool kernelMatrixIsSet;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void set(unsigned int key, const gchar* value);

	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags);

	virtual void build_renderer(Inkscape::Filters::Filter* filter);
};

#endif /* !SP_FECONVOLVEMATRIX_H_SEEN */

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
