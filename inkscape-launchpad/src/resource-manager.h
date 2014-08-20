/*
 * Inkscape::ResourceManager - Manages external resources such as image and css files.
 *
 * Copyright 2011  Jon A. Cruz  <jon@joncruz.org>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_RESOURCE_MANAGER_H
#define SEEN_INKSCAPE_RESOURCE_MANAGER_H

#include <glibmm/object.h>

class SPDocument;

namespace Inkscape {

class ResourceManager : public Glib::Object {

public:
    static ResourceManager& getManager();

    virtual bool fixupBrokenLinks(SPDocument *doc) = 0;

protected:
    ResourceManager();
    virtual ~ResourceManager();

private:
    ResourceManager(ResourceManager const &); // no copy
    void operator=(ResourceManager const &); // no assign
};



} // namespace Inkscape

#endif // SEEN_INKSCAPE_RESOURCE_MANAGER_H

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
