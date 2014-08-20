#ifndef INKSCAPE_MEDIA_H
#define INKSCAPE_MEDIA_H

class Media {
public:
    bool print;
    bool screen;
};

void media_clear_all(Media &);
void media_set_all(Media &);

#endif /* !INKSCAPE_MEDIA_H */

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
