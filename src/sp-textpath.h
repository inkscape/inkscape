#ifndef INKSCAPE_SP_TEXTPATH_H
#define INKSCAPE_SP_TEXTPATH_H

#include <glib/gtypes.h>
#include "svg/svg-length.h"
#include "sp-item.h"
#include "sp-text.h"
class SPUsePath;
class Path;


#define SP_TYPE_TEXTPATH (sp_textpath_get_type())
#define SP_TEXTPATH(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_TEXTPATH, SPTextPath))
#define SP_TEXTPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_TEXTPATH, SPTextPathClass))
#define SP_IS_TEXTPATH(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_TEXTPATH))
#define SP_IS_TEXTPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_TEXTPATH))


struct SPTextPath : public SPItem {
    TextTagAttributes attributes;
    SVGLength startOffset;

    Path *originalPath;
    bool isUpdating;
    SPUsePath *sourcePath;
};

struct SPTextPathClass {
    SPItemClass parent_class;
};

GType sp_textpath_get_type();

#define SP_IS_TEXT_TEXTPATH(obj) (SP_IS_TEXT(obj) && sp_object_first_child(obj) && SP_IS_TEXTPATH(sp_object_first_child(obj)))

SPItem *sp_textpath_get_path_item(SPTextPath *tp);
void sp_textpath_to_text(SPObject *tp);


#endif /* !INKSCAPE_SP_TEXTPATH_H */

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
