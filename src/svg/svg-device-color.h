#ifndef SVG_DEVICE_COLOR_H_SEEN
#define SVG_DEVICE_COLOR_H_SEEN

#include <string>
#include <vector>

typedef enum {DEVICE_COLOR_INVALID, DEVICE_GRAY, DEVICE_RGB, DEVICE_CMYK, DEVICE_NCHANNEL} SVGDeviceColorType;

struct SVGDeviceColor {
    SVGDeviceColorType type;
    std::vector<double> colors;
};


#endif /* !SVG_DEVICE_COLOR_H_SEEN */

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
