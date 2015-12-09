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

#include <2geom/point.h>
#include <vector>

#include "sp-object.h"
#include "sp-guide-attachment.h"

typedef unsigned int guint32;
extern "C" {
    typedef void (*GCallback) (void);
}

class SPDesktop;
struct SPCanvas;
struct SPCanvasGroup;
struct SPGuideLine;
#define SP_GUIDE(obj) (dynamic_cast<SPGuide*>((SPObject*)obj))
#define SP_IS_GUIDE(obj) (dynamic_cast<const SPGuide*>((SPObject*)obj) != NULL)

/* Represents the constraint on p that dot(g.direction, p) == g.position. */
class SPGuide : public SPObject {
public:
    SPGuide();
    virtual ~SPGuide() {}

    void set_color(const unsigned r, const unsigned g, const unsigned b, bool const commit);
    void setColor(guint32 c);
    void setHiColor(guint32 h) { hicolor = h; }

    guint32 getColor() const { return color; }
    guint32 getHiColor() const { return hicolor; }
    Geom::Point getPoint() const { return point_on_line; }
    Geom::Point getNormal() const { return normal_to_line; }

    void moveto(Geom::Point const point_on_line, bool const commit);
    void set_normal(Geom::Point const normal_to_line, bool const commit);

    void set_label(const char* label, bool const commit);
    char const* getLabel() const { return label; }

    void set_locked(const bool locked, bool const commit);
    bool getLocked() const { return locked; }

    static SPGuide *createSPGuide(SPDocument *doc, Geom::Point const &pt1, Geom::Point const &pt2);

    void showSPGuide(SPCanvasGroup *group, GCallback handler);
    void hideSPGuide(SPCanvas *canvas);
    void showSPGuide(); // argument-free versions
    void hideSPGuide();

    void sensitize(SPCanvas *canvas, bool sensitive);

    bool isHorizontal() const { return (normal_to_line[Geom::X] == 0.); };
    bool isVertical() const { return (normal_to_line[Geom::Y] == 0.); };

    char* description(bool const verbose = true) const;

    double angle() const { return std::atan2( - normal_to_line[Geom::X], normal_to_line[Geom::Y] ); }
    double getDistanceFrom(Geom::Point const &pt) const;
    Geom::Point getPositionFrom(Geom::Point const &pt) const;

protected:
    virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
    virtual void release();
    virtual void set(unsigned int key, const char* value);

    char* label;
    std::vector<SPGuideLine *> views; // contains an object of type SPGuideline (see display/guideline.cpp for definition)
    bool locked;
    Geom::Point normal_to_line;
    Geom::Point point_on_line;

    guint32 color;
    guint32 hicolor;
public:
    std::vector<SPGuideAttachment> attached_items; // unused
};

// These functions rightfully belong to SPDesktop. What gives?!
void sp_guide_pt_pairs_to_guides(SPDocument *doc, std::list<std::pair<Geom::Point, Geom::Point> > &pts);
void sp_guide_create_guides_around_page(SPDesktop *dt);
void sp_guide_delete_all_guides(SPDesktop *dt);

void sp_guide_remove(SPGuide *guide);

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
