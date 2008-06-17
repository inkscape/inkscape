#ifndef __SVG_H__
#define __SVG_H__

/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright(C) 2005-2008 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or(at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * =======================================================================
 * NOTES
 *
 * This API follows:
 * http://www.w3.org/TR/SVG11/svgdom.html
 *
 * This file defines the main SVG-DOM Node types.  Other non-Node types are
 * defined in svgtypes.h.
 *    
 */


// For access to DOM2 core
#include "dom/dom.h"

// For access to DOM2 events
#include "dom/events.h"

// For access to those parts from DOM2 CSS OM used by SVG DOM.
#include "dom/css.h"

// For access to those parts from DOM2 Views OM used by SVG DOM.
#include "dom/views.h"

// For access to the SMIL OM used by SVG DOM.
#include "dom/smil.h"



#include "svgtypes.h"

#include <math.h>

#define SVG_NAMESPACE "http://www.w3.org/2000/svg"


namespace org
{
namespace w3c
{
namespace dom
{
namespace svg
{


//local definitions
typedef dom::DOMString DOMString;
typedef dom::DOMException DOMException;
typedef dom::Element Element;
typedef dom::ElementPtr ElementPtr;
typedef dom::Document Document;
typedef dom::DocumentPtr DocumentPtr;
typedef dom::NodeList NodeList;




class SVGElement;
typedef Ptr<SVGElement> SVGElementPtr;
class SVGDocument;
typedef Ptr<SVGDocument> SVGDocumentPtr;



//########################################################################
//########################################################################
//#   V A L U E S
//########################################################################
//########################################################################





/*#########################################################################
## SVGAngle
#########################################################################*/

/**
 *
 */
class SVGAngle
{
public:

    /**
     *  Angle Unit Types
     */
    typedef enum
        {
        SVG_ANGLETYPE_UNKNOWN     = 0,
        SVG_ANGLETYPE_UNSPECIFIED = 1,
        SVG_ANGLETYPE_DEG         = 2,
        SVG_ANGLETYPE_RAD         = 3,
        SVG_ANGLETYPE_GRAD        = 4
        } AngleUnitType;



    /**
     *
     */
    unsigned short getUnitType();

    /**
     *
     */
    double getValue();

    /**
     *
     */
    void setValue(double val) throw(DOMException);

    /**
     *
     */
    double getValueInSpecifiedUnits();

    /**
     *
     */
    void setValueInSpecifiedUnits(double /*val*/)
                                     throw(DOMException);

    /**
     *
     */
    DOMString getValueAsString();

    /**
     *
     */
    void setValueAsString(const DOMString &/*val*/)
                                  throw(DOMException);

    /**
     *
     */
    void newValueSpecifiedUnits(unsigned short /*unitType*/,
                                double /*valueInSpecifiedUnits*/);

    /**
     *
     */
    void convertToSpecifiedUnits(unsigned short /*unitType*/);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGAngle();

    /**
     *
     */
    SVGAngle(const SVGAngle &other);

    /**
     *
     */
    ~SVGAngle();

protected:

    int unitType;

    double value;

};




/*#########################################################################
## SVGColor
#########################################################################*/

/**
 *
 */
class SVGColor : public css::CSSValue
{
public:


    /**
     * Color Types
     */
    typedef enum
        {
        SVG_COLORTYPE_UNKNOWN           = 0,
        SVG_COLORTYPE_RGBCOLOR          = 1,
        SVG_COLORTYPE_RGBCOLOR_ICCCOLOR = 2,
        SVG_COLORTYPE_CURRENTCOLOR      = 3
        } ColorType;


    /**
     *
     */
    unsigned short getColorType();

    /**
     *
     */
    css::RGBColor getRgbColor();

    /**
     *
     */
    SVGICCColor getIccColor();


    /**
     *
     */
    void setRGBColor(const DOMString& /*rgbColor*/)
                              throw(SVGException);

    /**
     *
     */
    void setRGBColorICCColor(const DOMString& /*rgbColor*/,
                                      const DOMString& /*iccColor*/)
                                      throw(SVGException);

    /**
     *
     */
    void setColor(unsigned short /*colorType*/,
                           const DOMString& /*rgbColor*/,
                           const DOMString& /*iccColor*/)
                           throw(SVGException);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGColor();

    /**
     *
     */
    SVGColor(const SVGColor &other) : css::CSSValue(other);

    /**
     *
     */
    ~SVGColor();

protected:

    int colorType;

};




/*#########################################################################
## SVGICCColor
#########################################################################*/

/**
 *
 */
class SVGICCColor
{
public:

    /**
     *
     */
    DOMString getColorProfile();

    /**
     *
     */
    void setColorProfile(const DOMString &val) throw(DOMException);

    /**
     *
     */
    SVGNumberList &getColors();



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGICCColor();

    /**
     *
     */
    SVGICCColor(const SVGICCColor &other);

    /**
     *
     */
    ~SVGICCColor();

protected:

    DOMString colorProfile;

    SVGNumberList colors;

};


/*#########################################################################
## SVGLength
#########################################################################*/

/**
 *
 */
class SVGLength
{
public:

    /**
     * Length Unit Types
     */
    typedef enum
        {
        SVG_LENGTHTYPE_UNKNOWN    = 0,
        SVG_LENGTHTYPE_NUMBER     = 1,
        SVG_LENGTHTYPE_PERCENTAGE = 2,
        SVG_LENGTHTYPE_EMS        = 3,
        SVG_LENGTHTYPE_EXS        = 4,
        SVG_LENGTHTYPE_PX         = 5,
        SVG_LENGTHTYPE_CM         = 6,
        SVG_LENGTHTYPE_MM         = 7,
        SVG_LENGTHTYPE_IN         = 8,
        SVG_LENGTHTYPE_PT         = 9,
        SVG_LENGTHTYPE_PC         = 10
        } LengthUnitType;

    /**
     *
     */
    unsigned short getUnitType();

    /**
     *
     */
    double getValue();

    /**
     *
     */
    void setValue(double val)  throw(DOMException);

    /**
     *
     */
    double getValueInSpecifiedUnits();

    /**
     *
     */
    void setValueInSpecifiedUnits(double /*val*/) throw(DOMException);

    /**
     *
     */
    DOMString getValueAsString();

    /**
     *
     */
    void setValueAsString(const DOMString& /*val*/) throw(DOMException);

    /**
     *
     */
    void newValueSpecifiedUnits(unsigned short /*unitType*/, double /*val*/);

    /**
     *
     */
    void convertToSpecifiedUnits(unsigned short /*unitType*/);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGLength();

    /**
     *
     */
    SVGLength(const SVGLength &other);

    /**
     *
     */
    ~SVGLength();

protected:

    int unitType;

    double value;

};

/*#########################################################################
## SVGMatrix
#########################################################################*/

/**
 *  In SVG, a Matrix is defined like this:
 *
 * | a  c  e |
 * | b  d  f |
 * | 0  0  1 |
 *
 */
class SVGMatrix
{
public:


    /**
     *
     */
    double getA();

    /**
     *
     */
    void setA(double val) throw(DOMException);

    /**
     *
     */
    double getB();

    /**
     *
     */
    void setB(double val) throw(DOMException);

    /**
     *
     */
    double getC();

    /**
     *
     */
    void setC(double val) throw(DOMException);

    /**
     *
     */
    double getD();

    /**
     *
     */
    void setD(double val) throw(DOMException);

    /**
     *
     */
    double getE();

    /**
     *
     */
    void setE(double val) throw(DOMException);

    /**
     *
     */
    double getF();

    /**
     *
     */
    void setF(double val) throw(DOMException);


    /**
     * Return the result of postmultiplying this matrix with another.
     */
    SVGMatrix multiply(const SVGMatrix &other);

    /**
     *  Calculate the inverse of this matrix
     *
     *
     *   The determinant of a 3x3 matrix E
     *     (let's use our own notation for a bit)
     *
     *       A  B  C
     *       D  E  F
     *       G  H  I
     *   is
     *       AEI - AFH - BDI + BFG + CDH - CEG
     *
     *   Since in our affine transforms, G and H==0 and I==1,
     *   this reduces to:
     *       AE - BD
     *   In SVG's naming scheme, that is:  a * d - c * b .  SIMPLE!
     *
     *   In a similar method of attack, SVG's adjunct matrix is:
     *
     *      d  -c   cf-ed
     *     -b   a   eb-af
     *      0   0   ad-cb
     *
     *   To get the inverse matrix, we divide the adjunct matrix by
     *   the determinant.  Notice that(ad-cb)/(ad-cb)==1.  Very cool.
     *   So what we end up with is this:
     *
     *      a =  d/(ad-cb)  c = -c/(ad-cb)   e =(cf-ed)/(ad-cb)
     *      b = -b/(ad-cb)  d =  a/(ad-cb)   f =(eb-af)/(ad-cb)
     *
     *  (Since this would be in all SVG-DOM implementations,
     *    somebody needed to document this!  ^^)
     *
     */
    SVGMatrix inverse() throw(SVGException);

    /**
     * Equivalent to multiplying by:
     *  | 1  0  x |
     *  | 0  1  y |
     *  | 0  0  1 |
     *
     */
    SVGMatrix translate(double x, double y);

    /**
     * Equivalent to multiplying by:
     *  | scale  0      0 |
     *  | 0      scale  0 |
     *  | 0      0      1 |
     *
     */
    SVGMatrix scale(double scale);

    /**
     * Equivalent to multiplying by:
     *  | scaleX  0       0 |
     *  | 0       scaleY  0 |
     *  | 0       0       1 |
     *
     */
    SVGMatrix scaleNonUniform(double scaleX, double scaleY);

    /**
     * Equivalent to multiplying by:
     *  | cos(a) -sin(a)   0 |
     *  | sin(a)  cos(a)   0 |
     *  | 0       0        1 |
     *
     */
    SVGMatrix rotate(double angle);

    /**
     * Equivalent to multiplying by:
     *  | cos(a) -sin(a)   0 |
     *  | sin(a)  cos(a)   0 |
     *  | 0       0        1 |
     *  In this case, angle 'a' is computed as the artangent
     *  of the slope y/x .  It is negative if the slope is negative.
     */
    SVGMatrix rotateFromVector(double x, double y) throw(SVGException);

    /**
     * Equivalent to multiplying by:
     *  | -1   0   0 |
     *  | 0    1   0 |
     *  | 0    0   1 |
     *
     */
    SVGMatrix flipX();

    /**
     * Equivalent to multiplying by:
     *  | 1   0   0 |
     *  | 0  -1   0 |
     *  | 0   0   1 |
     *
     */
    SVGMatrix flipY();

    /**
     *  | 1   tan(a)  0 |
     *  | 0   1       0 |
     *  | 0   0       1 |
     *
     */
    SVGMatrix skewX(double angle);

    /**
     * Equivalent to multiplying by:
     *  | 1       0   0 |
     *  | tan(a)  1   0 |
     *  | 0       0   1 |
     *
     */
    SVGMatrix skewY(double angle);


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGMatrix();

    /**
     *
     */
    SVGMatrix(double aArg, double bArg, double cArg,
              double dArg, double eArg, double fArg);

    /**
     * Copy constructor
     */
    SVGMatrix(const SVGMatrix &other);

    /**
     *
     */
    ~SVGMatrix() {}

protected:

friend class SVGTransform;

    /*
     * Set to the identify matrix
     */
    void identity();

    double a, b, c, d, e, f;

};


/*#########################################################################
## SVGNumber
#########################################################################*/

/**
 *
 */
class SVGNumber
{
public:

    /**
     *
     */
    double getValue();

    /**
     *
     */
    void setValue(double val) throw(DOMException);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGNumber();

    /**
     *
     */
    SVGNumber(const SVGNumber &other);

    /**
     *
     */
    ~SVGNumber();

protected:

    double value;

};

/*#########################################################################
## SVGPoint
#########################################################################*/

/**
 *
 */
class SVGPoint
{
public:

    /**
     *
     */
    double getX();

    /**
     *
     */
    void setX(double val) throw(DOMException);

    /**
     *
     */
    double getY();

    /**
     *
     */
    void setY(double val) throw(DOMException);

    /**
     *
     */
    SVGPoint matrixTransform(const SVGMatrix &/*matrix*/);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPoint();

    /**
     *
     */
    SVGPoint(const SVGPoint &other);

    /**
     *
     */
    ~SVGPoint();

protected:

    double x, y;
};


/*#########################################################################
## SVGPathSeg
#########################################################################*/

/**
 *
 */
class SVGPathSeg
{
public:

    /**
     *  Path Segment Types
     */
    typedef enum
        {
        PATHSEG_UNKNOWN                      = 0,
        PATHSEG_CLOSEPATH                    = 1,
        PATHSEG_MOVETO_ABS                   = 2,
        PATHSEG_MOVETO_REL                   = 3,
        PATHSEG_LINETO_ABS                   = 4,
        PATHSEG_LINETO_REL                   = 5,
        PATHSEG_CURVETO_CUBIC_ABS            = 6,
        PATHSEG_CURVETO_CUBIC_REL            = 7,
        PATHSEG_CURVETO_QUADRATIC_ABS        = 8,
        PATHSEG_CURVETO_QUADRATIC_REL        = 9,
        PATHSEG_ARC_ABS                      = 10,
        PATHSEG_ARC_REL                      = 11,
        PATHSEG_LINETO_HORIZONTAL_ABS        = 12,
        PATHSEG_LINETO_HORIZONTAL_REL        = 13,
        PATHSEG_LINETO_VERTICAL_ABS          = 14,
        PATHSEG_LINETO_VERTICAL_REL          = 15,
        PATHSEG_CURVETO_CUBIC_SMOOTH_ABS     = 16,
        PATHSEG_CURVETO_CUBIC_SMOOTH_REL     = 17,
        PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS = 18,
        PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL = 19
        } PathSegmentType;

    /**
     *
     */
    unsigned short getPathSegType();

    /**
     *
     */
    DOMString getPathSegTypeAsLetter();

    /**
     * From the various subclasses
     */

    /**
     *
     */
    double getX();

    /**
     *
     */
    void setX(double val) throw(DOMException);

    /**
     *
     */
    double getX1();

    /**
     *
     */
    void setX1(double val) throw(DOMException);

    /**
     *
     */
    double getX2();

    /**
     *
     */
    void setX2(double val) throw(DOMException);

    /**
     *
     */
    double getY()

    /**
     *
     */
    void setY(double val) throw(DOMException);

    /**
     *
     */
    double getY1()

    /**
     *
     */
    void setY1(double val) throw(DOMException);

    /**
     *
     */
    double getY2();

    /**
     *
     */
    void setY2(double val) throw(DOMException);

    /**
     *
     */
    double getR1();

    /**
     *
     */
    void setR1(double val) throw(DOMException);

    /**
     *
     */
    double getR2();

    /**
     *
     */
    void setR2(double val) throw(DOMException);

    /**
     *
     */
    double getAngle();

    /**
     *
     */
    void setAngle(double val) throw(DOMException);

    /**
     *
     */
    bool getLargeArcFlag();

    /**
     *
     */
    void setLargeArcFlag(bool val) throw(DOMException);

    /**
     *
     */
    bool getSweepFlag();

    /**
     *
     */
    void setSweepFlag(bool val) throw(DOMException);


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSeg();

    /**
     *
     */
    SVGPathSeg(int typeArg);

    /**
     *
     */
    SVGPathSeg(const SVGPathSeg &other);

    /**
     *
     */
    SVGPathSeg &operator=(const SVGPathSeg &other);

    /**
     *
     */
    ~SVGPathSeg();

protected:

    void init();
    
    void assign(const SVGPathSeg &other);

    int type;
    double x, y, x1, y1, x2, y2;
    double r1, r2;
    double angle;
    bool largeArcFlag;
    bool sweepFlag;
};




/*#########################################################################
## SVGPaint
#########################################################################*/

/**
 *
 */
class SVGPaint : public SVGColor
{
public:

    /**
     * Paint Types
     */
    typedef enum
        {
        SVG_PAINTTYPE_UNKNOWN               = 0,
        SVG_PAINTTYPE_RGBCOLOR              = 1,
        SVG_PAINTTYPE_RGBCOLOR_ICCCOLOR     = 2,
        SVG_PAINTTYPE_NONE                  = 101,
        SVG_PAINTTYPE_CURRENTCOLOR          = 102,
        SVG_PAINTTYPE_URI_NONE              = 103,
        SVG_PAINTTYPE_URI_CURRENTCOLOR      = 104,
        SVG_PAINTTYPE_URI_RGBCOLOR          = 105,
        SVG_PAINTTYPE_URI_RGBCOLOR_ICCCOLOR = 106,
        SVG_PAINTTYPE_URI                   = 107
        } PaintType;


    /**
     *
     */
    unsigned short getPaintType();

    /**
     *
     */
    DOMString getUri();

    /**
     *
     */
    void setUri(const DOMString& uriArg);

    /**
     *
     */
    void setPaint(unsigned short paintTypeArg,
                           const DOMString& uriArg,
                           const DOMString& /*rgbColor*/,
                           const DOMString& /*iccColor*/)
                           throw(SVGException);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPaint();

    /**
     *
     */
    SVGPaint(const SVGPaint &other);

    /**
     *
     */
    ~SVGPaint();

protected:

    unsigned int paintType;
    DOMString uri;

};



/*#########################################################################
## SVGPreserveAspectRatio
#########################################################################*/

/**
 *
 */
class SVGPreserveAspectRatio
{
public:


    /**
     * Alignment Types
     */
    typedef enum
        {
        SVG_PRESERVEASPECTRATIO_UNKNOWN  = 0,
        SVG_PRESERVEASPECTRATIO_NONE     = 1,
        SVG_PRESERVEASPECTRATIO_XMINYMIN = 2,
        SVG_PRESERVEASPECTRATIO_XMIDYMIN = 3,
        SVG_PRESERVEASPECTRATIO_XMAXYMIN = 4,
        SVG_PRESERVEASPECTRATIO_XMINYMID = 5,
        SVG_PRESERVEASPECTRATIO_XMIDYMID = 6,
        SVG_PRESERVEASPECTRATIO_XMAXYMID = 7,
        SVG_PRESERVEASPECTRATIO_XMINYMAX = 8,
        SVG_PRESERVEASPECTRATIO_XMIDYMAX = 9,
        SVG_PRESERVEASPECTRATIO_XMAXYMAX = 10
        } AlignmentType;


    /**
     * Meet-or-slice Types
     */
    typedef enum
        {
        SVG_MEETORSLICE_UNKNOWN  = 0,
        SVG_MEETORSLICE_MEET     = 1,
        SVG_MEETORSLICE_SLICE    = 2
        } MeetOrSliceType;


    /**
     *
     */
    unsigned short getAlign();

    /**
     *
     */
    void setAlign(unsigned short val) throw(DOMException);

    /**
     *
     */
    unsigned short getMeetOrSlice();

    /**
     *
     */
    void setMeetOrSlice(unsigned short val) throw(DOMException);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPreserveAspectRatio();

    /**
     *
     */
    SVGPreserveAspectRatio(const SVGPreserveAspectRatio &other);

    /**
     *
     */
    ~SVGPreserveAspectRatio();

protected:

    unsigned short align;
    unsigned short meetOrSlice;

};



/*#########################################################################
## SVGRect
#########################################################################*/

/**
 *
 */
class SVGRect
{
public:

    /**
     *
     */
    double getX();

    /**
     *
     */
    void setX(double val) throw(DOMException);

    /**
     *
     */
    double getY();

    /**
     *
     */
    void setY(double val) throw(DOMException);

    /**
     *
     */
    double getWidth();

    /**
     *
     */
    void setWidth(double val) throw(DOMException);

    /**
     *
     */
    double getHeight();

    /**
     *
     */
    void setHeight(double val) throw(DOMException);


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGRect();

    /**
     *
     */
    SVGRect(const SVGRect &other);

    /**
     *
     */
    ~SVGRect();

protected:

    double x, y, width, height;

};

/*#########################################################################
## SVGTransform
#########################################################################*/

/**
 *
 */
class SVGTransform
{
public:

    /**
     * Transform Types
     */
    typedef enum
        {
        SVG_TRANSFORM_UNKNOWN   = 0,
        SVG_TRANSFORM_MATRIX    = 1,
        SVG_TRANSFORM_TRANSLATE = 2,
        SVG_TRANSFORM_SCALE     = 3,
        SVG_TRANSFORM_ROTATE    = 4,
        SVG_TRANSFORM_SKEWX     = 5,
        SVG_TRANSFORM_SKEWY     = 6,
        } TransformType;

    /**
     *
     */
    unsigned short getType();


    /**
     *
     */
    SVGMatrix getMatrix();

    /**
     *
     */
    double getAngle();

    /**
     *
     */
    void setMatrix(const SVGMatrix &matrixArg);

    /**
     *
     */
    void setTranslate(double tx, double ty);

    /**
     *
     */
    void setScale(double sx, double sy);

    /**
     *
     */
    void setRotate(double angleArg, double cx, double cy);

    /**
     *
     */
    void setSkewX(double angleArg);

    /**
     *
     */
    void setSkewY(double angleArg);


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGTransform();

    /**
     *
     */
    SVGTransform(const SVGTransform &other);

    /**
     *
     */
    ~SVGTransform();

protected:

    int type;
    double angle;

    SVGMatrix matrix;
};




/*#########################################################################
## SVGUnitTypes
#########################################################################*/

/**
 *
 */
class SVGUnitTypes
{
public:

    /**
     * Unit Types
     */
    typedef enum
        {
        SVG_UNIT_TYPE_UNKNOWN           = 0,
        SVG_UNIT_TYPE_USERSPACEONUSE    = 1,
        SVG_UNIT_TYPE_OBJECTBOUNDINGBOX = 2
        } UnitType;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGUnitTypes();

    /**
     *
     */
    ~SVGUnitTypes();

};




/*#########################################################################
## SVGValue
#########################################################################*/

/**
 * This is a helper class that will hold several types of data.  It will
 * be used in those situations where methods are common to different 
 * interfaces, except for the data type.  This class holds the following:
 * SVGAngle
 * SVGBoolean
 * SVGEnumeration
 * SVGInteger
 * SVGLength
 * SVGNumber
 * SVGPreserveAspectRatio
 * SVGRect
 * SVGString
 */   
class SVGValue
{
public:

    /**
     *
     */
    typedef enum
        {
        SVG_ANGLE,
        SVG_BOOLEAN,
        SVG_ENUMERATION,
        SVG_INTEGER,
        SVG_LENGTH,
        SVG_NUMBER,
        SVG_PRESERVE_ASPECT_RATIO,
        SVG_RECT,
        SVG_STRING,
        } SVGValueType;

    /**
     * Constructor
     */
    SVGValue();
    
    /**
     * Copy constructor
     */
    SVGValue(const SVGValue &other);
    
    /**
     * Assignment
     */
    SVGValue &operator=(const SVGValue &other);
    
    /**
     *
     */
    ~SVGValue();
        
    //###########################
    //  TYPES
    //###########################

    /**
     *  Angle
     */
    SVGValue(const SVGAngle &v);
    
    SVGAngle angleValue();
    
    /**
     * Boolean
     */
    SVGValue(bool v);
    
    bool booleanValue();
    
    
    /**
     * Enumeration
     */
    SVGValue(short v);
    
    short enumerationValue();

    /**
     * Integer
     */
    SVGValue(long v);
    
    long integerValue();
    
    /**
     * Length
     */
    SVGValue(const SVGLength &v);
    
    SVGLength lengthValue();
    
    /**
     * Number
     */
    SVGValue(double v);
    
    double numberValue();

    /**
     * PathSegment
     */
    SVGValue(const SVGPathSeg &v);
    
    SVGPathSeg pathDataValue();
    
    
    /**
     * Points
     */
    SVGValue(const SVGPointList &v);
    
    SVGPointList pointListValue();
    
    
    /**
     * PreserveAspectRatio
     */
    SVGValue(const SVGPreserveAspectRatio &v);
    
    SVGPreserveAspectRatio preserveAspectRatioValue();
    
    /**
     * Rect
     */
    SVGValue(const SVGRect &v);
    
    SVGRect rectValue();
    
    /**
     * String
     */
    SVGValue(const DOMString &v);
    
    DOMString stringValue();
    
    /**
     * TransformList
     */
    SVGValue(const SVGTransformList &v);
    
    SVGTransformList transformListValue();
    
    
private:

    void init();
    
    void assign(const SVGValue &other);
    
    short                  type;
    SVGAngle               angleval;       // SVGAngle
    bool                   bval;           // SVGBoolean
    short                  eval;           // SVGEnumeration
    long                   ival;           // SVGInteger
    SVGLength              lengthval;      // SVGLength
    double                 dval;           // SVGNumber
    SVGPathSegment         segval;         // SVGPathSegment
    SVGPreserveAcpectRatio parval;         // SVGPreserveAspectRatio
    SVGRect                rval;           // SVGRect
    DOMString              sval;           // SVGString

};


/*#########################################################################
## SVGValueList
#########################################################################*/

/**
 * THis is used to generify a bit the several different types of lists:
 *
 * SVGLengthList      -> SVGValueList<SVGLength>
 * SVGNumberList      -> SVGValueList<SVGNumber>
 * SVGPathData        -> SVGValueList<SVGPathSeg>
 * SVGPoints          -> SVGValueList<SVGPoint>
 * SVGTransformList   -> SVGValueList<SVGTransform>
 */
class SVGValueList
{
public:

    /**
     *
     */
    typedef enum
        {
        SVG_LIST_LENGTH,
        SVG_LIST_NUMBER,
        SVG_LIST_PATHSEG,
        SVG_LIST_POINT,
        SVG_LIST_TRANSFORM
        } SVGValueListTypes;

    /**
     *
     */
    unsigned long getNumberOfItems();


    /**
     *
     */
    void clear() throw(DOMException);

    /**
     *
     */
    SVGValue getItem(unsigned long index) throw(DOMException);

    /**
     *
     */
    SVGValue insertItemBefore(const SVGValue &newItem,
                                   unsigned long index)
                                   throw(DOMException, SVGException);

    /**
     *
     */
    SVGValue replaceItem(const SVGValue &newItem,
                              unsigned long index)
                              throw(DOMException, SVGException);

    /**
     *
     */
    SVGValue removeItem(unsigned long index) throw(DOMException);

    /**
     *
     */
    SVGValue appendItem(const SVGValue &newItem)
                             throw(DOMException, SVGException);

    /**
     * Matrix
     */
    SVGValue initialize(const SVGValue &newItem)
                         throw(DOMException, SVGException);

    /**
     * Matrix
     */
    SVGValue createSVGTransformFromMatrix(const SVGValue &matrix);

    /**
     * Matrix
     */
    SVGValue consolidate();


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGValueList();

    /**
     *
     */
    SVGValueList(const SVGValueList &other);

    /**
     *
     */
    ~SVGValueList();

protected:

    std::vector<SVGValue> items;

};





/*#########################################################################
## SVGAnimatedValue
#########################################################################*/

/**
 * This class is used to merge all of the "Animated" values, with only
 * a different type, into a single API.  This class subsumes the following:
 * SVGAnimatedAngle
 * SVGAnimatedBoolean
 * SVGAnimatedEnumeration
 * SVGAnimatedInteger
 * SVGAnimatedLength
 * SVGAnimatedLengthList
 * SVGAnimatedNumber
 * SVGAnimatedNumberList
 * SVGAnimatedPathData
 * SVGAnimatedPoints
 * SVGAnimatedPreserveAspectRatio
 * SVGAnimatedRect
 * SVGAnimatedString
 * SVGAnimatedTransformList
 */
class SVGAnimatedValue
{
public:

    /**
     *
     */
    SVGValue &getBaseVal();

    /**
     *
     */
    void setBaseVal(const SVGValue &val) throw (DOMException);

    /**
     *
     */
    SVGValue &getAnimVal();

    /**
     *
     */
    SVGAnimatedValue();
    
    /**
     *
     */
    SVGAnimatedValue(const SVGValue &baseValue);

    /**
     *
     */
    SVGAnimatedValue(const SVGValue &baseValue, const SVGValue &animValue);

    /**
     *
     */
    SVGAnimatedValue(const SVGAnimatedValue &other);

    /**
     *
     */
    SVGAnimatedValue &operator=(const SVGAnimatedValue &other);

    /**
     *
     */
    SVGAnimatedValue &operator=(const SVGValue &baseVal);

    /**
     *
     */
    ~SVGAnimatedValue();
    
private:

    void init();
    
    void assign(const SVGAnimatedValue &other);
    
    SVGValue baseVal;
    
    SVGValue animVal;
}







//########################################################################
//########################################################################
//#   I N T E R F A C E S
//########################################################################
//########################################################################







/*#########################################################################
## SVGStylable
#########################################################################*/

/**
 *
 */
class SVGStylable
{
public:

    /**
     *
     */
    SVGAnimatedString getClassName();

    /**
     *
     */
    css::CSSStyleDeclaration getStyle();


    /**
     *
     */
    css::CSSValue getPresentationAttribute(const DOMString& /*name*/);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGStylable();

    /**
     *
     */
    SVGStylable(const SVGStylable &other);

    /**
     *
     */
    ~SVGStylable();

protected:

    SVGAnimatedString className;
    css::CSSStyleDeclaration style;

};





/*#########################################################################
## SVGLocatable
#########################################################################*/

/**
 *
 */
class SVGLocatable
{
public:

    /**
     *
     */
    SVGElementPtr getNearestViewportElement();

    /**
     *
     */
    SVGElementPtr getFarthestViewportElement();

    /**
     *
     */
    SVGRect getBBox();

    /**
     *
     */
    SVGMatrix getCTM();

    /**
     *
     */
    SVGMatrix getScreenCTM();

    /**
     *
     */
    SVGMatrix getTransformToElement(const SVGElement &/*element*/)
                    throw(SVGException);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGLocatable();

    /**
     *
     */
    SVGLocatable(const SVGLocatable &/*other*/);

    /**
     *
     */
    ~SVGLocatable();

protected:

    SVGRect bbox;
    SVGMatrix ctm;
    SVGMatrix screenCtm;

};


/*#########################################################################
## SVGTransformable
#########################################################################*/

/**
 *
 */
class SVGTransformable : public SVGLocatable
{
public:


    /**
     *
     */
    SVGAnimatedTransformList &getTransform();

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGTransformable();

    /**
     *
     */
    SVGTransformable(const SVGTransformable &other) : SVGLocatable(other);

    /**
     *
     */
    ~SVGTransformable();

protected:

    SVGAnimatedTransformList transforms;
};






/*#########################################################################
## SVGTests
#########################################################################*/

/**
 *
 */
class SVGTests
{
public:

    /**
     *
     */
    SVGValueList &getRequiredFeatures();

    /**
     *
     */
    SVGValueList &getRequiredExtensions();

    /**
     *
     */
    SVGValueList &getSystemLanguage();

    /**
     *
     */
    bool hasExtension(const DOMString& /*extension*/);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGTests();

    /**
     *
     */
    SVGTests(const SVGTests &other);

    /**
     *
     */
    ~SVGTests();

protected:

    SVGStringList requiredFeatures;
    SVGStringList requiredExtensions;
    SVGStringList systemLanguage;

};






/*#########################################################################
## SVGLangSpace
#########################################################################*/

/**
 *
 */
class SVGLangSpace
{
public:


    /**
     *
     */
    DOMString getXmlLang();

    /**
     *
     */
    void setXmlLang(const DOMString &val) throw(DOMException);

    /**
     *
     */
    DOMString getXmlSpace();

    /**
     *
     */
    void setXmlSpace(const DOMString &val) throw(DOMException);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGLangSpace();

    /**
     *
     */
    SVGLangSpace(const SVGLangSpace &other);

    /**
     *
     */
    ~SVGLangSpace();

protected:

    DOMString xmlLang;
    DOMString xmlSpace;

};



/*#########################################################################
## SVGExternalResourcesRequired
#########################################################################*/

/**
 *
 */
class SVGExternalResourcesRequired
{
public:

    /**
     *
     */
    SVGAnimatedBoolean getExternalResourcesRequired();

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGExternalResourcesRequired();

    /**
     *
     */
    SVGExternalResourcesRequired(const SVGExternalResourcesRequired &other);

    /**
     *
     */
    ~SVGExternalResourcesRequired();

protected:

    SVGAnimatedBoolean required;

};









/*#########################################################################
## SVGFitToViewBox
#########################################################################*/

/**
 *
 */
class SVGFitToViewBox
{
public:

    /**
     *
     */
    SVGAnimatedRect getViewBox();

    /**
     *
     */
    SVGAnimatedPreserveAspectRatio getPreserveAspectRatio();

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGFitToViewBox();

    /**
     *
     */
    SVGFitToViewBox(const SVGFitToViewBox &other);

    /**
     *
     */
    ~SVGFitToViewBox();

protected:

    SVGAnimatedRect viewBox;

    SVGAnimatedPreserveAspectRatio preserveAspectRatio;

};


/*#########################################################################
## SVGZoomAndPan
#########################################################################*/

/**
 *
 */
class SVGZoomAndPan
{
public:

    /**
     * Zoom and Pan Types
     */
    typedef enum
        {
        SVG_ZOOMANDPAN_UNKNOWN = 0,
        SVG_ZOOMANDPAN_DISABLE = 1,
        SVG_ZOOMANDPAN_MAGNIFY = 2
        } ZoomAndPanType;

    /**
     *
     */
    unsigned short getZoomAndPan();

    /**
     *
     */
    void setZoomAndPan(unsigned short val) throw(DOMException);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGZoomAndPan();

    /**
     *
     */
    SVGZoomAndPan(const SVGZoomAndPan &other);

    /**
     *
     */
    ~SVGZoomAndPan();

protected:

    unsigned short zoomAndPan;

};






/*#########################################################################
## SVGViewSpec
#########################################################################*/

/**
 *
 */
class SVGViewSpec : public SVGZoomAndPan,
                    public SVGFitToViewBox
{
public:

    /**
     *
     */
    SVGTransformList getTransform();

    /**
     *
     */
    SVGElementPtr getViewTarget();

    /**
     *
     */
    DOMString getViewBoxString();

    /**
     *
     */
    DOMString getPreserveAspectRatioString();

    /**
     *
     */
    DOMString getTransformString();

    /**
     *
     */
    DOMString getViewTargetString();

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGViewSpec();

    /**
     *
     */
    SVGViewSpec(const SVGViewSpec &other);

    /**
     *
     */
    ~SVGViewSpec();

protected:

    SVGElementPtr viewTarget;
    SVGTransformList transform;
};


/*#########################################################################
## SVGURIReference
#########################################################################*/

/**
 *
 */
class SVGURIReference
{
public:

    /**
     *
     */
    SVGAnimatedString getHref();

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGURIReference();

    /**
     *
     */
    SVGURIReference(const SVGURIReference &other);

    /**
     *
     */
    ~SVGURIReference();

protected:

    SVGAnimatedString href;

};






/*#########################################################################
## SVGCSSRule
#########################################################################*/

/**
 *
 */
class SVGCSSRule : public css::CSSRule
{
public:


    /**
     * Additional CSS RuleType to support ICC color specifications
     */
    typedef enum
        {
        COLOR_PROFILE_RULE = 7
        } ColorProfileRuleType;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGCSSRule();

    /**
     *
     */
    SVGCSSRule(const SVGCSSRule &other);

    /**
     *
     */
    ~SVGCSSRule();

};



/*#########################################################################
## SVGRenderingIntent
#########################################################################*/

/**
 *
 */
class SVGRenderingIntent
{
public:

    /**
     * Rendering Intent Types
     */
    typedef enum
        {
        RENDERING_INTENT_UNKNOWN               = 0,
        RENDERING_INTENT_AUTO                  = 1,
        RENDERING_INTENT_PERCEPTUAL            = 2,
        RENDERING_INTENT_RELATIVE_COLORIMETRIC = 3,
        RENDERING_INTENT_SATURATION            = 4,
        RENDERING_INTENT_ABSOLUTE_COLORIMETRIC = 5
        } RenderingIntentType;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGRenderingIntent();

    /**
     *
     */
    SVGRenderingIntent(const SVGRenderingIntent &other);

    /**
     *
     */
    ~SVGRenderingIntent();

protected:

    unsigned short renderingIntentType;
};









/*#########################################################################
## SVGColorProfileRule
#########################################################################*/

/**
 *
 */
class SVGColorProfileRule : public SVGCSSRule,
                            public SVGRenderingIntent
{

public:

   /**
     *
     */
    DOMString getSrc();

    /**
     *
     */
    void setSrc(const DOMString &val) throw(DOMException);

    /**
     *
     */
    DOMString getName();

    /**
     *
     */
    void setName(const DOMString &val) throw(DOMException);

    /**
     *
     */
    unsigned short getRenderingIntent();

    /**
     *
     */
    void setRenderingIntent(unsigned short val) throw(DOMException)


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGColorProfileRule();

    /**
     *
     */
    SVGColorProfileRule(const SVGColorProfileRule &other);

    /**
     *
     */
    ~SVGColorProfileRule();

protected:

    unsigned short renderingIntent;
    DOMString src;
    DOMString name;

};



/*#########################################################################
## SVGFilterPrimitiveStandardAttributes
#########################################################################*/

/**
 *
 */
class SVGFilterPrimitiveStandardAttributes : public SVGStylable
{
public:

    /**
     *
     */
    SVGAnimatedLength getX();

    /**
     *
     */
    SVGAnimatedLength getY();

    /**
     *
     */
    SVGAnimatedLength getWidth();

    /**
     *
     */
    SVGAnimatedLength getHeight();

    /**
     *
     */
    SVGAnimatedString getResult();

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGFilterPrimitiveStandardAttributes();

    /**
     *
     */
    SVGFilterPrimitiveStandardAttributes(
                const SVGFilterPrimitiveStandardAttributes &other);

    /**
     *
     */
    ~SVGFilterPrimitiveStandardAttributes();

protected:

    SVGAnimatedLength x;
    SVGAnimatedLength y;
    SVGAnimatedLength width;
    SVGAnimatedLength height;
    SVGAnimatedString result;

};


/*#########################################################################
## SVGEvent
#########################################################################*/

/**
 *
 */
class SVGEvent : events::Event
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGEvent();

    /**
     *
     */
    SVGEvent(const SVGEvent &other);

    /**
     *
     */
    ~SVGEvent();

};




/*#########################################################################
## SVGZoomEvent
#########################################################################*/

/**
 *
 */
class SVGZoomEvent : events::UIEvent
{
public:

    /**
     *
     */
    SVGRect getZoomRectScreen();

    /**
     *
     */
    double getPreviousScale();

    /**
     *
     */
    SVGPoint getPreviousTranslate();

    /**
     *
     */
    double getNewScale();

   /**
     *
     */
    SVGPoint getNewTranslate()


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGZoomEvent();

    /**
     *
     */
    SVGZoomEvent(const SVGZoomEvent &other);

    /**
     *
     */
    ~SVGZoomEvent();

protected:

    SVGRect  zoomRectScreen;
    double   previousScale;
    SVGPoint previousTranslate;
    double   newScale;
    SVGPoint newTranslate;

};



/*#########################################################################
## SVGElementInstance
#########################################################################*/

/**
 *
 */
class SVGElementInstance : public events::EventTarget
{
public:

    /**
     *
     */
    SVGElementPtr getCorrespondingElement();

    /**
     *
     */
    SVGUseElementPtr getCorrespondingUseElement();

    /**
     *
     */
    SVGElementInstance getParentNode();

    /**
     *  Since we are using stack types and this is a circular definition,
     *  we will instead implement this as a global function below:
     *   SVGElementInstanceList getChildNodes(const SVGElementInstance instance);
     */
    //SVGElementInstanceList getChildNodes();

    /**
     *
     */
    SVGElementInstance getFirstChild();

    /**
     *
     */
    SVGElementInstance getLastChild();

    /**
     *
     */
    SVGElementInstance getPreviousSibling();

    /**
     *
     */
    SVGElementInstance getNextSibling();


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGElementInstance();

    /**
     *
     */
    SVGElementInstance(const SVGElementInstance &other);

    /**
     *
     */
    ~SVGElementInstance();

protected:

    SVGElementPtr      correspondingElement;
    SVGUseElementPtr   correspondingUseElement;

};






/*#########################################################################
## SVGElementInstanceList
#########################################################################*/

/**
 *
 */
class SVGElementInstanceList
{
public:

    /**
     *
     */
    unsigned long getLength();

    /**
     *
     */
    SVGElementInstance item(unsigned long index);

    /**
     *  This static method replaces the circular definition of:
     *        SVGElementInstanceList SVGElementInstance::getChildNodes()
     *
     */
    static SVGElementInstanceList getChildNodes(const SVGElementInstance &/*instance*/);


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGElementInstanceList();

    /**
     *
     */
    SVGElementInstanceList(const SVGElementInstanceList &other);

    /**
     *
     */
    ~SVGElementInstanceList();

protected:

    std::vector<SVGElementInstance> items;


};








//########################################################################
//########################################################################
//########################################################################
//#   D O M
//########################################################################
//########################################################################
//########################################################################





/*#########################################################################
## Types
#########################################################################*/

/**
 * Bitmasks for has_an interface for SVGElement
 */ 
#define SVG_ANGLE                          0x00000001
#define SVG_ANIMATED_ANGLE                 0x00000002
#define SVG_ANIMATED_BOOLEAN               0x00000004
#define SVG_ANIMATED_ENUMERATION           0x00000008
#define SVG_ANIMATED_INTEGER               0x00000010
#define SVG_ANIMATED_LENGTH                0x00000020
#define SVG_ANIMATED_LENGTH_LIST           0x00000040
#define SVG_ANIMATED_NUMBER                0x00000080
#define SVG_ANIMATED_NUMBER_LIST           0x00000100
#define SVG_ANIMATED_RECT                  0x00000200
#define SVG_ANIMATED_STRING                0x00000400
#define SVG_COLOR                          0x00000800
#define SVG_CSS_RULE                       0x00001000
#define SVG_EXTERNAL_RESOURCES_REQUIRED    0x00002000
#define SVG_FIT_TO_VIEWBOX                 0x00004000
#define SVG_ICCCOLOR                       0x00008000
#define SVG_LANG_SPACE                     0x00010000
#define SVG_LENGTH                         0x00020000
#define SVG_LENGTH_LIST                    0x00040000
#define SVG_LOCATABLE                      0x00080000
#define SVG_NUMBER                         0x00100000
#define SVG_NUMBER_LIST                    0x00200000
#define SVG_RECT                           0x00400000
#define SVG_RENDERING_INTENT               0x00800000
#define SVG_STRING_LIST                    0x01000000
#define SVG_STYLABLE                       0x02000000
#define SVG_TESTS                          0x04000000
#define SVG_TRANSFORMABLE                  0x08000000
#define SVG_UNIT_TYPES                     0x10000000
#define SVG_URI_REFERENCE                  0x20000000
#define SVG_VIEW_SPEC                      0x40000000
#define SVG_ZOOM_AND_PAN                   0x80000000

/**
 * How many above?  Quite handy
 */ 
#define SVG_NR_INTERFACES                  32


/**
 * Enumerations for SVGElement types
 */ 
typedef enum
{
    SVG_A_ELEMENT = 0,
    SVG_ALTGLYPH_ELEMENT,
    SVG_ALTGLYPHDEF_ELEMENT,
    SVG_ALTGLYPHITEM_ELEMENT,
    SVG_ANIMATE_ELEMENT,
    SVG_ANIMATECOLOR_ELEMENT,
    SVG_ANIMATEMOTION_ELEMENT,
    SVG_ANIMATETRANSFORM_ELEMENT,
    SVG_CIRCLE_ELEMENT,
    SVG_CLIPPATH_ELEMENT,
    SVG_COLOR_PROFILE_ELEMENT,
    SVG_CURSOR_ELEMENT,
    SVG_DEFINITION_SRC_ELEMENT,
    SVG_DEFS_ELEMENT,
    SVG_DESC_ELEMENT,
    SVG_ELLIPSE_ELEMENT,
    SVG_FEBLEND_ELEMENT,
    SVG_FECOLORMATRIX_ELEMENT,
    SVG_FECOMPONENTTRANSFER_ELEMENT,
    SVG_FECOMPOSITE_ELEMENT,
    SVG_FECONVOLVEMATRIX_ELEMENT,
    SVG_FEDIFFUSELIGHTING_ELEMENT,
    SVG_FEDISPLACEMENTMAP_ELEMENT,
    SVG_FEDISTANTLIGHT_ELEMENT,
    SVG_FEFLOOD_ELEMENT,
    SVG_FEFUNCA_ELEMENT,
    SVG_FEFUNCB_ELEMENT,
    SVG_FEFUNCG_ELEMENT,
    SVG_FEFUNCR_ELEMENT,
    SVG_FEGAUSSIANBLUR_ELEMENT,
    SVG_FEIMAGE_ELEMENT,
    SVG_FEMERGE_ELEMENT,
    SVG_FEMERGENODE_ELEMENT,
    SVG_FEMORPHOLOGY_ELEMENT,
    SVG_FEOFFSET_ELEMENT,
    SVG_FEPOINTLIGHT_ELEMENT,
    SVG_FESPECULARLIGHTING_ELEMENT,
    SVG_FESPOTLIGHT_ELEMENT,
    SVG_FETILE_ELEMENT,
    SVG_FETURBULENCE_ELEMENT,
    SVG_FILTER_ELEMENT,
    SVG_FONT_ELEMENT,
    SVG_FONT_FACE_ELEMENT,
    SVG_FONT_FACE_FORMAT_ELEMENT,
    SVG_FONT_FACE_NAME_ELEMENT,
    SVG_FONT_FACE_SRC_ELEMENT,
    SVG_FONT_FACE_URI_ELEMENT,
    SVG_FOREIGNOBJECT_ELEMENT,
    SVG_G_ELEMENT,
    SVG_GLYPH_ELEMENT,
    SVG_GLYPHREF_ELEMENT,
    SVG_HKERN_ELEMENT,
    SVG_IMAGE_ELEMENT,
    SVG_LINE_ELEMENT,
    SVG_LINEARGRADIENT_ELEMENT,
    SVG_MARKER_ELEMENT,
    SVG_MASK_ELEMENT,
    SVG_METADATA_ELEMENT,
    SVG_MISSING_GLYPH_ELEMENT,
    SVG_MPATH_ELEMENT,
    SVG_PATH_ELEMENT,
    SVG_PATTERN_ELEMENT,
    SVG_POLYGON_ELEMENT,
    SVG_POLYLINE_ELEMENT,
    SVG_RADIALGRADIENT_ELEMENT,
    SVG_RECT_ELEMENT,
    SVG_SCRIPT_ELEMENT,
    SVG_SET_ELEMENT,
    SVG_STOP_ELEMENT,
    SVG_STYLE_ELEMENT,
    SVG_SVG_ELEMENT,
    SVG_SWITCH_ELEMENT,
    SVG_SYMBOL_ELEMENT,
    SVG_TEXT_ELEMENT,
    SVG_TEXTPATH_ELEMENT,
    SVG_TITLE_ELEMENT,
    SVG_TREF_ELEMENT,
    SVG_TSPAN_ELEMENT,
    SVG_USE_ELEMENT,
    SVG_VIEW_ELEMENT,
    SVG_VKERN_ELEMENT,
    SVG_MAX_ELEMENT
} SVGElementType;




/**
 * Look up the SVG Element type enum for a given string
 * Return -1 if not found
 */
int svgElementStrToEnum(const char *str);


/**
 * Return the string corresponding to a given SVG element type enum
 * Return "unknown" if not found
 */
const char *svgElementEnumToStr(int type);




/*#########################################################################
## SVGElement
#########################################################################*/

/**
 * All of the SVG DOM interfaces that correspond directly to elements in the SVG
 * language(e.g., the SVGPathElement interface corresponds directly to the
 * 'path' element in the language) are derivative from base class SVGElement.
 */
class SVGElement : public Element
{
public:

    //####################################################################
    //# BASE METHODS FOR SVGElement
    //####################################################################

    /**
     * Get the value of the id attribute on the given element.
     */
    DOMString getId();

    /**
     * Set the value of the id attribute on the given element.
     */
    void setId(const DOMString &val) throw(DOMException);

    /**
     * Corresponds to attribute xml:base on the given element.
     */
    DOMString getXmlBase();

    /**
     * Corresponds to attribute xml:base on the given element.
     */
    void setXmlBase(const DOMString &val) throw(DOMException);

    /**
     * The nearest ancestor 'svg' element. Null if the given element is the
     *      outermost 'svg' element.
     */
    SVGElementPtr getOwnerSVGElement();

    /**
     * The element which established the current viewport. Often, the nearest
     * ancestor 'svg' element. Null if the given element is the outermost 'svg'
     * element.
     */
    SVGElementPtr getViewportElement();


    //####################################################################
    //####################################################################
    //# I N T E R F A C E S
    //####################################################################
    //####################################################################

    //####################################################################
    //# SVGAngle                    
    //####################################################################

    /**
     *
     */
    unsigned short getUnitType();

    /**
     *
     */
    double getValue();

    /**
     *
     */
    void setValue(double val) throw(DOMException);

    /**
     *
     */
    double getValueInSpecifiedUnits();

    /**
     *
     */
    void setValueInSpecifiedUnits(double /*val*/) throw(DOMException);

    /**
     *
     */
    DOMString getValueAsString();

    /**
     *
     */
    void setValueAsString(const DOMString &/*val*/) throw(DOMException);


    /**
     *
     */
    void newValueSpecifiedUnits(unsigned short /*unitType*/,
                                double /*valueInSpecifiedUnits*/);

    /**
     *
     */
    void convertToSpecifiedUnits(unsigned short /*unitType*/);

    //####################################################################
    //## The following animated types are rolled up into a single
    //## SVGAnimatedValue interface
    //####################################################################

    //####################################################################
    //## SVGAnimatedAngle
    //####################################################################

    //####################################################################
    //## SVGAnimatedBoolean
    //####################################################################

    //####################################################################
    //## SVGAnimatedEnumeration
    //####################################################################

    //####################################################################
    //## SVGAnimatedInteger
    //####################################################################

    //####################################################################
    //## SVGAnimatedLength
    //####################################################################

    //####################################################################
    //## SVGAnimatedLengthList
    //####################################################################

    //####################################################################
    //## SVGAnimatedNumber
    //####################################################################

    //####################################################################
    //## SVGAnimatedNumberList
    //####################################################################

    //####################################################################
    //## SVGAnimatedPathData
    //####################################################################

    //####################################################################
    //## SVGAnimatedPoints
    //####################################################################

    //####################################################################
    //## SVGAnimatedPreserveAspectRatio
    //####################################################################

    //####################################################################
    //## SVGAnimatedRect
    //####################################################################

    //####################################################################
    //## SVGAnimatedString
    //####################################################################

    //####################################################################
    //## SVGAnimatedTransformList
    //####################################################################

    //####################################################################
    //# SVGAnimatedValue          
    //####################################################################

    /**
     *
     */
    SVGValue &getBaseVal();

    /**
     *
     */
    void setBaseVal(const SVGValue &val) throw (DOMException);

    /**
     *
     */
    SVGValue &getAnimVal();


    //####################################################################
    //# SVGColor                    
    //####################################################################

    /**
     * From CSSValue 
     * A code defining the type of the value as defined above.
     */
    unsigned short getCssValueType();

    /**
     * From CSSValue 
     * A string representation of the current value.
     */
    DOMString getCssText();

    /**
     * From CSSValue 
     * A string representation of the current value.
     * Note that setting implies parsing.     
     */
    void setCssText(const DOMString &val) throw(dom::DOMException);


    /**
     *
     */
    unsigned short getColorType();

    /**
     *
     */
    css::RGBColor getRgbColor();

    /**
     *
     */
    SVGICCColor getIccColor();


    /**
     *
     */
    void setRGBColor(const DOMString& /*rgbColor*/) throw(SVGException);

    /**
     *
     */
    void setRGBColorICCColor(const DOMString& /*rgbColor*/,
                             const DOMString& /*iccColor*/)
                             throw(SVGException);

    /**
     *
     */
    void setColor(unsigned short /*colorType*/,
                           const DOMString& /*rgbColor*/,
                           const DOMString& /*iccColor*/)
                           throw(SVGException);

    //####################################################################
    //# SVGCSSRule                  
    //####################################################################

    /**
     * From CSSRule    
     * The type of the rule, as defined above. The expectation is that 
     * binding-specific casting methods can be used to cast down from an instance of 
     * the CSSRule interface to the specific derived interface implied by the type.
     */
    unsigned short getType();

    /**
     * From CSSRule    
     * The parsable textual representation of the rule. This reflects the current 
     * state of the rule and not its initial value.
     */
    DOMString getCssText();

    /**
     * From CSSRule    
     * The parsable textual representation of the rule. This reflects the current 
     * state of the rule and not its initial value.
     * Note that setting involves reparsing.     
     */
    void setCssText(const DOMString &val) throw(DOMException);

    /**
     * From CSSRule    
     * The style sheet that contains this rule.
     */
    css::CSSStyleSheet *getParentStyleSheet();

    /**
     * From CSSRule    
     * If this rule is contained inside another rule(e.g. a style rule inside an 
     * @media block), this is the containing rule. If this rule is not nested inside 
     * any other rules, this returns null.
     */
    css::CSSRule *getParentRule();

    //####################################################################
    //# SVGExternalResourcesRequired
    //####################################################################

    /**
     *
     */
    SVGAnimatedBoolean getExternalResourcesRequired();

    //####################################################################
    //# SVGFitToViewBox             
    //####################################################################

    /**
     *
     */
    SVGAnimatedRect getViewBox();

    /**
     *
     */
    SVGAnimatedPreserveAspectRatio getPreserveAspectRatio();

    //####################################################################
    //# SVGICCColor                 
    //####################################################################

    /**
     *
     */
    DOMString getColorProfile();

    /**
     *
     */
    void setColorProfile(const DOMString &val) throw(DOMException);

    /**
     *
     */
    SVGNumberList &getColors();

    //####################################################################
    //# SVGLangSpace                
    //####################################################################

    /**
     *
     */
    DOMString getXmllang();

    /**
     *
     */
    void setXmllang(const DOMString &val) throw(DOMException);

    /**
     *
     */
    DOMString getXmlspace();

    /**
     *
     */
    void setXmlspace(const DOMString &val) throw(DOMException);

    //####################################################################
    //# SVGLength                   
    //####################################################################

    /**
     *
     */
    unsigned short getUnitType();

    /**
     *
     */
    double getValue();

    /**
     *
     */
    void setValue(double val) throw(DOMException);

    /**
     *
     */
    double getValueInSpecifiedUnits();

    /**
     *
     */
    void setValueInSpecifiedUnits(double /*val*/) throw(DOMException);

    /**
     *
     */
    DOMString getValueAsString();

    /**
     *
     */
    void setValueAsString(const DOMString& /*val*/) throw(DOMException);


    /**
     *
     */
    void newValueSpecifiedUnits(unsigned short /*unitType*/, double /*val*/);

    /**
     *
     */
    void convertToSpecifiedUnits(unsigned short /*unitType*/);

    //####################################################################
    //# SVGLengthList               
    //####################################################################

    /**
     *
     */
    unsigned long getNumberOfItems();


    /**
     *
     */
    void clear() throw(DOMException);

    /**
     *
     */
    SVGLength initialize(const SVGLength &newItem)
                    throw(DOMException, SVGException);

    /**
     *
     */
    SVGLength getItem(unsigned long index) throw(DOMException);

    /**
     *
     */
    SVGLength insertItemBefore(const SVGLength &newItem, unsigned long index)
                                   throw(DOMException, SVGException);

    /**
     *
     */
    SVGLength replaceItem(const SVGLength &newItem, unsigned long index)
                               throw(DOMException, SVGException);

    /**
     *
     */
    SVGLength removeItem(unsigned long index) throw(DOMException);

    /**
     *
     */
    SVGLength appendItem(const SVGLength &newItem)
                    throw(DOMException, SVGException);

    //####################################################################
    //# SVGLocatable                
    //####################################################################

    /**
     *
     */
    SVGElementPtr getNearestViewportElement();

    /**
     *
     */
    SVGElement *getFarthestViewportElement();

    /**
     *
     */
    SVGRect getBBox();

    /**
     *
     */
    SVGMatrix getCTM();

    /**
     *
     */
    SVGMatrix getScreenCTM();

    /**
     *
     */
    SVGMatrix getTransformToElement(const SVGElement &/*element*/)
                                    throw(SVGException);

    //####################################################################
    //# SVGNumber                   
    //####################################################################

    /**
     *
     */
    double getValue();

    /**
     *
     */
    void setValue(double val) throw(DOMException);

    //####################################################################
    //# SVGNumberList               
    //####################################################################

    /**
     *
     */
    unsigned long getNumberOfItems();


    /**
     *
     */
    void clear() throw(DOMException);

    /**
     *
     */
    SVGNumber initialize(const SVGNumber &newItem)
                    throw(DOMException, SVGException);

    /**
     *
     */
    SVGNumber getItem(unsigned long index) throw(DOMException);

    /**
     *
     */
    SVGNumber insertItemBefore(const SVGNumber &newItem, unsigned long index)
                                         throw(DOMException, SVGException);

    /**
     *
     */
    SVGNumber replaceItem(const SVGNumber &newItem, unsigned long index)
                          throw(DOMException, SVGException);

    /**
     *
     */
    SVGNumber removeItem(unsigned long index) throw(DOMException);

    /**
     *
     */
    SVGNumber appendItem(const SVGNumber &newItem)
                                   throw(DOMException, SVGException);

    //####################################################################
    //# SVGRect                     
    //####################################################################

    /**
     *
     */
    double getX();

    /**
     *
     */
    void setX(double val) throw(DOMException);

    /**
     *
     */
    double getY();

    /**
     *
     */
    void setY(double val) throw(DOMException);

    /**
     *
     */
    double getWidth();

    /**
     *
     */
    void setWidth(double val) throw(DOMException);

    /**
     *
     */
    double getHeight();

    /**
     *
     */
    void setHeight(double val) throw(DOMException);

    //####################################################################
    //# SVGRenderingIntent          
    //####################################################################

    //####################################################################
    //# SVGStringList               
    //####################################################################

    /**
     *
     */
    unsigned long getNumberOfItems();

    /**
     *
     */
    void clear() throw(DOMException);

    /**
     *
     */
    DOMString initialize(const DOMString& newItem)
                    throw(DOMException, SVGException);

    /**
     *
     */
    DOMString getItem(unsigned long index) throw(DOMException);

    /**
     *
     */
    DOMString insertItemBefore(const DOMString& newItem, unsigned long index)
                               throw(DOMException, SVGException);

    /**
     *
     */
    DOMString replaceItem(const DOMString& newItem, unsigned long index)
                                throw(DOMException, SVGException);

    /**
     *
     */
    DOMString removeItem(unsigned long index) throw(DOMException);

    /**
     *
     */
    DOMString appendItem(const DOMString& newItem)
                    throw(DOMException, SVGException);

    //####################################################################
    //# SVGStylable                 
    //####################################################################

    /**
     *
     */
    SVGAnimatedString getClassName();

    /**
     *
     */
    css::CSSStyleDeclaration getStyle();

    /**
     *
     */
    css::CSSValue getPresentationAttribute(const DOMString& /*name*/);

    //####################################################################
    //# SVGTests                    
    //####################################################################

    /**
     *
     */
    SVGStringList &getRequiredFeatures();

    /**
     *
     */
    SVGStringList &getRequiredExtensions();

    /**
     *
     */
    SVGStringList &getSystemLanguage();

    /**
     *
     */
    bool hasExtension(const DOMString& /*extension*/);

    //####################################################################
    //# SVGTransformable            
    //####################################################################

    /**
     *
     */
    SVGAnimatedTransformList &getTransform();

    //####################################################################
    //# SVGUnitTypes                
    //####################################################################

    //####################################################################
    //# SVGURIReference             
    //####################################################################

    /**
     *
     */
    SVGAnimatedString getHref();

    //####################################################################
    //# SVGViewSpec                 
    //####################################################################

    /**
     *
     */
    SVGTransformList getTransform();

    /**
     *
     */
    SVGElement *getViewTarget();

    /**
     *
     */
    DOMString getViewBoxString();

    /**
     *
     */
    DOMString getPreserveAspectRatioString();

    /**
     *
     */
    DOMString getTransformString();

    /**
     *
     */
    DOMString getViewTargetString();

    //####################################################################
    //# SVGZoomAndPan               
    //####################################################################

    /**
     *
     */
    unsigned short getZoomAndPan();

    /**
     *
     */
    void setZoomAndPan(unsigned short val) throw(DOMException);

    //####################################################################
    //####################################################################
    //# E L E M E N T S
    //####################################################################
    //####################################################################

    //####################################################################
    //# SVGAElement
    //####################################################################


    /**
     *
     */
    SVGAnimatedString getTarget();



    //####################################################################
    //# SVGAltGlyphElement
    //####################################################################


    /**
     * Get the attribute glyphRef on the given element.
     */
    DOMString getGlyphRef();

    /**
     * Set the attribute glyphRef on the given element.
     */
    void setGlyphRef(const DOMString &val) throw(DOMException);

    /**
     * Get the attribute format on the given element.
     */
    DOMString getFormat();

    /**
     * Set the attribute format on the given element.
     */
    void setFormat(const DOMString &val) throw(DOMException);


    //####################################################################
    //# SVGAltGlyphDefElement
    //####################################################################

    //####################################################################
    //# SVGAltGlyphItemElement
    //####################################################################


    //####################################################################
    //# SVGAnimateElement
    //####################################################################


    //####################################################################
    //# SVGAnimateColorElement
    //####################################################################

    //####################################################################
    //# SVGAnimateMotionElement
    //####################################################################


    //####################################################################
    //# SVGAnimateTransformElement
    //####################################################################


    //####################################################################
    //# SVGAnimationElement
    //####################################################################


    /**
     *
     */
    SVGElementPtr getTargetElement();

    /**
     *
     */
    double getStartTime();

    /**
     *
     */
    double getCurrentTime();

    /**
     *
     */
    double getSimpleDuration() throw(DOMException);



    //####################################################################
    //# SVGCircleElement
    //####################################################################

    /**
     * Corresponds to attribute cx on the given 'circle' element.
     */
    SVGAnimatedLength getCx();

    /**
     * Corresponds to attribute cy on the given 'circle' element.
     */
    SVGAnimatedLength getCy();

    /**
     * Corresponds to attribute r on the given 'circle' element.
     */
    SVGAnimatedLength getR();

    //####################################################################
    //# SVGClipPathElement
    //####################################################################


    /**
     * Corresponds to attribute clipPathUnits on the given 'clipPath' element.
     *      Takes one of the constants defined in SVGUnitTypes.
     */
    SVGAnimatedEnumeration getClipPathUnits();



    //####################################################################
    //# SVGColorProfileElement
    //####################################################################


    /**
     * Get the attribute local on the given element.
     */
    DOMString getLocal();

    /**
     * Set the attribute local on the given element.
     */
    void setLocal(const DOMString &val) throw(DOMException);

    /**
     * Get the attribute name on the given element.
     */
    DOMString getName();

    /**
     * Set the attribute name on the given element.
     */
    void setName(const DOMString &val) throw(DOMException);

    /**
     * Set the attribute rendering-intent on the given element.
     * The type of rendering intent, identified by one of the
     *      SVGRenderingIntent constants.
     */
    unsigned short getRenderingIntent();

    /**
     * Get the attribute rendering-intent on the given element.
     */
    void setRenderingIntent(unsigned short val) throw(DOMException);


    //####################################################################
    //# SVGComponentTransferFunctionElement
    //####################################################################


    /**
     * Component Transfer Types
     */
    typedef enum
        {
        SVG_FECOMPONENTTRANSFER_TYPE_UNKNOWN  = 0,
        SVG_FECOMPONENTTRANSFER_TYPE_IDENTITY = 1,
        SVG_FECOMPONENTTRANSFER_TYPE_TABLE    = 2,
        SVG_FECOMPONENTTRANSFER_TYPE_DISCRETE = 3,
        SVG_FECOMPONENTTRANSFER_TYPE_LINEAR   = 4,
        SVG_FECOMPONENTTRANSFER_TYPE_GAMMA    = 5
        } ComponentTransferType;


    /**
     * Corresponds to attribute type on the given element. Takes one\
     *      of the Component Transfer Types.
     */
    SVGAnimatedEnumeration getType();

    /**
     * Corresponds to attribute tableValues on the given element.
     */
    SVGAnimatedNumberList getTableValues();

    /**
     * Corresponds to attribute slope on the given element.
     */
    SVGAnimatedNumber getSlope();

    /**
     * Corresponds to attribute intercept on the given element.
     */
    SVGAnimatedNumber getIntercept();

    /**
     * Corresponds to attribute amplitude on the given element.
     */
    SVGAnimatedNumber getAmplitude();

    /**
     * Corresponds to attribute exponent on the given element.
     */
    SVGAnimatedNumber getExponent();

    /**
     * Corresponds to attribute offset on the given element.
     */
    SVGAnimatedNumber getOffset();

    //####################################################################
    //# SVGCursorElement
    //####################################################################

    /**
     *
     */
    SVGAnimatedLength getX();

    /**
     *
     */
    SVGAnimatedLength getY();


    //####################################################################
    //# SVGDefinitionSrcElement
    //####################################################################

    //####################################################################
    //# SVGDefsElement
    //####################################################################

    //####################################################################
    //# SVGDescElement
    //####################################################################

    //####################################################################
    //# SVGEllipseElement
    //####################################################################

    /**
     * Corresponds to attribute cx on the given 'ellipse' element.
     */
    SVGAnimatedLength getCx();

    /**
     * Corresponds to attribute cy on the given 'ellipse' element.
     */
    SVGAnimatedLength getCy();

    /**
     * Corresponds to attribute rx on the given 'ellipse' element.
     */
    SVGAnimatedLength getRx();

    /**
     * Corresponds to attribute ry on the given 'ellipse' element.
     */
    SVGAnimatedLength getRy();


    //####################################################################
    //# SVGFEBlendElement
    //####################################################################

    /**
     * Blend Mode Types
     */
    typedef enum
        {
        SVG_FEBLEND_MODE_UNKNOWN  = 0,
        SVG_FEBLEND_MODE_NORMAL   = 1,
        SVG_FEBLEND_MODE_MULTIPLY = 2,
        SVG_FEBLEND_MODE_SCREEN   = 3,
        SVG_FEBLEND_MODE_DARKEN   = 4,
        SVG_FEBLEND_MODE_LIGHTEN  = 5
        } BlendModeType;

    /**
     * Corresponds to attribute in on the given 'feBlend' element.
     */
    SVGAnimatedString getIn1();

    /**
     * Corresponds to attribute in2 on the given 'feBlend' element.
     */
    SVGAnimatedString getIn2();

    /**
     * Corresponds to attribute mode on the given 'feBlend' element.
     *      Takes one of the Blend Mode Types.
     */
    SVGAnimatedEnumeration getMode();


    //####################################################################
    //# SVGFEColorMatrixElement
    //####################################################################

    /**
     * Color Matrix Types
     */
    typedef enum
        {
        SVG_FECOLORMATRIX_TYPE_UNKNOWN          = 0,
        SVG_FECOLORMATRIX_TYPE_MATRIX           = 1,
        SVG_FECOLORMATRIX_TYPE_SATURATE         = 2,
        SVG_FECOLORMATRIX_TYPE_HUEROTATE        = 3,
        SVG_FECOLORMATRIX_TYPE_LUMINANCETOALPHA = 4
        } ColorMatrixType;


    /**
     * Corresponds to attribute in on the given 'feColorMatrix' element.
     */
    SVGAnimatedString getIn1();

    /**
     * Corresponds to attribute type on the given 'feColorMatrix' element.
     *      Takes one of the Color Matrix Types.
     */
    SVGAnimatedEnumeration getType();

    /**
     * Corresponds to attribute values on the given 'feColorMatrix' element.
     * Provides access to the contents of the values attribute.
     */
    SVGAnimatedNumberList getValues();


    //####################################################################
    //# SVGFEComponentTransferElement
    //####################################################################


    /**
     * Corresponds to attribute in on the given 'feComponentTransfer'  element.
     */
    SVGAnimatedString getIn1();

    //####################################################################
    //# SVGFECompositeElement
    //####################################################################

    /**
     *  Composite Operators
     */
    typedef enum
        {
        SVG_FECOMPOSITE_OPERATOR_UNKNOWN    = 0,
        SVG_FECOMPOSITE_OPERATOR_OVER       = 1,
        SVG_FECOMPOSITE_OPERATOR_IN         = 2,
        SVG_FECOMPOSITE_OPERATOR_OUT        = 3,
        SVG_FECOMPOSITE_OPERATOR_ATOP       = 4,
        SVG_FECOMPOSITE_OPERATOR_XOR        = 5,
        SVG_FECOMPOSITE_OPERATOR_ARITHMETIC = 6
        } CompositeOperatorType;

    /**
     * Corresponds to attribute in on the given 'feComposite' element.
     */
    SVGAnimatedString getIn1();

    /**
     * Corresponds to attribute in2 on the given 'feComposite' element.
     */
    SVGAnimatedString getIn2();

    /**
     * Corresponds to attribute operator on the given 'feComposite' element.
     *      Takes one of the Composite Operators.
     */
    SVGAnimatedEnumeration getOperator();

    /**
     * Corresponds to attribute k1 on the given 'feComposite' element.
     */
    SVGAnimatedNumber getK1();

    /**
     * Corresponds to attribute k2 on the given 'feComposite' element.
     */
    SVGAnimatedNumber getK2();

    /**
     * Corresponds to attribute k3 on the given 'feComposite' element.
     */
    SVGAnimatedNumber getK3();

    /**
     * Corresponds to attribute k4 on the given 'feComposite' element.
     */
    SVGAnimatedNumber getK4();


    //####################################################################
    //# SVGFEConvolveMatrixElement
    //####################################################################


    /**
     * Edge Mode Values
     */
    typedef enum
        {
        SVG_EDGEMODE_UNKNOWN   = 0,
        SVG_EDGEMODE_DUPLICATE = 1,
        SVG_EDGEMODE_WRAP      = 2,
        SVG_EDGEMODE_NONE      = 3
        } EdgeModeType;


    /**
     * Corresponds to attribute order on the given 'feConvolveMatrix'  element.
     */
    SVGAnimatedInteger getOrderX();

    /**
     * Corresponds to attribute order on the given 'feConvolveMatrix'  element.
     */
    SVGAnimatedInteger getOrderY();

    /**
     * Corresponds to attribute kernelMatrix on the given element.
     */
    SVGAnimatedNumberList getKernelMatrix();

    /**
     * Corresponds to attribute divisor on the given 'feConvolveMatrix' element.
     */
    SVGAnimatedNumber getDivisor();

    /**
     * Corresponds to attribute bias on the given 'feConvolveMatrix'  element.
     */
    SVGAnimatedNumber getBias();

    /**
     * Corresponds to attribute targetX on the given 'feConvolveMatrix'  element.
     */
    SVGAnimatedInteger getTargetX();

    /**
     * Corresponds to attribute targetY on the given 'feConvolveMatrix'  element.
     */
    SVGAnimatedInteger getTargetY();

    /**
     * Corresponds to attribute edgeMode on the given 'feConvolveMatrix'
     *      element. Takes one of the Edge Mode Types.
     */
    SVGAnimatedEnumeration getEdgeMode();

    /**
     * Corresponds to attribute kernelUnitLength on the
     *      given 'feConvolveMatrix'  element.
     */
    SVGAnimatedLength getKernelUnitLengthX();

    /**
     * Corresponds to attribute kernelUnitLength on the given
     *      'feConvolveMatrix'  element.
     */
    SVGAnimatedLength getKernelUnitLengthY();

    /**
     * Corresponds to attribute preserveAlpha on the
     *      given 'feConvolveMatrix'  element.
     */
    SVGAnimatedBoolean getPreserveAlpha();



    //####################################################################
    //# SVGFEDiffuseLightingElement
    //####################################################################


    /**
     * Corresponds to attribute in on the given 'feDiffuseLighting'  element.
     */
    SVGAnimatedString getIn1();

    /**
     * Corresponds to attribute surfaceScale on the given
     *      'feDiffuseLighting'  element.
     */
    SVGAnimatedNumber getSurfaceScale();

    /**
     * Corresponds to attribute diffuseConstant on the given
     *      'feDiffuseLighting'  element.
     */
    SVGAnimatedNumber getDiffuseConstant();

    /**
     * Corresponds to attribute kernelUnitLength on the given
     *      'feDiffuseLighting'  element.
     */
    SVGAnimatedNumber getKernelUnitLengthX();

    /**
     * Corresponds to attribute kernelUnitLength on the given
     *      'feDiffuseLighting'  element.
     */
    SVGAnimatedNumber getKernelUnitLengthY();




    //####################################################################
    //# SVGFEDisplacementMapElement
    //####################################################################


    /**
     *  Channel Selectors
     */
    typedef enum
        {
        SVG_CHANNEL_UNKNOWN = 0,
        SVG_CHANNEL_R       = 1,
        SVG_CHANNEL_G       = 2,
        SVG_CHANNEL_B       = 3,
        SVG_CHANNEL_A       = 4
        } ChannelSelector;

    /**
     *
     */
    SVGAnimatedString getIn1();

    /**
     *
     */
    SVGAnimatedString getIn2();


    /**
     *
     */
    SVGAnimatedNumber getScale();

    /**
     *
     */
    SVGAnimatedEnumeration getXChannelSelector();

    /**
     *
     */
    SVGAnimatedEnumeration getYChannelSelector();

    //####################################################################
    //# SVGFEDistantLightElement
    //####################################################################


    /**
     * Corresponds to attribute azimuth on the given 'feDistantLight'  element.
     */
    SVGAnimatedNumber getAzimuth();


    /**
     * Corresponds to attribute elevation on the given 'feDistantLight'
     *    element
     */
    SVGAnimatedNumber getElevation();


    //####################################################################
    //# SVGFEFloodElement
    //####################################################################


    /**
     *
     */
    SVGAnimatedString getIn1();


    //####################################################################
    //# SVGFEFuncAElement
    //####################################################################

    //####################################################################
    //# SVGFEFuncBElement
    //####################################################################

    //####################################################################
    //# SVGFEFuncGElement
    //####################################################################

    //####################################################################
    //# SVGFEFuncRElement
    //####################################################################


    //####################################################################
    //# SVGFEGaussianBlurElement
    //####################################################################


    /**
     *
     */
    SVGAnimatedString getIn1();


    /**
     *
     */
    SVGAnimatedNumber getStdDeviationX();

    /**
     *
     */
    SVGAnimatedNumber getStdDeviationY();


    /**
     *
     */
    void setStdDeviation(double stdDeviationX, double stdDeviationY);


    //####################################################################
    //# SVGFEImageElement
    //####################################################################


    //####################################################################
    //# SVGFEMergeElement
    //####################################################################

    //####################################################################
    //# SVGFEMergeNodeElement
    //####################################################################

    //####################################################################
    //# SVGFEMorphologyElement
    //####################################################################



    /**
     *  Morphology Operators
     */
    typedef enum
        {
        SVG_MORPHOLOGY_OPERATOR_UNKNOWN = 0,
        SVG_MORPHOLOGY_OPERATOR_ERODE   = 1,
        SVG_MORPHOLOGY_OPERATOR_DILATE  = 2
        } MorphologyOperatorType;


    /**
     *
     */
    SVGAnimatedString getIn1();


    /**
     *
     */
    SVGAnimatedEnumeration getOperator();

    /**
     *
     */
    SVGAnimatedLength getRadiusX();

    /**
     *
     */
    SVGAnimatedLength getRadiusY();

    //####################################################################
    //# SVGFEOffsetElement
    //####################################################################

    /**
     *
     */
    SVGAnimatedString getIn1();

    /**
     *
     */
    SVGAnimatedLength getDx();

    /**
     *
     */
    SVGAnimatedLength getDy();


    //####################################################################
    //# SVGFEPointLightElement
    //####################################################################

    /**
     * Corresponds to attribute x on the given 'fePointLight' element.
     */
    SVGAnimatedNumber getX();

    /**
     * Corresponds to attribute y on the given 'fePointLight' element.
     */
    SVGAnimatedNumber getY();

    /**
     * Corresponds to attribute z on the given 'fePointLight' element.
     */
    SVGAnimatedNumber getZ();

    //####################################################################
    //# SVGFESpecularLightingElement
    //####################################################################


    /**
     *
     */
    SVGAnimatedString getIn1();

    /**
     *
     */
    SVGAnimatedNumber getSurfaceScale();

    /**
     *
     */
    SVGAnimatedNumber getSpecularConstant();

    /**
     *
     */
    SVGAnimatedNumber getSpecularExponent();


    //####################################################################
    //# SVGFESpotLightElement
    //####################################################################

    /**
     * Corresponds to attribute x on the given 'feSpotLight' element.
     */
    SVGAnimatedNumber getX();

    /**
     * Corresponds to attribute y on the given 'feSpotLight' element.
     */
    SVGAnimatedNumber getY();

    /**
     * Corresponds to attribute z on the given 'feSpotLight' element.
     */
    SVGAnimatedNumber getZ();

    /**
     * Corresponds to attribute pointsAtX on the given 'feSpotLight' element.
     */
    SVGAnimatedNumber getPointsAtX();

    /**
     * Corresponds to attribute pointsAtY on the given 'feSpotLight' element.
     */
    SVGAnimatedNumber getPointsAtY();

    /**
     * Corresponds to attribute pointsAtZ on the given 'feSpotLight' element.
     */
    SVGAnimatedNumber getPointsAtZ();

    /**
     * Corresponds to attribute specularExponent on the
     *      given 'feSpotLight'  element.
     */
    SVGAnimatedNumber getSpecularExponent();

    /**
     * Corresponds to attribute limitingConeAngle on the
     *      given 'feSpotLight'  element.
     */
    SVGAnimatedNumber getLimitingConeAngle();


    //####################################################################
    //# SVGFETileElement
    //####################################################################


    /**
     *
     */
    SVGAnimatedString getIn1();


    //####################################################################
    //# SVGFETurbulenceElement
    //####################################################################


    /**
     *  Turbulence Types
     */
    typedef enum
        {
        SVG_TURBULENCE_TYPE_UNKNOWN      = 0,
        SVG_TURBULENCE_TYPE_FRACTALNOISE = 1,
        SVG_TURBULENCE_TYPE_TURBULENCE   = 2
        } TurbulenceType;

    /**
     *  Stitch Options
     */
    typedef enum
        {
        SVG_STITCHTYPE_UNKNOWN  = 0,
        SVG_STITCHTYPE_STITCH   = 1,
        SVG_STITCHTYPE_NOSTITCH = 2
        } StitchOption;



    /**
     *
     */
    SVGAnimatedNumber getBaseFrequencyX();

    /**
     *
     */
    SVGAnimatedNumber getBaseFrequencyY();

    /**
     *
     */
    SVGAnimatedInteger getNumOctaves();

    /**
     *
     */
    SVGAnimatedNumber getSeed();

    /**
     *
     */
    SVGAnimatedEnumeration getStitchTiles();

    /**
     *
     */
    SVGAnimatedEnumeration getType();



    //####################################################################
    //# SVGFilterElement
    //####################################################################


    /**
     * Corresponds to attribute filterUnits on the given 'filter' element. Takes one
     * of the constants defined in SVGUnitTypes.
     */
    SVGAnimatedEnumeration getFilterUnits();

    /**
     * Corresponds to attribute primitiveUnits on the given 'filter' element. Takes
     * one of the constants defined in SVGUnitTypes.
     */
    SVGAnimatedEnumeration getPrimitiveUnits();

    /**
     *
     */
    SVGAnimatedLength getX();

    /**
     * Corresponds to attribute x on the given 'filter' element.
     */
    SVGAnimatedLength getY();

    /**
     * Corresponds to attribute y on the given 'filter' element.
     */
    SVGAnimatedLength getWidth();

    /**
     * Corresponds to attribute height on the given 'filter' element.
     */
    SVGAnimatedLength getHeight();


    /**
     * Corresponds to attribute filterRes on the given 'filter' element.
     *      Contains the X component of attribute filterRes.
     */
    SVGAnimatedInteger getFilterResX();

    /**
     * Corresponds to attribute filterRes on the given 'filter' element.
     * Contains the Y component(possibly computed automatically)
     *      of attribute filterRes.
     */
    SVGAnimatedInteger getFilterResY();

    /**
     * Sets the values for attribute filterRes.
     */
    void setFilterRes(unsigned long filterResX, unsigned long filterResY);


    //####################################################################
    //# SVGFontElement
    //####################################################################

    //####################################################################
    //# SVGFontFaceElement
    //####################################################################

    //####################################################################
    //# SVGFontFaceFormatElement
    //####################################################################

    //####################################################################
    //# SVGFontFaceNameElement
    //####################################################################

    //####################################################################
    //# SVGFontFaceSrcElement
    //####################################################################

    //####################################################################
    //# SVGFontFaceUriElement
    //####################################################################

    //####################################################################
    //# SVGForeignObjectElement
    //####################################################################

    /**
     *
     */
    SVGAnimatedLength getX();

    /**
     *
     */
    SVGAnimatedLength getY();

    /**
     *
     */
    SVGAnimatedLength getWidth();

    /**
     *
     */
    SVGAnimatedLength getHeight();



    //####################################################################
    //# SVGGlyphRefElement
    //####################################################################


    /**
     * Get the attribute glyphRef on the given element.
     */
    DOMString getGlyphRef();

    /**
     * Set the attribute glyphRef on the given element.
     */
    void setGlyphRef(const DOMString &val) throw(DOMException);

    /**
     * Get the attribute format on the given element.
     */
    DOMString getFormat();

    /**
     * Set the attribute format on the given element.
     */
    void setFormat(const DOMString &val) throw(DOMException);

    /**
     * Get the attribute x on the given element.
     */
    double getX();

    /**
     * Set the attribute x on the given element.
     */
    void setX(double val) throw(DOMException);

    /**
     * Get the attribute y on the given element.
     */
    double getY();

    /**
     * Set the attribute y on the given element.
     */
    void setY(double val) throw(DOMException);

    /**
     * Get the attribute dx on the given element.
     */
    double getDx();

    /**
     * Set the attribute dx on the given element.
     */
    void setDx(double val) throw(DOMException);

    /**
     * Get the attribute dy on the given element.
     */
    double getDy();

    /**
     * Set the attribute dy on the given element.
     */
    void setDy(double val) throw(DOMException);


    //####################################################################
    //# SVGGradientElement
    //####################################################################


    /**
     * Spread Method Types
     */
    typedef enum
        {
        SVG_SPREADMETHOD_UNKNOWN = 0,
        SVG_SPREADMETHOD_PAD     = 1,
        SVG_SPREADMETHOD_REFLECT = 2,
        SVG_SPREADMETHOD_REPEAT  = 3
        } SpreadMethodType;


    /**
     * Corresponds to attribute gradientUnits on the given element.
     *      Takes one of the constants defined in SVGUnitTypes.
     */
    SVGAnimatedEnumeration getGradientUnits();

    /**
     * Corresponds to attribute gradientTransform on the given element.
     */
    SVGAnimatedTransformList getGradientTransform();

    /**
     * Corresponds to attribute spreadMethod on the given element.
     *      One of the Spread Method Types.
     */
    SVGAnimatedEnumeration getSpreadMethod();



    //####################################################################
    //# SVGHKernElement
    //####################################################################

    //####################################################################
    //# SVGImageElement
    //####################################################################

    /**
     * Corresponds to attribute x on the given 'image' element.
     */
    SVGAnimatedLength getX();

    /**
     * Corresponds to attribute y on the given 'image' element.
     */
    SVGAnimatedLength getY();

    /**
     * Corresponds to attribute width on the given 'image' element.
     */
    SVGAnimatedLength getWidth();

    /**
     * Corresponds to attribute height on the given 'image' element.
     */
    SVGAnimatedLength getHeight();


    /**
     * Corresponds to attribute preserveAspectRatio on the given element.
     */
    SVGAnimatedPreserveAspectRatio getPreserveAspectRatio();

    //####################################################################
    //# SVGLinearGradientElement
    //####################################################################

    /**
     * Corresponds to attribute x1 on the given 'linearGradient'  element.
     */
    SVGAnimatedLength getX1();

    /**
     * Corresponds to attribute y1 on the given 'linearGradient'  element.
     */
    SVGAnimatedLength getY1();

    /**
     * Corresponds to attribute x2 on the given 'linearGradient'  element.
     */
    SVGAnimatedLength getX2();

    /**
     * Corresponds to attribute y2 on the given 'linearGradient'  element.
     */
    SVGAnimatedLength getY2();



    //####################################################################
    //# SVGLineElement
    //####################################################################

    /**
     * Corresponds to attribute x1 on the given 'line' element.
     */
    SVGAnimatedLength getX1();

    /**
     * Corresponds to attribute y1 on the given 'line' element.
     */
    SVGAnimatedLength getY1();

    /**
     * Corresponds to attribute x2 on the given 'line' element.
     */
    SVGAnimatedLength getX2();

    /**
     * Corresponds to attribute y2 on the given 'line' element.
     */
    SVGAnimatedLength getY2();


    //####################################################################
    //# SVGMarkerElement
    //####################################################################


    /**
     * Marker Unit Types
     */
    typedef enum
        {
        SVG_MARKERUNITS_UNKNOWN        = 0,
        SVG_MARKERUNITS_USERSPACEONUSE = 1,
        SVG_MARKERUNITS_STROKEWIDTH    = 2
        } MarkerUnitType;

    /**
     * Marker Orientation Types
     */
    typedef enum
        {
        SVG_MARKER_ORIENT_UNKNOWN      = 0,
        SVG_MARKER_ORIENT_AUTO         = 1,
        SVG_MARKER_ORIENT_ANGLE        = 2
        } MarkerOrientationType;


    /**
     * Corresponds to attribute refX on the given 'marker' element.
     */
    SVGAnimatedLength getRefX();

    /**
     * Corresponds to attribute refY on the given 'marker' element.
     */
    SVGAnimatedLength getRefY();

    /**
     * Corresponds to attribute markerUnits on the given 'marker' element.
     *      One of the Marker Units Types defined above.
     */
    SVGAnimatedEnumeration getMarkerUnits();

    /**
     * Corresponds to attribute markerWidth on the given 'marker' element.
     */
    SVGAnimatedLength getMarkerWidth();

    /**
     * Corresponds to attribute markerHeight on the given 'marker' element.
     */
    SVGAnimatedLength getMarkerHeight();

    /**
     * Corresponds to attribute orient on the given 'marker' element.
     *      One of the Marker Orientation Types defined above.
     */
    SVGAnimatedEnumeration getOrientType();

    /**
     * Corresponds to attribute orient on the given 'marker' element.
     * If markerUnits is SVG_MARKER_ORIENT_ANGLE, the angle value for
     * attribute orient; otherwise, it will be set to zero.
     */
    SVGAnimatedAngle getOrientAngle();


    /**
     * Sets the value of attribute orient to 'auto'.
     */
    void setOrientToAuto();

    /**
     * Sets the value of attribute orient to the given angle.
     */
    void setOrientToAngle(const SVGAngle &angle);


    //####################################################################
    //# SVGMaskElement
    //####################################################################


    /**
     * Corresponds to attribute maskUnits on the given 'mask' element. Takes one of
     * the constants defined in SVGUnitTypes.
     */
    SVGAnimatedEnumeration getMaskUnits();

    /**
     * Corresponds to attribute maskContentUnits on the given 'mask' element. Takes
     * one of the constants defined in SVGUnitTypes.
     */
    SVGAnimatedEnumeration getMaskContentUnits();

    /**
     * Corresponds to attribute x on the given 'mask' element.
     */
    SVGAnimatedLength getX();

    /**
     * Corresponds to attribute y on the given 'mask' element.
     */
    SVGAnimatedLength getY();

    /**
     * Corresponds to attribute width on the given 'mask' element.
     */
    SVGAnimatedLength getWidth();

    /**
     * Corresponds to attribute height on the given 'mask' element.
     */
    SVGAnimatedLength getHeight();

    //####################################################################
    //# SVGMetadataElement
    //####################################################################

    //####################################################################
    //# SVGMissingGlyphElement
    //####################################################################


    //####################################################################
    //# SVGMPathElement
    //####################################################################

    /**
     * Corresponds to attribute pathLength on the given 'path' element.
     */
    SVGAnimatedNumber getPathLength();

    /**
     * Returns the user agent's computed value for the total length of the path using
     * the user agent's distance-along-a-path algorithm, as a distance in the current
     * user coordinate system.
     */
    double getTotalLength();

    /**
     * Returns the(x,y) coordinate in user space which is distance units along the
     * path, utilizing the user agent's distance-along-a-path algorithm.
     */
    SVGPoint getPointAtLength(double distance);

    /**
     * Returns the index into pathSegList which is distance units along the path,
     * utilizing the user agent's distance-along-a-path algorithm.
     */
    unsigned long getPathSegAtLength(double distance);

    /**
     * Returns a stand-alone, parentless SVGPathSegClosePath object.
     */
    SVGPathSeg createSVGPathSegClosePath();

    /**
     * Returns a stand-alone, parentless SVGPathSegMovetoAbs object.
     */
    SVGPathSeg createSVGPathSegMovetoAbs(double x, double y);

    /**
     * Returns a stand-alone, parentless SVGPathSegMovetoRel object.
     */
    SVGPathSeg createSVGPathSegMovetoRel(double x, double y);

    /**
     * Returns a stand-alone, parentless SVGPathSegLinetoAbs object.
     */
    SVGPathSeg createSVGPathSegLinetoAbs(double x, double y);

    /**
     * Returns a stand-alone, parentless SVGPathSegLinetoRel object.
     */
    SVGPathSeg createSVGPathSegLinetoRel(double x, double y);

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoCubicAbs object.
     */
    SVGPathSeg createSVGPathSegCurvetoCubicAbs(double x, double y,
                        double x1, double y1, double x2, double y2);

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoCubicRel object.
     */
    SVGPathSeg createSVGPathSegCurvetoCubicRel(double x, double y,
                        double x1, double y1, double x2, double y2);

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoQuadraticAbs object.
     */
    SVGPathSeg createSVGPathSegCurvetoQuadraticAbs(double x, double y,
                         double x1, double y1);

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoQuadraticRel object.
     */
    SVGPathSeg createSVGPathSegCurvetoQuadraticRel(double x, double y,
                         double x1, double y1);

    /**
     * Returns a stand-alone, parentless SVGPathSegArcAbs object.
     */
    SVGPathSeg createSVGPathSegArcAbs(double x, double y,
                         double r1, double r2, double angle,
                         bool largeArcFlag, bool sweepFlag);

    /**
     * Returns a stand-alone, parentless SVGPathSegArcRel object.
     */
    SVGPathSeg createSVGPathSegArcRel(double x, double y, double r1,
                         double r2, double angle, bool largeArcFlag,
                         bool sweepFlag);

    /**
     * Returns a stand-alone, parentless SVGPathSegLinetoHorizontalAbs object.
     */
    SVGPathSeg createSVGPathSegLinetoHorizontalAbs(double x);

    /**
     * Returns a stand-alone, parentless SVGPathSegLinetoHorizontalRel object.
     */
    SVGPathSeg createSVGPathSegLinetoHorizontalRel(double x);

    /**
     * Returns a stand-alone, parentless SVGPathSegLinetoVerticalAbs object.
     */
    SVGPathSeg createSVGPathSegLinetoVerticalAbs(double y);

    /**
     * Returns a stand-alone, parentless SVGPathSegLinetoVerticalRel object.
     */
    SVGPathSeg createSVGPathSegLinetoVerticalRel(double y);

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoCubicSmoothAbs object.
     */
    SVGPathSeg createSVGPathSegCurvetoCubicSmoothAbs(double x, double y,
                                             double x2, double y2);

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoCubicSmoothRel object.
     */
    SVGPathSeg createSVGPathSegCurvetoCubicSmoothRel(double x, double y,
                                                      double x2, double y2);

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoQuadraticSmoothAbs
     *      object.
     */
    SVGPathSeg createSVGPathSegCurvetoQuadraticSmoothAbs(double x, double y);

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoQuadraticSmoothRel
     *      object.
     */
    SVGPathSeg createSVGPathSegCurvetoQuadraticSmoothRel(double x, double y);

    //####################################################################
    //# SVGPathElement
    //####################################################################

    //####################################################################
    //# SVGPatternElement
    //####################################################################

    /**
     * Corresponds to attribute patternUnits on the given 'pattern' element.
     * Takes one of the constants defined in SVGUnitTypes.
     */
    SVGAnimatedEnumeration getPatternUnits();

    /**
     * Corresponds to attribute patternContentUnits on the given 'pattern'
     *      element. Takes one of the constants defined in SVGUnitTypes.
     */
    SVGAnimatedEnumeration getPatternContentUnits();

    /**
     * Corresponds to attribute patternTransform on the given 'pattern' element.
     */
    SVGAnimatedTransformList getPatternTransform();

    /**
     * Corresponds to attribute x on the given 'pattern' element.
     */
    SVGAnimatedLength getX();

    /**
     *
     */
    SVGAnimatedLength getY();

    /**
     * Corresponds to attribute width on the given 'pattern' element.
     */
    SVGAnimatedLength getWidth();

    /**
     * Corresponds to attribute height on the given 'pattern' element.
     */
    SVGAnimatedLength getHeight();


    //####################################################################
    //# SVGPolyLineElement
    //####################################################################

    //####################################################################
    //# SVGPolygonElement
    //####################################################################

    //####################################################################
    //# SVGRadialGradientElement
    //####################################################################


    /**
     * Corresponds to attribute cx on the given 'radialGradient'  element.
     */
    SVGAnimatedLength getCx();


    /**
     * Corresponds to attribute cy on the given 'radialGradient'  element.
     */
    SVGAnimatedLength getCy();


    /**
     * Corresponds to attribute r on the given 'radialGradient'  element.
     */
    SVGAnimatedLength getR();


    /**
     * Corresponds to attribute fx on the given 'radialGradient'  element.
     */
    SVGAnimatedLength getFx();


    /**
     * Corresponds to attribute fy on the given 'radialGradient'  element.
     */
    SVGAnimatedLength getFy();


    //####################################################################
    //# SVGRectElement
    //####################################################################

    /**
     * Corresponds to attribute x on the given 'rect' element.
     */
    SVGAnimatedLength getX();

    /**
     * Corresponds to attribute y on the given 'rect' element.
     */
    SVGAnimatedLength getY();

    /**
     * Corresponds to attribute width on the given 'rect' element.
     */
    SVGAnimatedLength getWidth();

    /**
     * Corresponds to attribute height on the given 'rect' element.
     */
    SVGAnimatedLength getHeight();


    /**
     * Corresponds to attribute rx on the given 'rect' element.
     */
    SVGAnimatedLength getRx();

    /**
     * Corresponds to attribute ry on the given 'rect' element.
     */
    SVGAnimatedLength getRy();


    //####################################################################
    //# SVGScriptElement
    //####################################################################

    /**
     *
     */
    DOMString getType();

    /**
     *
     */
    void setType(const DOMString &val) throw(DOMException);

    //####################################################################
    //# SVGSetElement
    //####################################################################

    //####################################################################
    //# SVGStopElement
    //####################################################################


    /**
     * Corresponds to attribute offset on the given 'stop' element.
     */
    SVGAnimatedNumber getOffset();


    //####################################################################
    //# SVGStyleElement
    //####################################################################

    /**
     * Get the attribute xml:space on the given element.
     */
    DOMString getXmlspace();

    /**
     * Set the attribute xml:space on the given element.
     */
    void setXmlspace(const DOMString &val) throw(DOMException);

    /**
     * Get the attribute type on the given 'style' element.
     */
    DOMString getType();

    /**
     * Set the attribute type on the given 'style' element.
     */
    void setType(const DOMString &val) throw(DOMException);

    /**
     * Get the attribute media on the given 'style' element.
     */
    DOMString getMedia();

    /**
     * Set the attribute media on the given 'style' element.
     */
    void setMedia(const DOMString &val) throw(DOMException);

    /**
     * Get the attribute title on the given 'style' element.
     */
    DOMString getTitle();

    /**
     * Set the attribute title on the given 'style' element.
     */
    void setTitle(const DOMString &val) throw(DOMException);

    //####################################################################
    //# SVGSymbolElement
    //####################################################################

    //####################################################################
    //# SVGSVGElement
    //####################################################################

    /**
     * Corresponds to attribute x on the given 'svg' element.
     */
    SVGAnimatedLength getX();

    /**
     * Corresponds to attribute y on the given 'svg' element.
     */
    SVGAnimatedLength getY();

    /**
     * Corresponds to attribute width on the given 'svg' element.
     */
    SVGAnimatedLength getWidth();

    /**
     * Corresponds to attribute height on the given 'svg' element.
     */
    SVGAnimatedLength getHeight();

    /**
     * Get the attribute contentScriptType on the given 'svg' element.
     */
    DOMString getContentScriptType();

    /**
     * Set the attribute contentScriptType on the given 'svg' element.
     */
    void setContentScriptType(const DOMString &val) throw(DOMException);


    /**
     * Get the attribute contentStyleType on the given 'svg' element.
     */
    DOMString getContentStyleType();

    /**
     * Set the attribute contentStyleType on the given 'svg' element.
     */
    void setContentStyleType(const DOMString &val) throw(DOMException);

    /**
     * The position and size of the viewport(implicit or explicit) that corresponds
     * to this 'svg' element. When the user agent is actually rendering the content,
     * then the position and size values represent the actual values when rendering.
     * The position and size values are unitless values in the coordinate system of
     * the parent element. If no parent element exists(i.e., 'svg' element
     * represents the root of the document tree), if this SVG document is embedded as
     * part of another document(e.g., via the HTML 'object' element), then the
     * position and size are unitless values in the coordinate system of the parent
     * document.(If the parent uses CSS or XSL layout, then unitless values
     * represent pixel units for the current CSS or XSL viewport, as described in the
     * CSS2 specification.) If the parent element does not have a coordinate system,
     * then the user agent should provide reasonable default values for this attribute.
     *      */
    SVGRect getViewport();

    /**
     * Size of a pixel units(as defined by CSS2) along the x-axis of the viewport,
     * which represents a unit somewhere in the range of 70dpi to 120dpi, and, on
     * systems that support this, might actually match the characteristics of the
     * target medium. On systems where it is impossible to know the size of a pixel,
     * a suitable default pixel size is provided.
     */
    double getPixelUnitToMillimeterX();

    /**
     * Corresponding size of a pixel unit along the y-axis of the viewport.
     */
    double getPixelUnitToMillimeterY();

    /**
     * User interface(UI) events in DOM Level 2 indicate the screen positions at
     * which the given UI event occurred. When the user agent actually knows the
     * physical size of a "screen unit", this attribute will express that information;
     *  otherwise, user agents will provide a suitable default value such as .28mm.
     */
    double getScreenPixelToMillimeterX();

    /**
     * Corresponding size of a screen pixel along the y-axis of the viewport.
     */
    double getScreenPixelToMillimeterY();


    /**
     * The initial view(i.e., before magnification and panning) of the current
     * innermost SVG document fragment can be either the "standard" view(i.e., based
     * on attributes on the 'svg' element such as fitBoxToViewport) or to a "custom"
     * view(i.e., a hyperlink into a particular 'view' or other element - see
     * Linking into SVG content: URI fragments and SVG views). If the initial view is
     * the "standard" view, then this attribute is false. If the initial view is a
     * "custom" view, then this attribute is true.
     */
    bool getUseCurrentView();

    /**
     * Set the value above
     */
    void setUseCurrentView(bool val) throw(DOMException);

    /**
     * The definition of the initial view(i.e., before magnification and panning) of
     * the current innermost SVG document fragment. The meaning depends on the
     * situation:
     * 
     *    * If the initial view was a "standard" view, then:
     *      o the values for viewBox, preserveAspectRatio and zoomAndPan within
     *        currentView will match the values for the corresponding DOM attributes that
     *        are on SVGSVGElement directly
     *      o the values for transform and viewTarget within currentView will be null
     *    * If the initial view was a link into a 'view' element, then:
     *      o the values for viewBox, preserveAspectRatio and zoomAndPan within
     *        currentView will correspond to the corresponding attributes for the given
     *        'view' element
     *      o the values for transform and viewTarget within currentView will be null
     *    * If the initial view was a link into another element(i.e., other than a
     *      'view'), then:
     *      o the values for viewBox, preserveAspectRatio and zoomAndPan within
     *        currentView will match the values for the corresponding DOM attributes that
     *        are on SVGSVGElement directly for the closest ancestor 'svg' element
     *      o the values for transform within currentView will be null
     *      o the viewTarget within currentView will represent the target of the link
     *    * If the initial view was a link into the SVG document fragment using an SVG
     *      view specification fragment identifier(i.e., #svgView(...)), then:
     *      o the values for viewBox, preserveAspectRatio, zoomAndPan, transform and
     *        viewTarget within currentView will correspond to the values from the SVG view
     *        specification fragment identifier
     * 
     */
    SVGViewSpec getCurrentView();


    /**
     * This attribute indicates the current scale factor relative to the initial view
     * to take into account user magnification and panning operations, as described
     * under Magnification and panning. DOM attributes currentScale and
     * currentTranslate are equivalent to the 2x3 matrix [a b c d e f] =
     * [currentScale 0 0 currentScale currentTranslate.x currentTranslate.y]. If
     * "magnification" is enabled(i.e., zoomAndPan="magnify"), then the effect is as
     * if an extra transformation were placed at the outermost level on the SVG
     * document fragment(i.e., outside the outermost 'svg' element).
     */
    double getCurrentScale();

    /**
     *  Set the value above.
     */
    void setCurrentScale(double val) throw(DOMException);

    /**
     * The corresponding translation factor that takes into account
     *      user "magnification".
     */
    SVGPoint getCurrentTranslate();

    /**
     * Takes a time-out value which indicates that redraw shall not occur until:(a)
     * the corresponding unsuspendRedraw(suspend_handle_id) call has been made,(b)
     * an unsuspendRedrawAll() call has been made, or(c) its timer has timed out. In
     * environments that do not support interactivity(e.g., print media), then
     * redraw shall not be suspended. suspend_handle_id =
     * suspendRedraw(max_wait_milliseconds) and unsuspendRedraw(suspend_handle_id)
     * must be packaged as balanced pairs. When you want to suspend redraw actions as
     * a collection of SVG DOM changes occur, then precede the changes to the SVG DOM
     * with a method call similar to suspend_handle_id =
     * suspendRedraw(max_wait_milliseconds) and follow the changes with a method call
     * similar to unsuspendRedraw(suspend_handle_id). Note that multiple
     * suspendRedraw calls can be used at once and that each such method call is
     * treated independently of the other suspendRedraw method calls.
     */
    unsigned long suspendRedraw(unsigned long max_wait_milliseconds);

    /**
     * Cancels a specified suspendRedraw() by providing a unique suspend_handle_id.
     */
    void unsuspendRedraw(unsigned long suspend_handle_id) throw(DOMException);

    /**
     * Cancels all currently active suspendRedraw() method calls. This method is most
     * useful at the very end of a set of SVG DOM calls to ensure that all pending
     * suspendRedraw() method calls have been cancelled.
     */
    void unsuspendRedrawAll();

    /**
     * In rendering environments supporting interactivity, forces the user agent to
     * immediately redraw all regions of the viewport that require updating.
     */
    void forceRedraw();

    /**
     * Suspends(i.e., pauses) all currently running animations that are defined
     * within the SVG document fragment corresponding to this 'svg' element, causing
     * the animation clock corresponding to this document fragment to stand still
     * until it is unpaused.
     */
    void pauseAnimations();

    /**
     * Unsuspends(i.e., unpauses) currently running animations that are defined
     * within the SVG document fragment, causing the animation clock to continue from
     * the time at which it was suspended.
     */
    void unpauseAnimations();

    /**
     * Returns true if this SVG document fragment is in a paused state.
     */
    bool animationsPaused();

    /**
     * Returns the current time in seconds relative to the start time for
     *      the current SVG document fragment.
     */
    double getCurrentTime();

    /**
     * Adjusts the clock for this SVG document fragment, establishing
     *      a new current time.
     */
    void setCurrentTime(double seconds);

    /**
     * Returns the list of graphics elements whose rendered content intersects the
     * supplied rectangle, honoring the 'pointer-events' property value on each
     * candidate graphics element.
     */
    NodeList getIntersectionList(const SVGRect &rect,
                                 const SVGElementPtr referenceElement);

    /**
     * Returns the list of graphics elements whose rendered content is entirely
     * contained within the supplied rectangle, honoring the 'pointer-events'
     * property value on each candidate graphics element.
     */
    NodeList getEnclosureList(const SVGRect &rect,
                              const SVGElementPtr referenceElement);

    /**
     * Returns true if the rendered content of the given element intersects the
     * supplied rectangle, honoring the 'pointer-events' property value on each
     * candidate graphics element.
     */
    bool checkIntersection(const SVGElementPtr element, const SVGRect &rect);

    /**
     * Returns true if the rendered content of the given element is entirely
     * contained within the supplied rectangle, honoring the 'pointer-events'
     * property value on each candidate graphics element.
     */
    bool checkEnclosure(const SVGElementPtr element, const SVGRect &rect);

    /**
     * Unselects any selected objects, including any selections of text
     *      strings and type-in bars.
     */
    void deselectAll();

    /**
     * Creates an SVGNumber object outside of any document trees. The object
     *      is initialized to a value of zero.
     */
    SVGNumber createSVGNumber();

    /**
     * Creates an SVGLength object outside of any document trees. The object
     *      is initialized to the value of 0 user units.
     */
    SVGLength createSVGLength();

    /**
     * Creates an SVGAngle object outside of any document trees. The object
     *      is initialized to the value 0 degrees(unitless).
     */
    SVGAngle createSVGAngle();

    /**
     * Creates an SVGPoint object outside of any document trees. The object
     * is initialized to the point(0,0) in the user coordinate system.
     */
    SVGPoint createSVGPoint();

    /**
     * Creates an SVGMatrix object outside of any document trees. The object
     *      is initialized to the identity matrix.
     */
    SVGMatrix createSVGMatrix();

    /**
     * Creates an SVGRect object outside of any document trees. The object
     *      is initialized such that all values are set to 0 user units.
     */
    SVGRect createSVGRect();

    /**
     * Creates an SVGTransform object outside of any document trees.
     * The object is initialized to an identity matrix transform
     *    (SVG_TRANSFORM_MATRIX).
     */
    SVGTransform createSVGTransform();

    /**
     * Creates an SVGTransform object outside of any document trees.
     * The object is initialized to the given matrix transform
     *    (i.e., SVG_TRANSFORM_MATRIX).
     */
    SVGTransform createSVGTransformFromMatrix(const SVGMatrix &matrix);

    /**
     * Searches this SVG document fragment(i.e., the search is restricted to a
     * subset of the document tree) for an Element whose id is given by elementId. If
     * an Element is found, that Element is returned. If no such element exists,
     * returns null. Behavior is not defined if more than one element has this id.
     */
    ElementPtr getElementById(const DOMString& elementId);


    //####################################################################
    //# SVGTextElement
    //####################################################################


    //####################################################################
    //# SVGTextContentElement
    //####################################################################


    /**
     * lengthAdjust Types
     */
    typedef enum
        {
        LENGTHADJUST_UNKNOWN          = 0,
        LENGTHADJUST_SPACING          = 1,
        LENGTHADJUST_SPACINGANDGLYPHS = 2
        } LengthAdjustType;


    /**
     * Corresponds to attribute textLength on the given element.
     */
    SVGAnimatedLength getTextLength();


    /**
     * Corresponds to attribute lengthAdjust on the given element. The value must be
     * one of the length adjust constants specified above.
     */
    SVGAnimatedEnumeration getLengthAdjust();


    /**
     * Returns the total number of characters to be rendered within the current
     * element. Includes characters which are included via a 'tref' reference.
     */
    long getNumberOfChars();

    /**
     * The total sum of all of the advance values from rendering all of the
     * characters within this element, including the advance value on the glyphs
     *(horizontal or vertical), the effect of properties 'kerning', 'letter-spacing'
     * and 'word-spacing' and adjustments due to attributes dx and dy on 'tspan'
     * elements. For non-rendering environments, the user agent shall make reasonable
     * assumptions about glyph metrics.
     */
    double getComputedTextLength();

    /**
     * The total sum of all of the advance values from rendering the specified
     * substring of the characters, including the advance value on the glyphs
     *(horizontal or vertical), the effect of properties 'kerning', 'letter-spacing'
     * and 'word-spacing' and adjustments due to attributes dx and dy on 'tspan'
     * elements. For non-rendering environments, the user agent shall make reasonable
     * assumptions about glyph metrics.
     */
    double getSubStringLength(unsigned long charnum, unsigned long nchars)
                                     throw(DOMException);

    /**
     * Returns the current text position before rendering the character in the user
     * coordinate system for rendering the glyph(s) that correspond to the specified
     * character. The current text position has already taken into account the
     * effects of any inter-character adjustments due to properties 'kerning',
     * 'letter-spacing' and 'word-spacing' and adjustments due to attributes x, y, dx
     * and dy. If multiple consecutive characters are rendered inseparably(e.g., as
     * a single glyph or a sequence of glyphs), then each of the inseparable
     * characters will return the start position for the first glyph.
     */
    SVGPoint getStartPositionOfChar(unsigned long charnum) throw(DOMException);

    /**
     * Returns the current text position after rendering the character in the user
     * coordinate system for rendering the glyph(s) that correspond to the specified
     * character. This current text position does not take into account the effects
     * of any inter-character adjustments to prepare for the next character, such as
     * properties 'kerning', 'letter-spacing' and 'word-spacing' and adjustments due
     * to attributes x, y, dx and dy. If multiple consecutive characters are rendered
     * inseparably(e.g., as a single glyph or a sequence of glyphs), then each of
     * the inseparable characters will return the end position for the last glyph.
     */
    SVGPoint getEndPositionOfChar(unsigned long charnum) throw(DOMException);

    /**
     * Returns a tightest rectangle which defines the minimum and maximum X and Y
     * values in the user coordinate system for rendering the glyph(s) that
     * correspond to the specified character. The calculations assume that all glyphs
     * occupy the full standard glyph cell for the font. If multiple consecutive
     * characters are rendered inseparably(e.g., as a single glyph or a sequence of
     * glyphs), then each of the inseparable characters will return the same extent.
     */
    SVGRect getExtentOfChar(unsigned long charnum) throw(DOMException);

    /**
     * Returns the rotation value relative to the current user coordinate system used
     * to render the glyph(s) corresponding to the specified character. If multiple
     * glyph(s) are used to render the given character and the glyphs each have
     * different rotations(e.g., due to text-on-a-path), the user agent shall return
     * an average value(e.g., the rotation angle at the midpoint along the path for
     * all glyphs used to render this character). The rotation value represents the
     * rotation that is supplemental to any rotation due to properties
     * 'glyph-orientation-horizontal' and 'glyph-orientation-vertical'; thus, any
     * glyph rotations due to these properties are not included into the returned
     * rotation value. If multiple consecutive characters are rendered inseparably
     *(e.g., as a single glyph or a sequence of glyphs), then each of the
     * inseparable characters will return the same rotation value.
     */
    double getRotationOfChar(unsigned long charnum) throw(DOMException);

    /**
     * Returns the index of the character whose corresponding glyph cell bounding box
     * contains the specified point. The calculations assume that all glyphs occupy
     * the full standard glyph cell for the font. If no such character exists, a
     * value of -1 is returned. If multiple such characters exist, the character
     * within the element whose glyphs were rendered last(i.e., take into account
     * any reordering such as for bidirectional text) is used. If multiple
     * consecutive characters are rendered inseparably(e.g., as a single glyph or a
     * sequence of glyphs), then the user agent shall allocate an equal percentage of
     * the text advance amount to each of the contributing characters in determining
     * which of the characters is chosen.
     */
    long getCharNumAtPosition(const SVGPoint &point);

    /**
     * Causes the specified substring to be selected just as if the user
     *      selected the substring interactively.
     */
    void selectSubString(unsigned long charnum, unsigned long nchars)
                                  throw(DOMException);





    //####################################################################
    //# SVGTextPathElement
    //####################################################################


    /**
     * textPath Method Types
     */
    typedef enum
        {
        TEXTPATH_METHODTYPE_UNKNOWN   = 0,
        TEXTPATH_METHODTYPE_ALIGN     = 1,
        TEXTPATH_METHODTYPE_STRETCH   = 2
        } TextPathMethodType;

    /**
     * textPath Spacing Types
     */
    typedef enum
        {
        TEXTPATH_SPACINGTYPE_UNKNOWN  = 0,
        TEXTPATH_SPACINGTYPE_AUTO     = 1,
        TEXTPATH_SPACINGTYPE_EXACT    = 2
        } TextPathSpacingType;


    /**
     * Corresponds to attribute startOffset on the given 'textPath' element.
     */
    SVGAnimatedLength getStartOffset();

    /**
     * Corresponds to attribute method on the given 'textPath' element. The value
     * must be one of the method type constants specified above.
     */
    SVGAnimatedEnumeration getMethod();

    /**
     * Corresponds to attribute spacing on the given 'textPath' element.
     *  The value must be one of the spacing type constants specified above.
     */
    SVGAnimatedEnumeration getSpacing();


    //####################################################################
    //# SVGTextPositioningElement
    //####################################################################


    /**
     * Corresponds to attribute x on the given element.
     */
    SVGAnimatedLength getX();

    /**
     * Corresponds to attribute y on the given element.
     */
    SVGAnimatedLength getY();

    /**
     * Corresponds to attribute dx on the given element.
     */
    SVGAnimatedLength getDx();

    /**
     * Corresponds to attribute dy on the given element.
     */
    SVGAnimatedLength getDy();


    /**
     * Corresponds to attribute rotate on the given element.
     */
    SVGAnimatedNumberList getRotate();


    //####################################################################
    //# SVGTitleElement
    //####################################################################

    //####################################################################
    //# SVGTRefElement
    //####################################################################

    //####################################################################
    //# SVGTSpanElement
    //####################################################################

    //####################################################################
    //# SVGSwitchElement
    //####################################################################

    //####################################################################
    //# SVGUseElement
    //####################################################################

    /**
     * Corresponds to attribute x on the given 'use' element.
     */
    SVGAnimatedLength getX();

    /**
     * Corresponds to attribute y on the given 'use' element.
     */
    SVGAnimatedLength getY();

    /**
     * Corresponds to attribute width on the given 'use' element.
     */
    SVGAnimatedLength getWidth();

    /**
     * Corresponds to attribute height on the given 'use' element.
     */
    SVGAnimatedLength getHeight();

    /**
     * The root of the "instance tree". See description of SVGElementInstance for
     * a discussion on the instance tree.
     *      */
    SVGElementInstance getInstanceRoot();

    /**
     * If the 'href' attribute is being animated, contains the current animated root
     * of the "instance tree". If the 'href' attribute is not currently being
     * animated, contains the same value as 'instanceRoot'. The root of the "instance
     * tree". See description of SVGElementInstance for a discussion on the instance
     * tree.
     */
    SVGElementInstance getAnimatedInstanceRoot();

    //####################################################################
    //# SVGVKernElement
    //####################################################################

    //####################################################################
    //# SVGViewElement
    //####################################################################


    /**
     *
     */
    SVGStringList getViewTarget();




    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    ~SVGElement() {}


};



/*#########################################################################
## SVGDocument
#########################################################################*/

/**
 * When an 'svg' element is embedded inline as a component of a document from
 * another namespace, such as when an 'svg' element is embedded inline within an
 * XHTML document [XHTML], then an SVGDocument object will not exist; instead,
 * the root object in the document object hierarchy will be a Document object of
 * a different type, such as an HTMLDocument object.
 *
 * However, an SVGDocument object will indeed exist when the root element of the
 * XML document hierarchy is an 'svg' element, such as when viewing a stand-alone
 * SVG file(i.e., a file with MIME type "image/svg+xml"). In this case, the
 * SVGDocument object will be the root object of the document object model
 * hierarchy.
 *
 * In the case where an SVG document is embedded by reference, such as when an
 * XHTML document has an 'object' element whose href attribute references an SVG
 * document(i.e., a document whose MIME type is "image/svg+xml" and whose root
 * element is thus an 'svg' element), there will exist two distinct DOM
 * hierarchies. The first DOM hierarchy will be for the referencing document
 *(e.g., an XHTML document). The second DOM hierarchy will be for the referenced
 * SVG document. In this second DOM hierarchy, the root object of the document
 * object model hierarchy is an SVGDocument object.
 */
class SVGDocument : public Document,
                    public events::DocumentEvent
{
public:


    /**
     * The title of a document as specified by the title sub-element of the 'svg'
     * root element(i.e., <svg><title>Here is the title</title>...</svg>)
     */
    DOMString getTitle();

    /**
     * Returns the URI of the page that linked to this page. The value is an empty
     * string if the user navigated to the page directly(not through a link, but,
     * for example, via a bookmark).
     */
    DOMString getReferrer();

    /**
     * The domain name of the server that served the document, or a null string if
     * the server cannot be identified by a domain name.
     */
    DOMString getDomain();

    /**
     * The complete URI of the document.
     */
    DOMString getURL();

    /**
     * The root 'svg'  element in the document hierarchy.
     */
    SVGElementPtr getRootElement();


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ~SVGDocument() {}

};



/*#########################################################################
## GetSVGDocument
#########################################################################*/

/**
 * In the case where an SVG document is embedded by reference, such as when an
 * XHTML document has an 'object' element whose href(or equivalent) attribute
 * references an SVG document(i.e., a document whose MIME type is
 * "image/svg+xml" and whose root element is thus an 'svg' element), the SVG user
 * agent is required to implement the GetSVGDocument interface for the element
 * which references the SVG document(e.g., the HTML 'object' or comparable
 * referencing elements).
 */
class GetSVGDocument
{
public:

    /**
     * Returns the SVGDocument  object for the referenced SVG document.
     */
    SVGDocumentPtr getSVGDocument()
                    throw(DOMException);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ~GetSVGDocument() {}

};







}  //namespace svg
}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif // __SVG_H__
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

