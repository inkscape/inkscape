/*
   Copyright 2005, 2006 by Gerald Friedland, Kristian Jantz and Lars Knipping

   Conversion to C++ for Inkscape by Bob Jamison
   
   Released under GNU GPL, read the file 'COPYING' for more information
 */
#include "siox.h"

#include <math.h>
#include <stdarg.h>
#include <map>
#include <algorithm>
#include <cstdlib>


namespace org
{

namespace siox
{



//########################################################################
//#  C L A B
//########################################################################

/**
 * Convert integer A, R, G, B values into an pixel value.
 */
static unsigned long getRGB(int a, int r, int g, int b)
{
    if (a<0)  a=0;
    else if (a>255) a=255;

    if (r<0) r=0;
    else if (r>255) r=255;

    if (g<0) g=0;
    else if (g>255) g=255;

    if (b<0) b=0;
    else if (b>255) b=255;

    return (a<<24)|(r<<16)|(g<<8)|b;
}



/**
 * Convert float A, R, G, B values (0.0-1.0) into an pixel value.
 */
static unsigned long getRGB(float a, float r, float g, float b)
{
    return getRGB((int)(a * 256.0),
                  (int)(r * 256.0),
                  (int)(g * 256.0),
                  (int)(b * 256.0));
}



//#########################################
//# Root approximations for large speedup.
//# By njh!
//#########################################
static const int ROOT_TAB_SIZE = 16;
static float cbrt_table[ROOT_TAB_SIZE +1];

double CieLab::cbrt(double x)
{
    double y = cbrt_table[int(x*ROOT_TAB_SIZE )]; // assuming x \in [0, 1]
    y = (2.0 * y + x/(y*y))/3.0;
    y = (2.0 * y + x/(y*y))/3.0; // polish twice
    return y;
}

static float qn_table[ROOT_TAB_SIZE +1];

double CieLab::qnrt(double x)
{
    double y = qn_table[int(x*ROOT_TAB_SIZE )]; // assuming x \in [0, 1]
    double Y = y*y;
    y = (4.0*y + x/(Y*Y))/5.0;
    Y = y*y;
    y = (4.0*y + x/(Y*Y))/5.0; // polish twice
    return y;
}

double CieLab::pow24(double x)
{
    double onetwo = x*qnrt(x);
    return onetwo*onetwo;
}


static bool _clab_inited_ = false;
void CieLab::init()
{
    if (!_clab_inited_)
        {
        cbrt_table[0] = pow(float(1)/float(ROOT_TAB_SIZE*2), 0.3333);
        qn_table[0]   = pow(float(1)/float(ROOT_TAB_SIZE*2), 0.2);
        for(int i = 1; i < ROOT_TAB_SIZE +1; i++)
            {
            cbrt_table[i] = pow(float(i)/float(ROOT_TAB_SIZE), 0.3333);
            qn_table[i] = pow(float(i)/float(ROOT_TAB_SIZE), 0.2);
            }
        _clab_inited_ = true;
        }
}



/**
 * Construct this CieLab from a packed-pixel ARGB value
 */
CieLab::CieLab(unsigned long rgb)
{
    init();

    int ir  = (rgb>>16) & 0xff;
    int ig  = (rgb>> 8) & 0xff;
    int ib  = (rgb    ) & 0xff;

    float fr = ((float)ir) / 255.0;
    float fg = ((float)ig) / 255.0;
    float fb = ((float)ib) / 255.0;

    //printf("fr:%f fg:%f fb:%f\n", fr, fg, fb);
    if (fr > 0.04045)
        //fr = (float) pow((fr + 0.055) / 1.055, 2.4);
        fr = (float) pow24((fr + 0.055) / 1.055);
    else
        fr = fr / 12.92;

    if (fg > 0.04045)
        //fg = (float) pow((fg + 0.055) / 1.055, 2.4);
        fg = (float) pow24((fg + 0.055) / 1.055);
    else
        fg = fg / 12.92;

    if (fb > 0.04045)
        //fb = (float) pow((fb + 0.055) / 1.055, 2.4);
        fb = (float) pow24((fb + 0.055) / 1.055);
    else
        fb = fb / 12.92;

    // Use white = D65
    const float x = fr * 0.4124 + fg * 0.3576 + fb * 0.1805;
    const float y = fr * 0.2126 + fg * 0.7152 + fb * 0.0722;
    const float z = fr * 0.0193 + fg * 0.1192 + fb * 0.9505;

    float vx = x / 0.95047;
    float vy = y;
    float vz = z / 1.08883;

    //printf("vx:%f vy:%f vz:%f\n", vx, vy, vz);
    if (vx > 0.008856)
        //vx = (float) pow(vx, 0.3333);
        vx = (float) cbrt(vx);
    else
        vx = (7.787 * vx) + (16.0 / 116.0);

    if (vy > 0.008856)
        //vy = (float) pow(vy, 0.3333);
        vy = (float) cbrt(vy);
    else
        vy = (7.787 * vy) + (16.0 / 116.0);

    if (vz > 0.008856)
        //vz = (float) pow(vz, 0.3333);
        vz = (float) cbrt(vz);
    else
        vz = (7.787 * vz) + (16.0 / 116.0);

    C = 0;
    L = 116.0 * vy - 16.0;
    A = 500.0 * (vx - vy);
    B = 200.0 * (vy - vz);
}



/**
 * Return this CieLab's value converted to a packed-pixel ARGB value
 */
unsigned long CieLab::toRGB()
{
    float vy = (L + 16.0) / 116.0;
    float vx = A / 500.0 + vy;
    float vz = vy - B / 200.0;

    float vx3 = vx * vx * vx;
    float vy3 = vy * vy * vy;
    float vz3 = vz * vz * vz;

    if (vy3 > 0.008856)
        vy = vy3;
    else
        vy = (vy - 16.0 / 116.0) / 7.787;

    if (vx3 > 0.008856)
        vx = vx3;
    else
        vx = (vx - 16.0 / 116.0) / 7.787;

    if (vz3 > 0.008856)
        vz = vz3;
    else
        vz = (vz - 16.0 / 116.0) / 7.787;

    vx *= 0.95047; //use white = D65
    vz *= 1.08883;

    float vr =(float)(vx *  3.2406 + vy * -1.5372 + vz * -0.4986);
    float vg =(float)(vx * -0.9689 + vy *  1.8758 + vz *  0.0415);
    float vb =(float)(vx *  0.0557 + vy * -0.2040 + vz *  1.0570);

    if (vr > 0.0031308)
        vr = (float)(1.055 * pow(vr, (1.0 / 2.4)) - 0.055);
    else
        vr = 12.92 * vr;

    if (vg > 0.0031308)
        vg = (float)(1.055 * pow(vg, (1.0 / 2.4)) - 0.055);
    else
        vg = 12.92 * vg;

    if (vb > 0.0031308)
        vb = (float)(1.055 * pow(vb, (1.0 / 2.4)) - 0.055);
    else
        vb = 12.92 * vb;

    return getRGB(0.0, vr, vg, vb);
}


/**
 * Squared Euclidian distance between this and another color
 */
float CieLab::diffSq(const CieLab &other)
{
    float sum=0.0;
    sum += (L - other.L) * (L - other.L);
    sum += (A - other.A) * (A - other.A);
    sum += (B - other.B) * (B - other.B);
    return sum;
}

/**
 * Computes squared euclidian distance in CieLab space for two colors
 * given as RGB values.
 */
float CieLab::diffSq(unsigned int rgb1, unsigned int rgb2)
{
    CieLab c1(rgb1);
    CieLab c2(rgb2);
    float euclid = c1.diffSq(c2);
    return euclid;
}


/**
 * Computes squared euclidian distance in CieLab space for two colors
 * given as RGB values.
 */
float CieLab::diff(unsigned int rgb0, unsigned int rgb1)
{
    return (float) sqrt(diffSq(rgb0, rgb1));
}



//########################################################################
//#  T U P E L
//########################################################################

/**
 * Helper class for storing the minimum distances to a cluster centroid
 * in background and foreground and the index to the centroids in each
 * signature for a given color.
 */
class Tupel {
public:

