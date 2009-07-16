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
#include <2geom/point.h>
#include "sp-object.h"
#include "sp-guide-attachment.h"

#define SP_TYPE_GUIDE            (sp_guide_get_type())
#define SP_GUIDE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_GUIDE, SPGuide))
#define SP_GUIDE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_GUIDE, SPGuideClass))
#define SP_IS_GUIDE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_GUIDE))
#define SP_IS_GUIDE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_GUIDE))

/* Represents the constraint on p that dot(g.direction, p) == g.position. */
struct SPGuide : public SPObject {
    Geom::Point normal_to_line;
    Geom::Point point_on_line;

    guint32 color;
    guint32 hicolor;
    GSList *views;
    std::vector<SPGuideAttachment> attached_items;

    inline bool is_horizontal() const { return (normal_to_line[Geom::X] == 0.); };
    inline bool is_vertical() const { return (normal_to_line[Geom::Y] == 0.); };
    inline double angle() const { return std::atan2( - normal_to_line[Geom::X], normal_to_line[Geom::Y] ); };
};

struct SPGuideClass {
    SPObjectClass parent_class;
};

GType sp_guide_get_type();

SPGuide *sp_guide_create(SPDesktop *desktop, Geom::Point const &pt1, Geom::Point const &pt2);
void sp_guide_pt_pairs_to_guides(SPDesktop *dt, std::list<std::pair<Geom::Point, Geom::Point> > &pts);
void sp_guide_create_guides_around_page(SPDesktop *dt);

void sp_guide_show(SPGuide *guide, SPCanvasGroup *group, GCallback handler);
void sp_guide_hide(SPGuide *guide, SPCanvas *canvas);
void sp_guide_sensitize(SPGuide *guide, SPCanvas *canvas, gboolean sensitive);

Geom::Point sp_guide_position_from_pt(SPGuide const *guide, Geom::Point const &pt);
double sp_guide_distance_from_pt(SPGuide const *guide, Geom::Point const &pt);
void sp_guide_moveto(SPGuide const &guide, Geom::Point const point_on_line, bool const commit);
void sp_guide_set_normal(SPGuide const &guide, Geom::Point const normal_to_line, bool const commit);
void sp_guide_remove(SPGuide *guide);

char *sp_guide_description(SPGuide const *guide, const bool verbose = true);


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
