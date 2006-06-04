#ifndef __SIOX_H__
#define __SIOX_H__
/**
 *  Copyright 2005, 2006 by Gerald Friedland, Kristian Jantz and Lars Knipping
 *
 *  Conversion to C++ for Inkscape by Bob Jamison
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * Note by Bob Jamison:
 * After translating the siox.org Java API to C++ and receiving an
 * education into this wonderful code,  I began again,
 * and started this version using lessons learned.  This version is
 * an attempt to provide an dependency-free SIOX engine that anyone
 * can use in their project with minimal effort.
 *
 * Many thanks to the fine people at siox.org.
 */

#include <string>
#include <vector>

#define HAVE_GLIB

#ifdef HAVE_GLIB
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif


namespace org
{

namespace siox
{


//########################################################################
//#  C L A B
//########################################################################

/**
 *
 */
class CLAB
{
public:

    /**
     *
     */
    CLAB()
        {
        C = 0;
        L = A = B = 0.0f;
        }


    /**
     *
     */
    CLAB(unsigned long rgb);


    /**
     *
     */
    CLAB(float lArg, float aArg, float bArg)
        {
        C = 0;
        L = lArg;
        A = aArg;
        B = bArg;
        }


    /**
     *
     */
    CLAB(const CLAB &other)
        {
        C = other.C;
        L = other.L;
        A = other.A;
        B = other.B;
        }


    /**
     *
     */
    CLAB &operator=(const CLAB &other)
        {
        C = other.C;
        L = other.L;
        A = other.A;
        B = other.B;
        return *this;
        }

    /**
     *
     */
    virtual ~CLAB()
        {}

    /**
     * Retrieve a CLAB value via index.
     */
    virtual float operator()(unsigned int index)
        {
        if      (index==0) return L;
        else if (index==1) return A;
        else if (index==2) return B;
        else return 0;
        }


    /**
     *
     */
    virtual void add(const CLAB &other)
        {
        C += other.C;
        L += other.L;
        A += other.A;
        B += other.B;
        }


    /**
     *
     */
    virtual void mul(float scale)
        {
        L *= scale;
        A *= scale;
        B *= scale;
        }


    /**
     *
     */
    virtual unsigned long toRGB();

    /**
     * Computes squared euclidian distance in CLAB space for two colors
     * given as RGB values.
     */
    static float diffSq(unsigned int rgb1, unsigned int rgb2);

    /**
     * Computes squared euclidian distance in CLAB space for two colors
     * given as RGB values.
     */
    static float diff(unsigned int rgb0, unsigned int rgb1);


    unsigned int C;
    float L;
    float A;
    float B;


};


//########################################################################
//#  S I O X    I M A G E
//########################################################################

/**
 * This is a generic image type that provides a consistent interface
 * to Siox, so that developers will not need to worry about data arrays.
 */
class SioxImage
{
public:

    /**
     *  Create an image with the given width and height
     */
    SioxImage(unsigned int width, unsigned int height);

    /**
     *  Copy constructor
     */
    SioxImage(const SioxImage &other);

    /**
     *  Assignment
     */
    SioxImage &operator=(const SioxImage &other);

    /**
     * Clean up after use.
     */
    virtual ~SioxImage();

    /**
     * Returns true if the previous operation on this image
     * was successful, else false.
     */
    virtual bool isValid();

    /**
     * Sets whether an operation was successful, and whether
     * this image should be considered a valid one.
     * was successful, else false.
     */
    virtual void setValid(bool val);

    /**
     * Set a pixel at the x,y coordinates to the given value.
     * If the coordinates are out of range, do nothing.
     */
    virtual void setPixel(unsigned int x,
                          unsigned int y,
                          unsigned int pixval);

    /**
     * Set a pixel at the x,y coordinates to the given r, g, b values.
     * If the coordinates are out of range, do nothing.
     */
    virtual void setPixel(unsigned int x, unsigned int y,
                          unsigned int a, 
                          unsigned int r, 
                          unsigned int g,
                          unsigned int b);

    /**
     *  Get a pixel at the x,y coordinates given.  If
     *  the coordinates are out of range, return 0
     */
    virtual unsigned int getPixel(unsigned int x, unsigned int y);


    /**
     *  Return the image data buffer
     */
    virtual unsigned int *getImageData();

    /**
     * Set a confidence value at the x,y coordinates to the given value.
     * If the coordinates are out of range, do nothing.
     */
    virtual void setConfidence(unsigned int x,
                               unsigned int y,
                               float conf);

    /**
     *  Get a confidence value at the x,y coordinates given.  If
     *  the coordinates are out of range, return 0
     */
    virtual float getConfidence(unsigned int x, unsigned int y);

    /**
     *  Return the confidence data buffer
     */
    virtual float *getConfidenceData();

    /**
     * Return the width of this image
     */
    virtual int getWidth();

    /**
     * Return the height of this image
     */
    virtual int getHeight();

    /**
     * Saves this image as a simple color PPM
     */
    bool writePPM(const std::string fileName);



#ifdef HAVE_GLIB

    /**
     * Special constructor to create an image from a GdkPixbuf.
     */
    SioxImage(GdkPixbuf *buf);

