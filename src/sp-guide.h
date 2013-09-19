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
class SPDesktop;

#define SP_GUIDE(obj) (dynamic_cast<SPGuide*>((SPObject*)obj))
#define SP_IS_GUIDE(obj) (dynamic_cast<const SPGuide*>((SPObject*)obj) != NULL)

/* Represents the constraint on p that dot(g.direction, p) == g.position. */
class SPGuide : public SPObject {
public:
	SPGuide();
	virtual ~SPGuide();

    char* label;
    Geom::Point normal_to_line;
    Geom::Point point_on_line;

    guint32 color;
    guint32 hicolor;
    GSList *views;
    std::vector<SPGuideAttachment> attached_items;

    guint32 getColor() const;
    guint32 getHiColor() const;
    void setColor(guint32 c);
    void setHiColor(guint32 h);

    inline bool isHorizontal() const { return (normal_to_line[Geom::X] == 0.); };
    inline bool isVertical() const { return (normal_to_line[Geom::Y] == 0.); };
    inline double angle() const { return std::atan2( - normal_to_line[Geom::X], normal_to_line[Geom::Y] ); };
    static SPGuide *createSPGuide(SPDocument *doc, Geom::Point const &pt1, Geom::Point const &pt2);
    void showSPGuide(SPCanvasGroup *group, GCallback handler);
    void hideSPGuide(SPCanvas *canvas);
    void sensitize(SPCanvas *canvas, gboolean sensitive);
    Geom::Point getPositionFrom(Geom::Point const &pt) const;
    double getDistanceFrom(Geom::Point const &pt) const;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();
	virtual void set(unsigned int key, const gchar* value);
};

void sp_guide_pt_pairs_to_guides(SPDocument *doc, std::list<std::pair<Geom::Point, Geom::Point> > &pts);
void sp_guide_create_guides_around_page(SPDesktop *dt);
void sp_guide_delete_all_guides(SPDesktop *dt);

void sp_guide_moveto(SPGuide &guide, Geom::Point const point_on_line, bool const commit);
void sp_guide_set_normal(SPGuide &guide, Geom::Point const normal_to_line, bool const commit);
void sp_guide_set_label(SPGuide &guide, const char* label, bool const commit);
void sp_guide_set_color(SPGuide &guide, const unsigned r, const unsigned g, const unsigned b, bool const commit);
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
