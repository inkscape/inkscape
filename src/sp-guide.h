#ifndef SEEN_SP_GUIDE_H
#define SEEN_SP_GUIDE_H

/*
 * SPGuide
 *
 * A guideline
 *
 * Copyright (C) Lauris Kaplinski 2000
 * Copyright (C) Johan Engelen 2007
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 */

#include <vector>

#include <2geom/point.h>
#include "sp-object.h"
#include "sp-guide-attachment.h"

struct SPCanvas;
struct SPCanvasGroup;

#define SP_TYPE_GUIDE            (sp_guide_get_type())
#define SP_GUIDE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_GUIDE, SPGuide))
#define SP_GUIDE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_GUIDE, SPGuideClass))
#define SP_IS_GUIDE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_GUIDE))
#define SP_IS_GUIDE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_GUIDE))

/* Represents the constraint on p that dot(g.direction, p) == g.position. */
class SPGuide : public SPObject {
public:
    Geom::Point normal_to_line;
    Geom::Point point_on_line;

    guint32 color;
    guint32 hicolor;
    GSList *views;
    std::vector<SPGuideAttachment> attached_items;

    inline bool isHorizontal() const { return (normal_to_line[Geom::X] == 0.); };
    inline bool isVertical() const { return (normal_to_line[Geom::Y] == 0.); };
    inline double angle() const { return std::atan2( - normal_to_line[Geom::X], normal_to_line[Geom::Y] ); };
    static SPGuide *createSPGuide(SPDesktop *desktop, Geom::Point const &pt1, Geom::Point const &pt2);
    void showSPGuide(SPCanvasGroup *group, GCallback handler);
    void hideSPGuide(SPCanvas *canvas);
    void sensitize(SPCanvas *canvas, gboolean sensitive);
    Geom::Point getPositionFrom(Geom::Point const &pt) const;
    double getDistanceFrom(Geom::Point const &pt) const;
};

class SPGuideClass {
public:
    SPObjectClass parent_class;
};

GType sp_guide_get_type();

void sp_guide_pt_pairs_to_guides(SPDesktop *dt, std::list<std::pair<Geom::Point, Geom::Point> > &pts);
void sp_guide_create_guides_around_page(SPDesktop *dt);

void sp_guide_moveto(SPGuide &guide, Geom::Point const point_on_line, bool const commit);
void sp_guide_set_normal(SPGuide &guide, Geom::Point const normal_to_line, bool const commit);
void sp_guide_remove(SPGuide *guide);

char *sp_guide_description(SPGuide const *guide, const bool verbose = true);


#endif // SEEN_SP_GUIDE_H

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
