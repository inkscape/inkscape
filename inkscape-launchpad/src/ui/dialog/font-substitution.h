/** @file
 * @brief FontSubstitution dialog
 */
/* Authors:
 *
 *
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_FONT_SUBSTITUTION_H
#define INKSCAPE_UI_FONT_SUBSTITUTION_H

#include <glibmm/ustring.h>

class SPItem;
class SPDocument;

namespace Inkscape {
namespace UI {
namespace Dialog {

class FontSubstitution  {
public:
    FontSubstitution();
    virtual ~FontSubstitution();
    void checkFontSubstitutions(SPDocument* doc);
    void show(Glib::ustring out, std::vector<SPItem*> &l);

    static FontSubstitution &getInstance() { return *new FontSubstitution(); }
    Glib::ustring getSubstituteFontName (Glib::ustring font);

protected:
    std::vector<SPItem*> getFontReplacedItems(SPDocument* doc, Glib::ustring *out);

private:
    FontSubstitution(FontSubstitution const &d);
    FontSubstitution& operator=(FontSubstitution const &d);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_FONT_SUBSTITUTION_H

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
