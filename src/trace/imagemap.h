#ifndef __IMAGEMAP_H__
#define __IMAGEMAP_H__

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif


/*#########################################################################
### G R A Y M A P
#########################################################################*/


typedef struct GrayMap_def GrayMap;

#define GRAYMAP_BLACK 0
#define GRAYMAP_WHITE 765

/**
 *
 */
struct GrayMap_def
{

    /*#################
    ### METHODS
    #################*/

    /**
     *
     */
    void (*setPixel)(GrayMap *me, int x, int y, unsigned long val);

    /**
     *
     */
    unsigned long (*getPixel)(GrayMap *me, int x, int y);

    /**
     *
     */
    int (*writePPM)(GrayMap *me, char *fileName);



    /**
     *
     */
    void (*destroy)(GrayMap *me);



    /*#################
    ### FIELDS
    #################*/

    /**
     *
     */
    int width;

    /**
     *
     */
    int height;

    /**
     *  The pixel array
     */
    unsigned long *pixels;

    /**
     *  Pointer to the beginning of each row
     */
    unsigned long **rows;

};

#ifdef __cplusplus
extern "C" {
#endif

GrayMap *GrayMapCreate(int width, int height);

#ifdef __cplusplus
}
#endif




/*#########################################################################
### P A C K E D    P I X E L     M A P
#########################################################################*/


typedef struct PackedPixelMap_def PackedPixelMap;

/**
 *
 */
struct PackedPixelMap_def
{

    /*#################
    ### METHODS
    #################*/

    /**
     *
     */
    void (*setPixel)(PackedPixelMap *me, int x, int y, int r, int g, int b);


    /**
     *
     */
    void (*setPixelLong)(PackedPixelMap *me, int x, int y, unsigned long rgb);


    /**
     *
     */
    unsigned long (*getPixel)(PackedPixelMap *me, int x, int y);


    /**
     *
     */
    int (*writePPM)(PackedPixelMap *me, char *fileName);



    /**
     *
     */
    void (*destroy)(PackedPixelMap *me);



    /*#################
    ### FIELDS
    #################*/

    /**
     *
     */
    int width;

    /**
     *
     */
    int height;

    /**
     * The allocated array of pixels
     */
    unsigned long *pixels;

    /**
     * Pointers to the beginning of each row of pixels
     */
    unsigned long **rows;


};



#ifdef __cplusplus
extern "C" {
#endif

PackedPixelMap *PackedPixelMapCreate(int width, int height);

#ifdef __cplusplus
}
#endif



/*#########################################################################
### R G B   M A P
#########################################################################*/

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGB;



typedef struct RgbMap_def RgbMap;

/**
 *
 */
struct RgbMap_def
{

    /*#################
    ### METHODS
    #################*/

    /**
     *
     */
    void (*setPixel)(RgbMap *me, int x, int y, int r, int g, int b);


    /**
     *
     */
    void (*setPixelRGB)(RgbMap *me, int x, int y, RGB rgb);

    /**
     *
     */
    RGB (*getPixel)(RgbMap *me, int x, int y);

    /**
     *
     */
    int (*writePPM)(RgbMap *me, char *fileName);



    /**
     *
     */
    void (*destroy)(RgbMap *me);



    /*#################
    ### FIELDS
    #################*/

    /**
     *
     */
    int width;

    /**
     *
     */
    int height;

    /**
     * The allocated array of pixels
     */
    RGB *pixels;

    /**
     * Pointers to the beginning of each row of pixels
     */
    RGB **rows;

};



#ifdef __cplusplus
extern "C" {
#endif

RgbMap *RgbMapCreate(int width, int height);

#ifdef __cplusplus
}
#endif




/*#########################################################################
### I N D E X E D     M A P
#########################################################################*/


typedef struct IndexedMap_def IndexedMap;

/**
 *
 */
struct IndexedMap_def
{

    /*#################
    ### METHODS
    #################*/

    /**
     *
     */
    void (*setPixel)(IndexedMap *me, int x, int y, unsigned int index);


    /**
     *
     */
    unsigned int (*getPixel)(IndexedMap *me, int x, int y);

    /**
     *
     */
    RGB (*getPixelValue)(IndexedMap *me, int x, int y);

    /**
     *
     */
    int (*writePPM)(IndexedMap *me, char *fileName);



    /**
     *
     */
    void (*destroy)(IndexedMap *me);



    /*#################
    ### FIELDS
    #################*/

    /**
     *
     */
    int width;

    /**
     *
     */
    int height;

    /**
     * The allocated array of pixels
     */
    unsigned int *pixels;

    /**
     * Pointers to the beginning of each row of pixels
     */
    unsigned int **rows;

    /**
     *
     */
    int nrColors;

    /**
     * Color look up table
     */
    RGB clut[256];

};



#ifdef __cplusplus
extern "C" {
#endif

IndexedMap *IndexedMapCreate(int width, int height);

#ifdef __cplusplus
}
#endif




#endif /* __IMAGEMAP_H__ */

/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/