    /**
     * Creates a GdkPixbuf from this image.  The user must
     * remember to destroy the image when no longer needed.
     * with g_free(pixbuf)
     */
    GdkPixbuf *getGdkPixbuf();

#endif

private:

    SioxImage()
        {}

    /**
     * Assign values to that of another
     */
    void assign(const SioxImage &other);

    /**
     * Initialize values.  Used by constructors
     */
    void init(unsigned int width, unsigned int height);

    bool valid;

    unsigned int width;

    unsigned int height;

    unsigned long imageSize;

    /**
     * Pixel data
     */
    unsigned int *pixdata;

    /**
     * Confidence matrix data
     */
    float *cmdata;
};





//########################################################################
//#  S I O X
//########################################################################

/**
 *
 */
class Siox
{
public:

    /**
     * Confidence corresponding to a certain foreground region (equals one).
     */
    static const float CERTAIN_FOREGROUND_CONFIDENCE; //=1.0f;

    /**
     * Confidence for a region likely being foreground.
     */
    static const float FOREGROUND_CONFIDENCE; //=0.8f;

    /** 
     * Confidence for foreground or background type being equally likely.
     */
    static const float UNKNOWN_REGION_CONFIDENCE; //=0.5f;

    /**
     * Confidence for a region likely being background.
     */
    static const float BACKGROUND_CONFIDENCE; //=0.1f;

    /**
     * Confidence corresponding to a certain background reagion (equals zero).
     */
    static const float CERTAIN_BACKGROUND_CONFIDENCE; //=0.0f;

    /**
     *  Construct a Siox engine
     */
    Siox();

    /**
     *
     */
    virtual ~Siox();

    /**
     *  Extract the foreground of the original image, according
     *  to the values in the confidence matrix.  If the operation fails,
     *  sioxImage.isValid()  will be false.
     *  backgroundFillColor is any ARGB color,  such as 0xffffff (white)
     *  or 0x000000 (black)
     */
    virtual SioxImage extractForeground(const SioxImage &originalImage,
                                        unsigned int backgroundFillColor);

private:

    /**
     * Our signature limits
     */
    float limits[3];

    /**
     * Image width
     */
    unsigned int width;

    /**
     * Image height
     */
    unsigned int height;

    /**
     * Image size in pixels
     */
    unsigned long pixelCount;

    /**
     * Image data
     */
    unsigned int *image;

    /**
     * Image confidence matrix
     */
    float *cm;

    /**
     * Markup for image editing
     */
    int *labelField;


    /**
     * Maximum distance of two lab values.
     */
    float clusterSize;

    /**
     *  Initialize the Siox engine to its 'pristine' state.
     *  Performed at the beginning of extractForeground().
     */
    void init();

    /**
     *  Clean up any debris from processing.
     */
    void cleanup();

    /**
     * Error logging
     */
    void error(char *fmt, ...);

    /**
     * Trace logging
     */
    void trace(char *fmt, ...);

    /**
     *  Stage 1 of the color signature work.  'dims' will be either
     *  2 for grays, or 3 for colors
     */
    void colorSignatureStage1(CLAB *points,
                              unsigned int leftBase,
                              unsigned int rightBase,
                              unsigned int recursionDepth,
                              unsigned int *clusters,
                              const unsigned int dims);

    /**
     *  Stage 2 of the color signature work
     */
    void colorSignatureStage2(CLAB         *points,
                              unsigned int leftBase,
                              unsigned int rightBase,
                              unsigned int recursionDepth,
                              unsigned int *clusters,
                              const float  threshold,
                              const unsigned int dims);

    /**
     *  Main color signature method
     */
    bool colorSignature(const std::vector<CLAB> &inputVec,
                        std::vector<CLAB> &result,
                        const unsigned int dims);


    /**
     *
     */
    void keepOnlyLargeComponents(float threshold,
                                 double sizeFactorToKeep);

    /**
     *
     */
    int depthFirstSearch(int startPos, float threshold, int curLabel);


    /**
     *
     */
    void fillColorRegions();

    /**
     * Applies the morphological dilate operator.
     *
     * Can be used to close small holes in the given confidence matrix.
     */
    void dilate(float *cm, int xres, int yres);

    /**
     * Applies the morphological erode operator.
     */
    void erode(float *cm, int xres, int yres);

    /**
     * Normalizes the matrix to values to [0..1].
     */
    void normalizeMatrix(float *cm, int cmSize);

    /**
     * Multiplies matrix with the given scalar.
     */
    void premultiplyMatrix(float alpha, float *cm, int cmSize);

    /**
     * Blurs confidence matrix with a given symmetrically weighted kernel.
     */
    void smooth(float *cm, int xres, int yres,
                  float f1, float f2, float f3);

    /**
     * Squared Euclidian distance of p and q.
     */
    float sqrEuclidianDist(float *p, int pSize, float *q);

    /**
     * Squared Euclidian distance of p and q.
     */
    float sqrEuclidianDist(const CLAB &p, const CLAB &q);


};




} // namespace siox
} // namespace org

#endif /* __SIOX_H__ */
//########################################################################
//#  E N D    O F    F I L E
//########################################################################



