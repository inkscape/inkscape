#ifndef SP_GUIDE_H
#define SP_GUIDE_H

/*
 * SPGuide
 *
 * A guideline
 *
 * Copyright (C) Lauris Kaplinski 2000
 * Copyright (C) Johan Engelen 2007
 *
 */

#include <vector>

#include "display/display-forward.h"
#include "libnr/nr-point.h"
#include "sp-object.h"
#include "sp-guide-attachment.h"

#define SP_TYPE_GUIDE            (sp_guide_get_type())
#define SP_GUIDE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_GUIDE, SPGuide))
#define SP_GUIDE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_GUIDE, SPGuideClass))
#define SP_IS_GUIDE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_GUIDE))
#define SP_IS_GUIDE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_GUIDE))

/* Represents the constraint on p that dot(g.direction, p) == g.position. */
struct SPGuide : public SPObject {
    NR::Point normal;
    gdouble position;
    guint32 color;
    guint32 hicolor;
    GSList *views;
    std::vector<SPGuideAttachment> attached_items;
};

struct SPGuideClass {
	SPObjectClass parent_class;
};

GType sp_guide_get_type();

void sp_guide_show(SPGuide *guide, SPCanvasGroup *group, GCallback handler);
void sp_guide_hide(SPGuide *guide, SPCanvas *canvas);
void sp_guide_sensitize(SPGuide *guide, SPCanvas *canvas, gboolean sensitive);

double sp_guide_position_from_pt(SPGuide const *guide, NR::Point const &pt);
void sp_guide_moveto(SPGuide const &guide, gdouble const position, bool const commit);
void sp_guide_remove(SPGuide *guide);

char *sp_guide_description(SPGuide const *guide);


#endif /* !SP_GUIDE_H */

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
