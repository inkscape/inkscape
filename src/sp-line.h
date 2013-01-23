#ifndef SEEN_SP_LINE_H
#define SEEN_SP_LINE_H

/*
 * SVG <line> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "svg/svg-length.h"
#include "sp-shape.h"



#define SP_TYPE_LINE            (sp_line_get_type())
#define SP_LINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_LINE, SPLine))
#define SP_LINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_LINE, SPLineClass))
#define SP_IS_LINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_LINE))
#define SP_IS_LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_LINE))

class SPLine;
class SPLineClass;

GType sp_line_get_type(void) G_GNUC_CONST;

class SPLine : public SPShape {
public:
    SVGLength x1;
    SVGLength y1;
    SVGLength x2;
    SVGLength y2;

private:
    friend class SPLineClass;
};

class SPLineClass {
public:
    SPShapeClass parent_class;

private:
    friend class SPLine;
};


#endif // SEEN_SP_LINE_H
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