    Tupel()
        {
        minBgDist  = 0.0f;
        indexMinBg = 0;
        minFgDist  = 0.0f;
        indexMinFg = 0;
        }
    Tupel(float minBgDistArg, long indexMinBgArg,
          float minFgDistArg, long indexMinFgArg)
        {
        minBgDist  = minBgDistArg;
        indexMinBg = indexMinBgArg;
        minFgDist  = minFgDistArg;
        indexMinFg = indexMinFgArg;
        }
    Tupel(const Tupel &other)
        {
        minBgDist  = other.minBgDist;
        indexMinBg = other.indexMinBg;
        minFgDist  = other.minFgDist;
        indexMinFg = other.indexMinFg;
        }
    Tupel &operator=(const Tupel &other)
        {
        minBgDist  = other.minBgDist;
        indexMinBg = other.indexMinBg;
        minFgDist  = other.minFgDist;
        indexMinFg = other.indexMinFg;
        return *this;
        }
    virtual ~Tupel()
        {}

    float minBgDist;
    long  indexMinBg;
    float minFgDist;
    long  indexMinFg;

 };



//########################################################################
//#  S I O X    I M A G E
//########################################################################

/**
 *  Create an image with the given width and height
 */
SioxImage::SioxImage(unsigned int widthArg, unsigned int heightArg)
{
    init(widthArg, heightArg);
}

/**
 *  Copy constructor
 */
SioxImage::SioxImage(const SioxImage &other)
{
    pixdata = NULL;
    cmdata  = NULL;
    assign(other);
}

/**
 *  Assignment
 */
SioxImage &SioxImage::operator=(const SioxImage &other)
{
    assign(other);
    return *this;
}


/**
 * Clean up after use.
 */
SioxImage::~SioxImage()
{
    if (pixdata) delete[] pixdata;
    if (cmdata)  delete[] cmdata;
}

/**
 * Error logging
 */
void SioxImage::error(const char *fmt, ...)
{
    char msgbuf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msgbuf, 255, fmt, args);
    va_end(args) ;
#ifdef HAVE_GLIB
    g_warning("SioxImage error: %s\n", msgbuf);
#else
    fprintf(stderr, "SioxImage error: %s\n", msgbuf);
#endif
}


/**
 * Returns true if the previous operation on this image
 * was successful, else false.
 */
bool SioxImage::isValid()
{
    return valid;
}

/**
 * Sets whether an operation was successful, and whether
 * this image should be considered a valid one.
 * was successful, else false.
 */
void SioxImage::setValid(bool val)
{
    valid = val;
}


/**
 * Set a pixel at the x,y coordinates to the given value.
 * If the coordinates are out of range, do nothing.
 */
void SioxImage::setPixel(unsigned int x,
                         unsigned int y,
                         unsigned int pixval)
{
    if (x >= width || y >= height)
        {
        error("setPixel: out of bounds (%d,%d)/(%d,%d)",
                   x, y, width, height);
        return;
        }
    unsigned long offset = width * y + x;
    pixdata[offset] = pixval;
}

/**
 * Set a pixel at the x,y coordinates to the given r, g, b values.
 * If the coordinates are out of range, do nothing.
 */
void SioxImage::setPixel(unsigned int x, unsigned int y,
                         unsigned int a,
                         unsigned int r,
                         unsigned int g,
                         unsigned int b)
{
    if (x >= width || y >= height)
        {
        error("setPixel: out of bounds (%d,%d)/(%d,%d)",
                   x, y, width, height);
        return;
        }
    unsigned long offset = width * y + x;
    unsigned int pixval = ((a << 24) & 0xff000000) |
                          ((r << 16) & 0x00ff0000) |
                          ((g <<  8) & 0x0000ff00) |
                          ((b      ) & 0x000000ff);
    pixdata[offset] = pixval;
}



/**
 *  Get a pixel at the x,y coordinates given.  If
 *  the coordinates are out of range, return 0;
 */
unsigned int SioxImage::getPixel(unsigned int x, unsigned int y)
{
    if (x >= width || y >= height)
        {
        error("getPixel: out of bounds (%d,%d)/(%d,%d)",
                   x, y, width, height);
        return 0L;
        }
    unsigned long offset = width * y + x;
    return pixdata[offset];
}

/**
 *  Return the image data buffer
 */
unsigned int *SioxImage::getImageData()
{
    return pixdata;
}

/**
 * Set a confidence value at the x,y coordinates to the given value.
 * If the coordinates are out of range, do nothing.
 */
void SioxImage::setConfidence(unsigned int x,
                              unsigned int y,
                              float confval)
{
    if (x >= width || y >= height)
        {
        error("setConfidence: out of bounds (%d,%d)/(%d,%d)",
                   x, y, width, height);
        return;
        }
    unsigned long offset = width * y + x;
    cmdata[offset] = confval;
}

/**
 *  Get a confidence valueat the x,y coordinates given.  If
 *  the coordinates are out of range, return 0;
 */
float SioxImage::getConfidence(unsigned int x, unsigned int y)
{
    if (x >= width || y >= height)
        {
        g_warning("getConfidence: out of bounds (%d,%d)/(%d,%d)",
                   x, y, width, height);
        return 0.0;
        }
    unsigned long offset = width * y + x;
    return cmdata[offset];
}

/**
 *  Return the confidence data buffer
 */
float *SioxImage::getConfidenceData()
{
    return cmdata;
}


/**
 * Return the width of this image
 */
int SioxImage::getWidth()
{
    return width;
}

/**
 * Return the height of this image
 */
int SioxImage::getHeight()
{
    return height;
}

/**
 * Initialize values.  Used by constructors
 */
void SioxImage::init(unsigned int widthArg, unsigned int heightArg)
{
    valid     = true;
    width     = widthArg;
    height    = heightArg;
    imageSize = width * height;
    pixdata   = new unsigned int[imageSize];
    cmdata    = new float[imageSize];
    for (unsigned long i=0 ; i<imageSize ; i++)
        {
        pixdata[i] = 0;
        cmdata[i]  = 0.0;
        }
}

/**
 * Assign values to that of another
 */
void SioxImage::assign(const SioxImage &other)
{
    if (pixdata) delete[] pixdata;
    if (cmdata)  delete[] cmdata;
    valid     = other.valid;
    width     = other.width;
    height    = other.height;
    imageSize = width * height;
    pixdata   = new unsigned int[imageSize];
    cmdata    = new float[imageSize];
    for (unsigned long i=0 ; i<imageSize ; i++)
        {
        pixdata[i] = other.pixdata[i];
        cmdata[i]  = other.cmdata[i];
        }
}


/**
 * Write the image to a PPM file
 */
bool SioxImage::writePPM(const std::string &fileName)
{

    FILE *f = fopen(fileName.c_str(), "wb");
    if (!f)
        return false;

    fprintf(f, "P6 %u %u 255\n", width, height);

    for (unsigned int y=0 ; y<height; y++)
        {
        for (unsigned int x=0 ; x<width ; x++)
            {
            unsigned int rgb = getPixel(x, y);
            //unsigned int alpha = (rgb>>24) & 0xff;
            unsigned int r = ((rgb>>16) & 0xff);
            unsigned int g = ((rgb>> 8) & 0xff);
            unsigned int b = ((rgb    ) & 0xff);
            fputc((unsigned char) r, f);
            fputc((unsigned char) g, f);
            fputc((unsigned char) b, f);
            }
        }
    fclose(f);
    return true;
}


#ifdef HAVE_GLIB

/**
 * Create an image from a GdkPixbuf
 */
SioxImage::SioxImage(GdkPixbuf *buf)
{
    if (!buf)
        return;

    unsigned int width  = gdk_pixbuf_get_width(buf);
    unsigned int height = gdk_pixbuf_get_height(buf);
    init(width, height); //DO THIS NOW!!


    guchar *pixldata    = gdk_pixbuf_get_pixels(buf);
    int rowstride       = gdk_pixbuf_get_rowstride(buf);
    int n_channels      = gdk_pixbuf_get_n_channels(buf);

    //### Fill in the cells with RGB values
    int row  = 0;
    for (unsigned int y=0 ; y<height ; y++)
        {
        guchar *p = pixldata + row;
        for (unsigned int x=0 ; x<width ; x++)
            {
            int r     = (int)p[0];
            int g     = (int)p[1];
            int b     = (int)p[2];
            int alpha = (int)p[3];

            setPixel(x, y, alpha, r, g, b);
            p += n_channels;
            }
        row += rowstride;
        }

}


/**
 * Create a GdkPixbuf from this image
 */
GdkPixbuf *SioxImage::getGdkPixbuf()
{
    bool has_alpha = true;
    int n_channels = has_alpha ? 4 : 3;

    guchar *pixdata = (guchar *)
          malloc(sizeof(guchar) * width * height * n_channels);
    if (!pixdata)
        return NULL;

    int rowstride  = width * n_channels;

    GdkPixbuf *buf = gdk_pixbuf_new_from_data(pixdata,
                        GDK_COLORSPACE_RGB,
                        has_alpha, 8, width, height,
                        rowstride, NULL, NULL);

    //### Fill in the cells with RGB values
    int row  = 0;
    for (unsigned int y=0 ; y < height ; y++)
        {
        guchar *p = pixdata + row;
        for (unsigned x=0 ; x < width ; x++)
            {
            unsigned int rgb = getPixel(x, y);
            p[0] = (rgb >> 16) & 0xff;//r
            p[1] = (rgb >>  8) & 0xff;//g
            p[2] = (rgb      ) & 0xff;//b
            if ( n_channels > 3 )
                {
                p[3] = (rgb >> 24) & 0xff;//a
                }
            p += n_channels;
            }
        row += rowstride;
        }
    free(pixdata);
    return buf;
}

#endif /* GLIB */




//########################################################################
//#  S I O X
//########################################################################

//##############
//## PUBLIC
//##############

/**
 * Confidence corresponding to a certain foreground region (equals one).
 */
const float Siox::CERTAIN_FOREGROUND_CONFIDENCE=1.0f;

/**
 * Confidence for a region likely being foreground.
 */
const float Siox::FOREGROUND_CONFIDENCE=0.8f;

/**
 * Confidence for foreground or background type being equally likely.
 */
const float Siox::UNKNOWN_REGION_CONFIDENCE=0.5f;

/**
 * Confidence for a region likely being background.
 */
const float Siox::BACKGROUND_CONFIDENCE=0.1f;

/**
 * Confidence corresponding to a certain background reagion (equals zero).
 */
const float Siox::CERTAIN_BACKGROUND_CONFIDENCE=0.0f;

/**
 *  Construct a Siox engine
 */
Siox::Siox() :
    sioxObserver(0),
    keepGoing(true),
    width(0),
    height(0),
    pixelCount(0),
    image(0),
    cm(0),
    labelField(0)
{
    init();
}

/**
 *  Construct a Siox engine
 */
Siox::Siox(SioxObserver *observer) :
    sioxObserver(observer),
    keepGoing(true),
    width(0),
    height(0),
    pixelCount(0),
    image(0),
    cm(0),
    labelField(0)
{
    init();
}


/**
 *
 */
Siox::~Siox()
{
    cleanup();
}


/**
 * Error logging
 */
void Siox::error(const char *fmt, ...)
{
    char msgbuf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msgbuf, 255, fmt, args);
    va_end(args) ;
#ifdef HAVE_GLIB
    g_warning("Siox error: %s\n", msgbuf);
#else
    fprintf(stderr, "Siox error: %s\n", msgbuf);
#endif
}

