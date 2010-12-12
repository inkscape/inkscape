#ifndef SEEN_SP_POLYLINE_H
#define SEEN_SP_POLYLINE_H

#include "sp-shape.h"



#define SP_TYPE_POLYLINE            (SPPolyLine::sp_polyline_get_type ())
#define SP_POLYLINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_POLYLINE, SPPolyLine))
#define SP_POLYLINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_POLYLINE, SPPolyLineClass))
#define SP_IS_POLYLINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_POLYLINE))
#define SP_IS_POLYLINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_POLYLINE))

class SPPolyLine;
class SPPolyLineClass;

class SPPolyLine : public SPShape {
public:
    static GType sp_polyline_get_type (void);

private:
    static void init(SPPolyLine *polyline);

    static void build(SPObject * object, SPDocument * document, Inkscape::XML::Node * repr);
    static void set(SPObject *object, unsigned int key, const gchar *value);
    static Inkscape::XML::Node *write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

    static gchar * getDescription(SPItem * item);

    friend class SPPolyLineClass;
};

class SPPolyLineClass {
public:
    SPShapeClass parent_class;

private:
    static SPShapeClass *static_parent_class;
    static void sp_polyline_class_init (SPPolyLineClass *klass);

    friend class SPPolyLine;	
};

#endif // SEEN_SP_POLYLINE_H

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
