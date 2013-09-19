#include <stdlib.h>

#include "imagemap-gdk.h"


/*#########################################################################
## G R A Y M A P
#########################################################################*/

GrayMap *gdkPixbufToGrayMap(GdkPixbuf *buf)
{
    if (!buf)
        return NULL;

    int width       = gdk_pixbuf_get_width(buf);
    int height      = gdk_pixbuf_get_height(buf);
    guchar *pixdata = gdk_pixbuf_get_pixels(buf);
    int rowstride   = gdk_pixbuf_get_rowstride(buf);
    int n_channels  = gdk_pixbuf_get_n_channels(buf);

    GrayMap *grayMap = GrayMapCreate(width, height);
    if (!grayMap)
        return NULL;

    //### Fill in the odd cells with RGB values
    int x,y;
    int row  = 0;
    for (y=0 ; y<height ; y++)
        {
        guchar *p = pixdata + row;
        for (x=0 ; x<width ; x++)
            {
            int alpha = (int)p[3];
            int white = 3 * (255-alpha);
            unsigned long sample = (int)p[0] + (int)p[1] +(int)p[2];
            unsigned long bright = sample * alpha / 256 + white;
            grayMap->setPixel(grayMap, x, y, bright);
            p += n_channels;
            }
        row += rowstride;
        }

    return grayMap;
}

GdkPixbuf *grayMapToGdkPixbuf(GrayMap *grayMap)
{
    if (!grayMap)
        return NULL;

    guchar *pixdata = (guchar *)
          malloc(sizeof(guchar) * grayMap->width * grayMap->height * 3);
    if (!pixdata)
        return NULL;

    int n_channels = 3;
    int rowstride  = grayMap->width * 3;

    GdkPixbuf *buf = gdk_pixbuf_new_from_data(pixdata, GDK_COLORSPACE_RGB,
                        0, 8, grayMap->width, grayMap->height,
                        rowstride, (GdkPixbufDestroyNotify)g_free, NULL);

    //### Fill in the odd cells with RGB values
    int x,y;
    int row  = 0;
    for (y=0 ; y<grayMap->height ; y++)
        {
        guchar *p = pixdata + row;
        for (x=0 ; x<grayMap->width ; x++)
            {
            unsigned long pix = grayMap->getPixel(grayMap, x, y) / 3;
            p[0] = p[1] = p[2] = (guchar)(pix & 0xff);
            p += n_channels;
            }
        row += rowstride;
        }

    return buf;
}



/*#########################################################################
## P A C K E D    P I X E L    M A P
#########################################################################*/

PackedPixelMap *gdkPixbufToPackedPixelMap(GdkPixbuf *buf)
{
    if (!buf)
        return NULL;

    int width       = gdk_pixbuf_get_width(buf);
    int height      = gdk_pixbuf_get_height(buf);
    guchar *pixdata = gdk_pixbuf_get_pixels(buf);
    int rowstride   = gdk_pixbuf_get_rowstride(buf);
    int n_channels  = gdk_pixbuf_get_n_channels(buf);

    PackedPixelMap *ppMap = PackedPixelMapCreate(width, height);
    if (!ppMap)
        return NULL;

    //### Fill in the cells with RGB values
    int x,y;
    int row  = 0;
    for (y=0 ; y<height ; y++)
        {
        guchar *p = pixdata + row;
        for (x=0 ; x<width ; x++)
            {
            int alpha = (int)p[3];
            int white = 255 - alpha;
            int r     = (int)p[0];  r = r * alpha / 256 + white;
            int g     = (int)p[1];  g = g * alpha / 256 + white;
            int b     = (int)p[2];  b = b * alpha / 256 + white;

            ppMap->setPixel(ppMap, x, y, r, g, b);
            p += n_channels;
            }
        row += rowstride;
        }

    return ppMap;
}


/*#########################################################################
## R G B   M A P
#########################################################################*/

RgbMap *gdkPixbufToRgbMap(GdkPixbuf *buf)
{
    if (!buf)
        return NULL;

    int width       = gdk_pixbuf_get_width(buf);
    int height      = gdk_pixbuf_get_height(buf);
    guchar *pixdata = gdk_pixbuf_get_pixels(buf);
    int rowstride   = gdk_pixbuf_get_rowstride(buf);
    int n_channels  = gdk_pixbuf_get_n_channels(buf);

    RgbMap *rgbMap = RgbMapCreate(width, height);
    if (!rgbMap)
        return NULL;

    //### Fill in the cells with RGB values
    int x,y;
    int row  = 0;
    for (y=0 ; y<height ; y++)
        {
        guchar *p = pixdata + row;
        for (x=0 ; x<width ; x++)
            {
            int alpha = (int)p[3];
            int white = 255 - alpha;
            int r     = (int)p[0];  r = r * alpha / 256 + white;
            int g     = (int)p[1];  g = g * alpha / 256 + white;
            int b     = (int)p[2];  b = b * alpha / 256 + white;

            rgbMap->setPixel(rgbMap, x, y, r, g, b);
            p += n_channels;
            }
        row += rowstride;
        }

    return rgbMap;
}



/*#########################################################################
## I N D E X E D   M A P
#########################################################################*/


GdkPixbuf *indexedMapToGdkPixbuf(IndexedMap *iMap)
{
    if (!iMap)
        return NULL;

    guchar *pixdata = (guchar *)
          malloc(sizeof(guchar) * iMap->width * iMap->height * 3);
    if (!pixdata)
        return NULL;

    int n_channels = 3;
    int rowstride  = iMap->width * 3;

    GdkPixbuf *buf = gdk_pixbuf_new_from_data(pixdata, GDK_COLORSPACE_RGB,
                        0, 8, iMap->width, iMap->height,
                        rowstride, (GdkPixbufDestroyNotify)g_free, NULL);

    //### Fill in the cells with RGB values
    int x,y;
    int row  = 0;
    for (y=0 ; y<iMap->height ; y++)
        {
        guchar *p = pixdata + row;
        for (x=0 ; x<iMap->width ; x++)
            {
            RGB rgb = iMap->getPixelValue(iMap, x, y);
            p[0] = rgb.r & 0xff;
            p[1] = rgb.g & 0xff;
            p[2] = rgb.b & 0xff;
            p += n_channels;
            }
        row += rowstride;
        }

    return buf;
}

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/