/**
 * Trace logging
 */
void Siox::trace(const char *fmt, ...)
{
    char msgbuf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msgbuf, 255, fmt, args);
    va_end(args) ;
#ifdef HAVE_GLIB
    g_message("Siox: %s\n", msgbuf);
#else
    fprintf(stdout, "Siox: %s\n", msgbuf);
#endif
}



/**
 * Progress reporting
 */
bool Siox::progressReport(float percentCompleted)
{
    if (!sioxObserver)
        return true;

    bool ret = sioxObserver->progress(percentCompleted);

    if (!ret)
      {
      trace("User selected abort");
      keepGoing = false;
      }

    return ret;
}




/**
 *  Extract the foreground of the original image, according
 *  to the values in the confidence matrix.
 */
SioxImage Siox::extractForeground(const SioxImage &originalImage,
                                  unsigned int backgroundFillColor)
{
    trace("### Start");

    init();
    keepGoing = true;

    SioxImage workImage = originalImage;

    //# fetch some info from the image
    width      = workImage.getWidth();
    height     = workImage.getHeight();
    pixelCount = width * height;
    image      = workImage.getImageData();
    cm         = workImage.getConfidenceData();
    labelField = new int[pixelCount];

    trace("### Creating signatures");

    //#### create color signatures
    std::vector<CieLab> knownBg;
    std::vector<CieLab> knownFg;
    CieLab *imageClab = new CieLab[pixelCount];
    for (unsigned long i=0 ; i<pixelCount ; i++)
        {
        float conf = cm[i];
        unsigned int pix = image[i];
        CieLab lab(pix);
        imageClab[i] = lab;
        if (conf <= BACKGROUND_CONFIDENCE)
            knownBg.push_back(lab);
        else if (conf >= FOREGROUND_CONFIDENCE)
            knownFg.push_back(lab);
        }

    /*
    std::vector<CieLab> imageClab;
    for (int y = 0 ; y < workImage.getHeight() ; y++)
        for (int x = 0 ; x < workImage.getWidth() ; x++)
            {
            float cm = workImage.getConfidence(x, y);
            unsigned int pix = workImage.getPixel(x, y);
            CieLab lab(pix);
            imageClab.push_back(lab);
            if (cm <= BACKGROUND_CONFIDENCE)
                knownBg.push_back(lab); //note: uses CieLab(rgb)
            else if (cm >= FOREGROUND_CONFIDENCE)
                knownFg.push_back(lab);
            }
    */

    if (!progressReport(10.0))
        {
        error("User aborted");
        workImage.setValid(false);
        delete[] imageClab;
        delete[] labelField;
        return workImage;
        }

    trace("knownBg:%u knownFg:%u", static_cast<unsigned int>(knownBg.size()), static_cast<unsigned int>(knownFg.size()));


    std::vector<CieLab> bgSignature ;
    if (!colorSignature(knownBg, bgSignature, 3))
        {
        error("Could not create background signature");
        workImage.setValid(false);
        delete[] imageClab;
        delete[] labelField;
        return workImage;
        }

    if (!progressReport(30.0))
        {
        error("User aborted");
        workImage.setValid(false);
        delete[] imageClab;
        delete[] labelField;
        return workImage;
        }


    std::vector<CieLab> fgSignature ;
    if (!colorSignature(knownFg, fgSignature, 3))
        {
        error("Could not create foreground signature");
        workImage.setValid(false);
        delete[] imageClab;
        delete[] labelField;
        return workImage;
        }

    //trace("### bgSignature:%d", bgSignature.size());

    if (bgSignature.empty())
        {
        // segmentation impossible
        error("Signature size is < 1.  Segmentation is impossible");
        workImage.setValid(false);
        delete[] imageClab;
        delete[] labelField;
        return workImage;
        }

    if (!progressReport(30.0))
        {
        error("User aborted");
        workImage.setValid(false);
        delete[] imageClab;
        delete[] labelField;
        return workImage;
        }


    // classify using color signatures,
    // classification cached in hashmap for drb and speedup purposes
    trace("### Analyzing image");

    std::map<unsigned int, Tupel> hs;

    unsigned int progressResolution = pixelCount / 10;

    for (unsigned int i=0; i<pixelCount; i++)
        {
        if (i % progressResolution == 0)
            {
            float progress =
                30.0 + 60.0 * (float)i / (float)pixelCount;
            //trace("### progress:%f", progress);
            if (!progressReport(progress))
                {
                error("User aborted");
                delete[] imageClab;
                delete[] labelField;
                workImage.setValid(false);
                return workImage;
                }
            }

        if (cm[i] >= FOREGROUND_CONFIDENCE)
            {
            cm[i] = CERTAIN_FOREGROUND_CONFIDENCE;
            }
        else if (cm[i] <= BACKGROUND_CONFIDENCE)
            {
            cm[i] = CERTAIN_BACKGROUND_CONFIDENCE;
            }
        else // somewhere in between
            {
            bool isBackground = true;
            std::map<unsigned int, Tupel>::iterator iter = hs.find(i);
            if (iter != hs.end()) //found
                {
                Tupel tupel  = iter->second;
                isBackground = tupel.minBgDist <= tupel.minFgDist;
                }
            else
                {
                CieLab lab   = imageClab[i];
                float minBg  = lab.diffSq(bgSignature[0]);
                int minIndex = 0;
                for (unsigned int j=1; j<bgSignature.size() ; j++)
                    {
                    float d = lab.diffSq(bgSignature[j]);
                    if (d<minBg)
                        {
                        minBg    = d;
                        minIndex = j;
                        }
                    }
                Tupel tupel(0.0f, 0,  0.0f, 0);
                tupel.minBgDist  = minBg;
                tupel.indexMinBg = minIndex;
                float minFg      = 1.0e6f;
                minIndex         = -1;
                for (unsigned int j = 0 ; j < fgSignature.size() ; j++)
                    {
                    float d = lab.diffSq(fgSignature[j]);
                    if (d < minFg)
                        {
                        minFg    = d;
                        minIndex = j;
                        }
                    }
                tupel.minFgDist  = minFg;
                tupel.indexMinFg = minIndex;
                if (fgSignature.empty())
                    {
                    isBackground = (minBg <= clusterSize);
                    // remove next line to force behaviour of old algorithm
                    //error("foreground signature does not exist");
                    //delete[] labelField;
                    //workImage.setValid(false);
                    //return workImage;
                    }
                else
                    {
                    isBackground = minBg < minFg;
                    }
                hs[image[i]] = tupel;
                }

            if (isBackground)
                cm[i] = CERTAIN_BACKGROUND_CONFIDENCE;
            else
                cm[i] = CERTAIN_FOREGROUND_CONFIDENCE;
            }
        }


    delete[] imageClab;

    trace("### postProcessing");


    //#### postprocessing
    smooth(cm, width, height, 0.333f, 0.333f, 0.333f); // average
    normalizeMatrix(cm, pixelCount);
    erode(cm, width, height);
    keepOnlyLargeComponents(UNKNOWN_REGION_CONFIDENCE, 1.0/*sizeFactorToKeep*/);

    //for (int i=0; i < 2/*smoothness*/; i++)
    //    smooth(cm, width, height, 0.333f, 0.333f, 0.333f); // average

    normalizeMatrix(cm, pixelCount);

    for (unsigned int i=0; i < pixelCount; i++)
        {
        if (cm[i] >= UNKNOWN_REGION_CONFIDENCE)
            cm[i] = CERTAIN_FOREGROUND_CONFIDENCE;
        else
            cm[i] = CERTAIN_BACKGROUND_CONFIDENCE;
        }

    keepOnlyLargeComponents(UNKNOWN_REGION_CONFIDENCE, 1.5/*sizeFactorToKeep*/);
    fillColorRegions();
    dilate(cm, width, height);

    if (!progressReport(100.0))
        {
        error("User aborted");
        delete[] labelField;
        workImage.setValid(false);
        return workImage;
        }


    //#### We are done.  Now clear everything but the background
    for (unsigned long i = 0; i<pixelCount ; i++)
        {
        float conf = cm[i];
        if (conf < FOREGROUND_CONFIDENCE)
            image[i] = backgroundFillColor;
        }

    delete[] labelField;

    trace("### Done");
    keepGoing = false;
    return workImage;
}



