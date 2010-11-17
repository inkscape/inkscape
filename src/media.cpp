#include "media.h"

void
media_clear_all(Media &media)
{
    media.print = false;
    media.screen = false;
}

void
media_set_all(Media &media)
{
    media.print = true;
    media.screen = true;
}


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
