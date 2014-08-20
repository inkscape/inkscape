/*
 * Authors:
 *   Daniel Wagenaar <daw@caltech.edu>
 *
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifndef IMAGE_RESOLUTION_H

#define IMAGE_RESOLUTION_H

namespace Inkscape {
namespace Extension {
namespace Internal {

class ImageResolution {
public:
    ImageResolution(char const *fn);
    bool ok() const;
    double x() const;
    double y() const;
private:
    bool ok_;
    double x_;
    double y_;
private:
    void readpng(char const *fn);
    void readexif(char const *fn);
    void readexiv(char const *fn);
    void readjfif(char const *fn);
    void readmagick(char const *fn);
};

}
}
}

#endif