//##############
//## PRIVATE
//##############

/**
 *  Initialize the Siox engine to its 'pristine' state.
 *  Performed at the beginning of extractForeground().
 */
void Siox::init()
{
    limits[0] = 0.64f;
    limits[1] = 1.28f;
    limits[2] = 2.56f;

    float negLimits[3];
    negLimits[0] = -limits[0];
    negLimits[1] = -limits[1];
    negLimits[2] = -limits[2];

    clusterSize = sqrEuclidianDist(limits, 3, negLimits);
}


/**
 *  Clean up any debris from processing.
 */
void Siox::cleanup()
{
}




/**
 *  Stage 1 of the color signature work.  'dims' will be either
 *  2 for grays, or 3 for colors
 */
void Siox::colorSignatureStage1(CieLab *points,
                                unsigned int leftBase,
                                unsigned int rightBase,
                                unsigned int recursionDepth,
                                unsigned int *clusterCount,
                                const unsigned int dims)
{

    unsigned int currentDim = recursionDepth % dims;
    CieLab point = points[leftBase];
    float min = point(currentDim);
    float max = min;

    for (unsigned int i = leftBase + 1; i < rightBase ; i++)
        {
        point = points[i];
        float curval = point(currentDim);
        if (curval < min) min = curval;
        if (curval > max) max = curval;
        }

    //Do the Rubner-rule split (sounds like a dance)
    if (max - min > limits[currentDim])
        {
        float pivotPoint = (min + max) / 2.0; //average
        unsigned int left  = leftBase;
        unsigned int right = rightBase - 1;

        //# partition points according to the dimension
        while (true)
            {
            while ( true )
                {
                point = points[left];
                if (point(currentDim) > pivotPoint)
                    break;
                left++;
                }
            while ( true )
                {
                point = points[right];
                if (point(currentDim) <= pivotPoint)
                    break;
                right--;
                }

            if (left > right)
                break;

            point = points[left];
            points[left] = points[right];
            points[right] = point;

            left++;
            right--;
            }

        //# Recurse and create sub-trees
        colorSignatureStage1(points, leftBase, left,
                 recursionDepth + 1, clusterCount, dims);
        colorSignatureStage1(points, left, rightBase,
                 recursionDepth + 1, clusterCount, dims);
        }
    else
        {
        //create a leaf
        CieLab newpoint;

        newpoint.C = rightBase - leftBase;

        for (; leftBase < rightBase ; leftBase++)
            {
            newpoint.add(points[leftBase]);
            }

        //printf("clusters:%d\n", *clusters);

        if (newpoint.C != 0)
            newpoint.mul(1.0 / (float)newpoint.C);
        points[*clusterCount] = newpoint;
        (*clusterCount)++;
        }
}



/**
 *  Stage 2 of the color signature work
 */
void Siox::colorSignatureStage2(CieLab         *points,
                                unsigned int leftBase,
                                unsigned int rightBase,
                                unsigned int recursionDepth,
                                unsigned int *clusterCount,
                                const float  threshold,
                                const unsigned int dims)
{
    unsigned int currentDim = recursionDepth % dims;
    CieLab point = points[leftBase];
    float min = point(currentDim);
    float max = min;

    for (unsigned int i = leftBase+ 1; i < rightBase; i++)
        {
        point = points[i];
        float curval = point(currentDim);
        if (curval < min) min = curval;
        if (curval > max) max = curval;
        }

    //Do the Rubner-rule split (sounds like a dance)
    if (max - min > limits[currentDim])
        {
        float pivotPoint = (min + max) / 2.0; //average
        unsigned int left  = leftBase;
        unsigned int right = rightBase - 1;

        //# partition points according to the dimension
        while (true)
            {
            while ( true )
                {
                point = points[left];
                if (point(currentDim) > pivotPoint)
                    break;
                left++;
                }
            while ( true )
                {
                point = points[right];
                if (point(currentDim) <= pivotPoint)
                    break;
                right--;
                }

            if (left > right)
                break;

            point = points[left];
            points[left] = points[right];
            points[right] = point;

            left++;
            right--;
            }

        //# Recurse and create sub-trees
        colorSignatureStage2(points, leftBase, left,
                 recursionDepth + 1, clusterCount, threshold, dims);
        colorSignatureStage2(points, left, rightBase,
                 recursionDepth + 1, clusterCount, threshold, dims);
        }
    else
        {
        //### Create a leaf
        unsigned int sum = 0;
        for (unsigned int i = leftBase; i < rightBase; i++)
            sum += points[i].C;

        if ((float)sum >= threshold)
            {
            float scale = (float)(rightBase - leftBase);
            CieLab newpoint;

            for (; leftBase < rightBase; leftBase++)
                newpoint.add(points[leftBase]);

            if (scale != 0.0)
                newpoint.mul(1.0 / scale);
            points[*clusterCount] = newpoint;
            (*clusterCount)++;
            }
      }
}



/**
 *  Main color signature method
 */
bool Siox::colorSignature(const std::vector<CieLab> &inputVec,
                          std::vector<CieLab> &result,
                          const unsigned int dims)
{

    unsigned int length = inputVec.size();

    if (length < 1) // no error. just don't do anything
        return true;

    CieLab *input = new CieLab [length];

    if (!input)
    {
        error("Could not allocate buffer for signature");
        return false;
    }
    for (unsigned int i=0 ; i < length ; i++)
    {
        input[i] = inputVec[i];
    }

    unsigned int stage1length = 0;
    colorSignatureStage1(input, 0, length, 0, &stage1length, dims);

    unsigned int stage2length = 0;
    colorSignatureStage2(input, 0, stage1length, 0, &stage2length, length * 0.001, dims);

    result.clear();
    for (unsigned int i=0 ; i < stage2length ; i++)
    {
        result.push_back(input[i]);
    }

    delete[] input;

    return true;
}



/**
 *
 */
void Siox::keepOnlyLargeComponents(float threshold,
                                   double sizeFactorToKeep)
{
    for (unsigned long idx = 0 ; idx<pixelCount ; idx++)
        labelField[idx] = -1;

    int curlabel = 0;
    int maxregion= 0;
    int maxblob  = 0;

    // slow but easy to understand:
    std::vector<int> labelSizes;
    for (unsigned long i=0 ; i<pixelCount ; i++)
        {
        int regionCount = 0;
        if (labelField[i] == -1 && cm[i] >= threshold)
            {
            regionCount = depthFirstSearch(i, threshold, curlabel++);
            labelSizes.push_back(regionCount);
            }

        if (regionCount>maxregion)
            {
            maxregion = regionCount;
            maxblob   = curlabel-1;
            }
        }

    for (unsigned int i=0 ; i<pixelCount ; i++)
        {
        if (labelField[i] != -1)
            {
            // remove if the component is to small
            if (labelSizes[labelField[i]] * sizeFactorToKeep < maxregion)
                cm[i] = CERTAIN_BACKGROUND_CONFIDENCE;

            // add maxblob always to foreground
            if (labelField[i] == maxblob)
                cm[i] = CERTAIN_FOREGROUND_CONFIDENCE;
            }
        }

}



int Siox::depthFirstSearch(int startPos,
              float threshold, int curLabel)
{
    // stores positions of labeled pixels, where the neighbours
    // should still be checked for processing:

    //trace("startPos:%d threshold:%f curLabel:%d",
    //     startPos, threshold, curLabel);

    std::vector<int> pixelsToVisit;
    int componentSize = 0;

    if (labelField[startPos]==-1 && cm[startPos]>=threshold)
        {
        labelField[startPos] = curLabel;
        componentSize++;
        pixelsToVisit.push_back(startPos);
        }


    while (!pixelsToVisit.empty())
        {
        int pos = pixelsToVisit[pixelsToVisit.size() - 1];
        pixelsToVisit.erase(pixelsToVisit.end() - 1);
        unsigned int x = pos % width;
        unsigned int y = pos / width;

        // check all four neighbours
        int left = pos - 1;
        if (((int)x)-1>=0 && labelField[left]==-1 && cm[left]>=threshold)
            {
            labelField[left]=curLabel;
            componentSize++;
            pixelsToVisit.push_back(left);
            }

        int right = pos + 1;
        if (x+1 < width && labelField[right]==-1 && cm[right]>=threshold)
            {
            labelField[right]=curLabel;
            componentSize++;
            pixelsToVisit.push_back(right);
            }

        int top = pos - width;
        if (((int)y)-1>=0 && labelField[top]==-1 && cm[top]>=threshold)
            {
            labelField[top]=curLabel;
            componentSize++;
            pixelsToVisit.push_back(top);
            }

        int bottom = pos + width;
        if (y+1 < height && labelField[bottom]==-1
                         && cm[bottom]>=threshold)
            {
            labelField[bottom]=curLabel;
            componentSize++;
            pixelsToVisit.push_back(bottom);
            }

        }
    return componentSize;
}



/**
 *
 */
void Siox::fillColorRegions()
{
    for (unsigned long idx = 0 ; idx<pixelCount ; idx++)
        labelField[idx] = -1;

    //int maxRegion=0; // unused now
    std::vector<int> pixelsToVisit;
    for (unsigned long i=0; i<pixelCount; i++)
        { // for all pixels
        if (labelField[i]!=-1 || cm[i]<UNKNOWN_REGION_CONFIDENCE)
            {
            continue; // already visited or bg
            }

        unsigned int  origColor = image[i];
        unsigned long curLabel  = i+1;
        labelField[i]           = curLabel;
        cm[i]                   = CERTAIN_FOREGROUND_CONFIDENCE;

        // int componentSize = 1;
        pixelsToVisit.push_back(i);
        // depth first search to fill region
        while (!pixelsToVisit.empty())
            {
            int pos = pixelsToVisit[pixelsToVisit.size() - 1];
            pixelsToVisit.erase(pixelsToVisit.end() - 1);
            unsigned int x=pos % width;
            unsigned int y=pos / width;
            // check all four neighbours
            int left = pos-1;
            if (((int)x)-1 >= 0 && labelField[left] == -1
                        && CieLab::diff(image[left], origColor)<1.0)
                {
                labelField[left]=curLabel;
                cm[left]=CERTAIN_FOREGROUND_CONFIDENCE;
                // ++componentSize;
                pixelsToVisit.push_back(left);
                }
            int right = pos+1;
            if (x+1 < width && labelField[right]==-1
                        && CieLab::diff(image[right], origColor)<1.0)
                {
                labelField[right]=curLabel;
                cm[right]=CERTAIN_FOREGROUND_CONFIDENCE;
                // ++componentSize;
                pixelsToVisit.push_back(right);
                }
            int top = pos - width;
            if (((int)y)-1>=0 && labelField[top]==-1
                        && CieLab::diff(image[top], origColor)<1.0)
                {
                labelField[top]=curLabel;
                cm[top]=CERTAIN_FOREGROUND_CONFIDENCE;
                // ++componentSize;
                pixelsToVisit.push_back(top);
                }
            int bottom = pos + width;
            if (y+1 < height && labelField[bottom]==-1
                        && CieLab::diff(image[bottom], origColor)<1.0)
                {
                labelField[bottom]=curLabel;
                cm[bottom]=CERTAIN_FOREGROUND_CONFIDENCE;
                // ++componentSize;
                pixelsToVisit.push_back(bottom);
                }
            }
        //if (componentSize>maxRegion) {
        //    maxRegion=componentSize;
        //}
        }
}




/**
 * Applies the morphological dilate operator.
 *
 * Can be used to close small holes in the given confidence matrix.
 */
void Siox::dilate(float *cm, int xres, int yres)
{

    for (int y=0; y<yres; y++)
        {
        for (int x=0; x<xres-1; x++)
             {
             int idx=(y*xres)+x;
             if (cm[idx+1]>cm[idx])
                 cm[idx]=cm[idx+1];
             }
        }

    for (int y=0; y<yres; y++)
        {
        for (int x=xres-1; x>=1; x--)
            {
            int idx=(y*xres)+x;
            if (cm[idx-1]>cm[idx])
                cm[idx]=cm[idx-1];
            }
        }

    for (int y=0; y<yres-1; y++)
        {
        for (int x=0; x<xres; x++)
            {
            int idx=(y*xres)+x;
            if (cm[((y+1)*xres)+x] > cm[idx])
                cm[idx]=cm[((y+1)*xres)+x];
            }
        }

    for (int y=yres-1; y>=1; y--)
        {
        for (int x=0; x<xres; x++)
            {
            int idx=(y*xres)+x;
            if (cm[((y-1)*xres)+x] > cm[idx])
                cm[idx]=cm[((y-1)*xres)+x];
            }
        }
}

/**
 * Applies the morphological erode operator.
 */
void Siox::erode(float *cm, int xres, int yres)
{
    for (int y=0; y<yres; y++)
        {
        for (int x=0; x<xres-1; x++)
            {
            int idx=(y*xres)+x;
            if (cm[idx+1] < cm[idx])
                cm[idx]=cm[idx+1];
            }
        }
    for (int y=0; y<yres; y++)
        {
        for (int x=xres-1; x>=1; x--)
            {
            int idx=(y*xres)+x;
            if (cm[idx-1] < cm[idx])
                cm[idx]=cm[idx-1];
            }
        }
    for (int y=0; y<yres-1; y++)
        {
        for (int x=0; x<xres; x++)
            {
            int idx=(y*xres)+x;
            if (cm[((y+1)*xres)+x] < cm[idx])
                cm[idx]=cm[((y+1)*xres)+x];
            }
        }
    for (int y=yres-1; y>=1; y--)
        {
        for (int x=0; x<xres; x++)
            {
            int idx=(y*xres)+x;
            if (cm[((y-1)*xres)+x] < cm[idx])
                cm[idx]=cm[((y-1)*xres)+x];
            }
        }
}



/**
 * Normalizes the matrix to values to [0..1].
 */
void Siox::normalizeMatrix(float *cm, int cmSize)
{
    float max= -1000000.0f;
    for (int i=0; i<cmSize; i++)
        if (cm[i] > max) max=cm[i];

    //good to use STL, but max() is not iterative
    //float max = *std::max(cm, cm + cmSize);

    if (max<=0.0 || max==1.0)
        return;

    float alpha=1.00f/max;
    premultiplyMatrix(alpha, cm, cmSize);
}

/**
 * Multiplies matrix with the given scalar.
 */
void Siox::premultiplyMatrix(float alpha, float *cm, int cmSize)
{
    for (int i=0; i<cmSize; i++)
        cm[i]=alpha*cm[i];
}

/**
 * Blurs confidence matrix with a given symmetrically weighted kernel.
 *
 * In the standard case confidence matrix entries are between 0...1 and
 * the weight factors sum up to 1.
 */
void Siox::smooth(float *cm, int xres, int yres,
                  float f1, float f2, float f3)
{
    for (int y=0; y<yres; y++)
        {
        for (int x=0; x<xres-2; x++)
            {
            int idx=(y*xres)+x;
            cm[idx]=f1*cm[idx]+f2*cm[idx+1]+f3*cm[idx+2];
            }
        }
    for (int y=0; y<yres; y++)
        {
        for (int x=xres-1; x>=2; x--)
            {
            int idx=(y*xres)+x;
            cm[idx]=f3*cm[idx-2]+f2*cm[idx-1]+f1*cm[idx];
            }
        }
    for (int y=0; y<yres-2; y++)
        {
        for (int x=0; x<xres; x++)
            {
            int idx=(y*xres)+x;
            cm[idx]=f1*cm[idx]+f2*cm[((y+1)*xres)+x]+f3*cm[((y+2)*xres)+x];
            }
        }
    for (int y=yres-1; y>=2; y--)
        {
        for (int x=0; x<xres; x++)
            {
            int idx=(y*xres)+x;
            cm[idx]=f3*cm[((y-2)*xres)+x]+f2*cm[((y-1)*xres)+x]+f1*cm[idx];
            }
        }
}

/**
 * Squared Euclidian distance of p and q.
 */
float Siox::sqrEuclidianDist(float *p, int pSize, float *q)
{
    float sum=0.0;
    for (int i=0; i<pSize; i++)
        {
        float v = p[i] - q[i];
        sum += v*v;
        }
    return sum;
}






} // namespace siox
} // namespace org

//########################################################################
//#  E N D    O F    F I L E
//########################################################################

