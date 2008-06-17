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

#include "svg.h"

#include <math.h>


namespace org
{
namespace w3c
{
namespace dom
{
namespace svg
{



//########################################################################
//########################################################################
//########################################################################
//#   I N T E R F A C E S
//########################################################################
//########################################################################
//########################################################################



/*#########################################################################
## SVGMatrix
#########################################################################*/

/**
 *
 */
double SVGMatrix::getA()
{ 
    return a;
}

/**
 *
 */
void SVGMatrix::setA(double val) throw (DOMException)
{ 
    a = val;
}

/**
 *
 */
double SVGMatrix::getB()
{
    return b;
}

/**
 *
 */
void SVGMatrix::setB(double val) throw (DOMException)
{
    b = val;
}

/**
 *
 */
double SVGMatrix::getC()
{
    return c;
}

/**
 *
 */
void SVGMatrix::setC(double val) throw (DOMException)
{
    c = val;
}

/**
 *
 */
double SVGMatrix::getD()
{
    return d;
}

/**
 *
 */
void SVGMatrix::setD(double val) throw (DOMException)
{
    d = val;
}

/**
 *
 */
double SVGMatrix::getE()
{
    return e;
}

/**
 *
 */
void SVGMatrix::setE(double val) throw (DOMException)
{
    e = val;
}

/**
 *
 */
double SVGMatrix::getF()
{
    return f;
}

/**
 *
 */
void SVGMatrix::setF(double val) throw (DOMException)
{
    f = val;
}


/**
 * Return the result of postmultiplying this matrix with another.
 */
SVGMatrix SVGMatrix::multiply(const SVGMatrix &other)
{
    SVGMatrix result;
    result.a = a * other.a  +  c * other.b;
    result.b = b * other.a  +  d * other.b;
    result.c = a * other.c  +  c * other.d;
    result.d = b * other.c  +  d * other.d;
    result.e = a * other.e  +  c * other.f  +  e;
    result.f = b * other.e  +  d * other.f  +  f;
    return result;
}

/**
 *  Calculate the inverse of this matrix
 *
 */
SVGMatrix SVGMatrix::inverse() throw (SVGException)
{
    /*###########################################
    The determinant of a 3x3 matrix E
       (let's use our own notation for a bit)

        A  B  C
        D  E  F
        G  H  I
    is
        AEI - AFH - BDI + BFG + CDH - CEG

    Since in our affine transforms, G and H==0 and I==1,
    this reduces to:
        AE - BD
    In SVG's naming scheme, that is:  a * d - c * b .  SIMPLE!

    In a similar method of attack, SVG's adjunct matrix is:

       d  -c   cf-ed
      -b   a   eb-af
       0   0   ad-cb

    To get the inverse matrix, we divide the adjunct matrix by
    the determinant.  Notice that (ad-cb)/(ad-cb)==1.  Very cool.
    So what we end up with is this:

       a =  d/(ad-cb)  c = -c/(ad-cb)   e = (cf-ed)/(ad-cb)
       b = -b/(ad-cb)  d =  a/(ad-cb)   f = (eb-af)/(ad-cb)

    (Since this would be in all SVG-DOM implementations,
     somebody needed to document this!  ^^)
    #############################################*/

    SVGMatrix result;
    double determinant = a * d  -  c * b;
    if (determinant < 1.0e-18)//invertible?
        {
        result.identity();//cop out
        return result;
        }

    double idet = 1.0 / determinant;
    result.a =   d * idet;
    result.b =  -b * idet;
    result.c =  -c * idet;
    result.d =   a * idet;
    result.e =  (c*f - e*d) * idet;
    result.f =  (e*b - a*f) * idet;
    return result;
}

/**
 * Equivalent to multiplying by:
 *  | 1  0  x |
 *  | 0  1  y |
 *  | 0  0  1 |
 *
 */
SVGMatrix SVGMatrix::translate(double x, double y)
{
    SVGMatrix result;
    result.a = a;
    result.b = b;
    result.c = c;
    result.d = d;
    result.e = a * x  +  c * y  +  e;
    result.f = b * x  +  d * y  +  f;
    return result;
}

/**
 * Equivalent to multiplying by:
 *  | scale  0      0 |
 *  | 0      scale  0 |
 *  | 0      0      1 |
 *
 */
:SVGMatrix SVGMatrix:scale(double scale)
{
    SVGMatrix result;
    result.a = a * scale;
    result.b = b * scale;
    result.c = c * scale;
    result.d = d * scale;
    result.e = e;
    result.f = f;
    return result;
}

/**
 * Equivalent to multiplying by:
 *  | scaleX  0       0 |
 *  | 0       scaleY  0 |
 *  | 0       0       1 |
 *
 */
SVGMatrix SVGMatrix::scaleNonUniform(double scaleX,
                                  double scaleY)
{
    SVGMatrix result;
    result.a = a * scaleX;
    result.b = b * scaleX;
    result.c = c * scaleY;
    result.d = d * scaleY;
    result.e = e;
    result.f = f;
    return result;
}

/**
 * Equivalent to multiplying by:
 *  | cos(a) -sin(a)   0 |
 *  | sin(a)  cos(a)   0 |
 *  | 0       0        1 |
 *
 */
SVGMatrix SVGMatrix::rotate (double angle)
{
    double sina  = sin(angle);
    double msina = -sina;
    double cosa  = cos(angle);
    SVGMatrix result;
    result.a = a * cosa   +  c * sina;
    result.b = b * cosa   +  d + sina;
    result.c = a * msina  +  c * cosa;
    result.d = b * msina  +  d * cosa;
    result.e = e;
    result.f = f;
    return result;
}

/**
 * Equivalent to multiplying by:
 *  | cos(a) -sin(a)   0 |
 *  | sin(a)  cos(a)   0 |
 *  | 0       0        1 |
 *  In this case, angle 'a' is computed as the artangent
 *  of the slope y/x .  It is negative if the slope is negative.
 */
SVGMatrix SVGMatrix::rotateFromVector(double x, double y)
                                  throw (SVGException)
{
    double angle = atan(y / x);
    if (y < 0.0)
        angle = -angle;
    SVGMatrix result;
    double sina  = sin(angle);
    double msina = -sina;
    double cosa  = cos(angle);
    result.a = a * cosa   +  c * sina;
    result.b = b * cosa   +  d + sina;
    result.c = a * msina  +  c * cosa;
    result.d = b * msina  +  d * cosa;
    result.e = e;
    result.f = f;
    return result;
}

/**
 * Equivalent to multiplying by:
 *  | -1   0   0 |
 *  | 0    1   0 |
 *  | 0    0   1 |
 *
 */
SVGMatrix SVGMatrix::flipX()
{
    SVGMatrix result;
    result.a = -a;
    result.b = -b;
    result.c =  c;
    result.d =  d;
    result.e =  e;
    result.f =  f;
    return result;
}

/**
 * Equivalent to multiplying by:
 *  | 1   0   0 |
 *  | 0  -1   0 |
 *  | 0   0   1 |
 *
 */
SVGMatrix SVGMatrix::flipY()
{
    SVGMatrix result;
    result.a =  a;
    result.b =  b;
    result.c = -c;
    result.d = -d;
    result.e =  e;
    result.f =  f;
    return result;
}

/**
 *  | 1   tan(a)  0 |
 *  | 0   1       0 |
 *  | 0   0       1 |
 *
 */
SVGMatrix SVGMatrix::skewX(double angle)
{
    double tana = tan(angle);
    SVGMatrix result;
    result.a =  a;
    result.b =  b;
    result.c =  a * tana + c;
    result.d =  b * tana + d;
    result.e =  e;
    result.f =  f;
    return result;
}

/**
 * Equivalent to multiplying by:
 *  | 1       0   0 |
 *  | tan(a)  1   0 |
 *  | 0       0   1 |
 *
 */
SVGMatrix::SVGMatrix SVGMatrix::skewY(double angle)
{
    double tana = tan(angle);
    SVGMatrix result;
    result.a =  a + c * tana;
    result.b =  b + d * tana;
    result.c =  c;
    result.d =  d;
    result.e =  e;
    result.f =  f;
    return result;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGMatrix::SVGMatrix()
{
    identity();
}

/**
 *
 */
SVGMatrix::SVGMatrix(double aArg, double bArg, double cArg,
          double dArg, double eArg, double fArg)
{
    a = aArg; b = bArg; c = cArg;
    d = dArg; e = eArg; f = fArg;
}

/**
 * Copy constructor
 */
SVGMatrix::SVGMatrix(const SVGMatrix &other)
{
    a = other.a;
    b = other.b;
    c = other.c;
    d = other.d;
    e = other.e;
    f = other.f;
}



/**
 *
 */
SVGMatrix::~SVGMatrix()
{
}

/*
 * Set to the identity matrix
 */
void SVGMatrix::identity()
{
    a = 1.0;
    b = 0.0;
    c = 0.0;
    d = 1.0;
    e = 0.0;
    f = 0.0;
}


/*#########################################################################
## SVGTransform
#########################################################################*/

/**
 *
 */
unsigned short SVGTransform::getType()
{
    return type;
}


/**
 *
 */
SVGMatrix SVGTransform::getMatrix()
{
    return matrix;
}

/**
 *
 */
double SVGTransform::getAngle()
{
    return angle;
}


/**
 *
 */
void SVGTransform::setMatrix(const SVGMatrix &matrixArg)
{
    type = SVG_TRANSFORM_MATRIX;
    matrix = matrixArg;
}

/**
 *
 */
void SVGTransform::setTranslate(double tx, double ty)
{
    type = SVG_TRANSFORM_TRANSLATE;
    matrix.setA(1.0);
    matrix.setB(0.0);
    matrix.setC(0.0);
    matrix.setD(1.0);
    matrix.setE(tx);
    matrix.setF(ty);
}

/**
 *
 */
void SVGTransform::setScale(double sx, double sy)
{
    type = SVG_TRANSFORM_SCALE;
    matrix.setA(sx);
    matrix.setB(0.0);
    matrix.setC(0.0);
    matrix.setD(sy);
    matrix.setE(0.0);
    matrix.setF(0.0);
}

/**
 *
 */
void SVGTransform::setRotate(double angleArg, double cx, double cy)
{
    angle = angleArg;
    setTranslate(cx, cy);
    type = SVG_TRANSFORM_ROTATE;
    matrix.rotate(angle);
}

/**
 *
 */
void SVGTransform::setSkewX(double angleArg)
{
    angle = angleArg;
    type = SVG_TRANSFORM_SKEWX;
    matrix.identity();
    matrix.skewX(angle);
}

/**
 *
 */
void SVGTransform::setSkewY(double angleArg)
{
    angle = angleArg;
    type = SVG_TRANSFORM_SKEWY;
    matrix.identity();
    matrix.skewY(angle);
}


//##################
//# Non-API methods
//##################

/**
 *
 */
SVGTransform::SVGTransform()
{
    type = SVG_TRANSFORM_UNKNOWN;
    angle = 0.0;
}

/**
 *
 */
SVGTransform::SVGTransform(const SVGTransform &other)
{
    type   = other.type;
    angle  = other.angle;
    matrix = other.matrix;
}

/**
 *
 */
~SVGTransform::SVGTransform()
{
}



/*#########################################################################
## SVGNumber
#########################################################################*/

/**
 *
 */
double SVGNumber::getValue()
{
    return value;
}

/**
 *
 */
void SVGNumber::setValue(double val) throw (DOMException)
{
    value = val;
}


//##################
//# Non-API methods
//##################

/**
 *
 */
SVGNumber::SVGNumber()
{
    value = 0.0;
}

/**
 *
 */
SVGNumber::SVGNumber(const SVGNumber &other)
{
    value = other.value;
}

/**
 *
 */
SVGNumber::~SVGNumber()
{
}



/*#########################################################################
## SVGLength
#########################################################################*/


/**
 *
 */
unsigned short SVGLength::getUnitType()
{
    return unitType;
}

/**
 *
 */
double SVGLength::getValue()
{
    return value;
}

/**
 *
 */
void SVGLength::setValue(double val) throw (DOMException)
{
    value = val;
}

/**
 *
 */
double SVGLength::getValueInSpecifiedUnits()
{
    double result = 0.0;
    //fill this in
    return result;
}

/**
 *
 */
void SVGLength::setValueInSpecifiedUnits(double /*val*/)
                                       throw (DOMException)
{
    //fill this in
}

/**
 *
 */
DOMString SVGLength::getValueAsString()
{
    DOMString ret;
    char buf[32];
    snprintf(buf, 31, "%f", value);
    ret.append(buf);
    return ret;
}

/**
 *
 */
void SVGLength::setValueAsString(const DOMString& /*val*/)
                               throw (DOMException)
{
}


/**
 *
 */
void SVGLength::newValueSpecifiedUnits (unsigned short /*unitType*/, double /*val*/)
{
}

/**
 *
 */
void SVGLength::convertToSpecifiedUnits (unsigned short /*unitType*/)
{
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGLength::SVGLength()
{
    unitType = SVG_LENGTHTYPE_UNKNOWN;
    value    = 0.0;
}


/**
 *
 */
SVGLength::SVGLength(const SVGLength &other)
{
    unitType  = other.unitType;
    value     = other.value;
}

/**
 *
 */
SVGLength::~SVGLength()
{
}




/*#########################################################################
## SVGAngle
#########################################################################*/

/**
 *
 */
unsigned short SVGAngle::getUnitType()
{
    return unitType;
}

/**
 *
 */
double SVGAngle::getValue()
{
    return value;
}

/**
 *
 */
void SVGAngle::setValue(double val) throw (DOMException)
{
    value = val;
}

/**
 *
 */
double SVGAngle::getValueInSpecifiedUnits()
{
    double result = 0.0;
    //convert here
    return result;
}

/**
 *
 */
void SVGAngle::setValueInSpecifiedUnits(double /*val*/)
                                        throw (DOMException)
{
    //do conversion
}

/**
 *
 */
DOMString SVGAngle::getValueAsString()
{
    DOMString result;
    char buf[32];
    snprintf(buf, 31, "%f", value);
    result.append(buf);
    return result;
}

/**
 *
 */
void SVGAngle::setValueAsString(const DOMString &/*val*/)
                                throw (DOMException)
{
    //convert here
}


/**
 *
 */
void SVGAngle::newValueSpecifiedUnits (unsigned short /*unitType*/,
                                       double /*valueInSpecifiedUnits*/)
{
    //convert here
}

/**
 *
 */
void SVGAngle::convertToSpecifiedUnits (unsigned short /*unitType*/)
{
    //convert here
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGAngle::SVGAngle()
{
    unitType = SVG_ANGLETYPE_UNKNOWN;
    value    = 0.0;
}

/**
 *
 */
SVGAngle::SVGAngle(const SVGAngle &other)
{
    unitType = other.unitType;
    value    = other.value;
}

/**
 *
 */
SVGAngle::~SVGAngle()
{
}




/*#########################################################################
## SVGICCColor
#########################################################################*/


/**
 *
 */
DOMString SVGICCColor::getColorProfile()
{
    return colorProfile;
}

/**
 *
 */
void SVGICCColor::setColorProfile(const DOMString &val) throw (DOMException)
{
    colorProfile = val;
}

/**
 *
 */
SVGNumberList &SVGICCColor::getColors()
{
    return colors;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGICCColor::SVGICCColor()
{
}

/**
 *
 */
SVGICCColor::SVGICCColor(const SVGICCColor &other)
{
    colorProfile = other.colorProfile;
    colors       = other.colors;
}

/**
 *
 */
SVGICCColor::~SVGICCColor()
{
}



/*#########################################################################
## SVGColor
#########################################################################*/



/**
 *
 */
unsigned short SVGColor::getColorType()
{
    return colorType;
}

/**
 *
 */
css::RGBColor SVGColor::getRgbColor()
{
    css::RGBColor col;
    return col;
}

/**
 *
 */
SVGICCColor SVGColor::getIccColor()
{
    SVGICCColor col;
    return col;
}


/**
 *
 */
void SVGColor::setRGBColor(const DOMString& /*rgbColor*/)
                           throw (SVGException)
{
}

/**
 *
 */
void SVGColor::setRGBColorICCColor(const DOMString& /*rgbColor*/,
                                   const DOMString& /*iccColor*/)
                                   throw (SVGException)
{
}

/**
 *
 */
void SVGColor::setColor (unsigned short /*colorType*/,
                         const DOMString& /*rgbColor*/,
                         const DOMString& /*iccColor*/)
                         throw (SVGException)
{
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGColor::SVGColor()
{
    colorType = SVG_COLORTYPE_UNKNOWN;
}

/**
 *
 */
SVGColor::SVGColor(const SVGColor &other) : css::CSSValue(other)
{
    colorType = other.colorType;
}

/**
 *
 */
SVGColor::~SVGColor()
{
}



/*#########################################################################
## SVGRect
#########################################################################*/


/**
 *
 */
double SVGRect::getX()
{
    return x;
}

/**
 *
 */
void SVGRect::setX(double val) throw (DOMException)
{
    x = val;
}

/**
 *
 */
double SVGRect::getY()
{
    return y;
}

/**
 *
 */
void SVGRect::setY(double val) throw (DOMException)
{
    y = val;
}

/**
 *
 */
double SVGRect::getWidth()
{
    return width;
}

/**
 *
 */
void SVGRect::setWidth(double val) throw (DOMException)
{
    width = val;
}

/**
 *
 */
double SVGRect::getHeight()
{
    return height;
}

/**
 *
 */
void SVGRect::setHeight(double val) throw (DOMException)
{
    height = val;
}


//##################
//# Non-API methods
//##################

/**
 *
 */
SVGRect::SVGRect()
{
    x = y = width = height = 0.0;
}

/**
 *
 */
SVGRect::SVGRect(const SVGRect &other)
{
    x = other.x;
    y = other.y;
    width = other.width;
    height = other.height;
}

/**
 *
 */
SVGRect::~SVGRect()
{
}



/*#########################################################################
## SVGPoint
#########################################################################*/


/**
 *
 */
double SVGPoint::getX()
{
    return x;
}

/**
 *
 */
void SVGPoint::setX(double val) throw (DOMException)
{
    x = val;
}

/**
 *
 */
double SVGPoint::getY()
{
    return y;
}

/**
 *
 */
void SVGPoint::setY(double val) throw (DOMException)
{
    y = val;
}

/**
 *
 */
SVGPoint SVGPoint::matrixTransform(const SVGMatrix &/*matrix*/)
{
    SVGPoint point;
    return point;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGPoint::SVGPoint()
{
    x = y = 0;
}

/**
 *
 */
SVGPoint::SVGPoint(const SVGPoint &other)
{
    x = other.x;
    y = other.y;
}

/**
 *
 */
SVGPoint::~SVGPoint()
{
}


/*#########################################################################
## SVGUnitTypes
#########################################################################*/

/**
 *
 */
SVGUnitTypes::SVGUnitTypes()
{
}



/**
 *
 */
SVGUnitTypes::~SVGUnitTypes()
{
}


/*#########################################################################
## SVGStylable
#########################################################################*/


/**
 *
 */
SVGAnimatedString SVGStylable::getClassName()
{
    return className;
}

/**
 *
 */
css::CSSStyleDeclaration SVGStylable::getStyle()
{
    return style;
}


/**
 *
 */
css::CSSValue SVGStylable::getPresentationAttribute(const DOMString& /*name*/)
{
    css::CSSValue val;
    //perform a lookup
    return val;
}


//##################
//# Non-API methods
//##################

/**
 *
 */
SVGStylable::SVGStylable()
{
}

/**
 *
 */
SVGStylable::SVGStylable(const SVGStylable &other)
{
    className = other.className;
    style     = other.style;
}

/**
 *
 */
SVGStylable::~SVGStylable()
{
}




/*#########################################################################
## SVGLocatable
#########################################################################*/


/**
 *
 */
SVGElementPtr SVGLocatable::getNearestViewportElement()
{
    SVGElementPtr result;
    return result;
}

/**
 *
 */
SVGElementPtr SVGLocatable::getFarthestViewportElement()
{
    SVGElementPtr result;
    return result;
}

/**
 *
 */
SVGRect SVGLocatable::getBBox ()
{
    return bbox;
}

/**
 *
 */
SVGMatrix SVGLocatable::getCTM ()
{
    return ctm;
}

/**
 *
 */
SVGMatrix SVGLocatable::getScreenCTM ()
{
    return screenCtm;
}

/**
 *
 */
SVGMatrix SVGLocatable::getTransformToElement (const SVGElement &/*element*/)
                throw (SVGException)
{
    SVGMatrix result;
    //do calculations
    return result;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGLocatable::SVGLocatable()
{
}

/**
 *
 */
SVGLocatable::SVGLocatable(const SVGLocatable &/*other*/)
{
}

/**
 *
 */
SVGLocatable::~SVGLocatable()
{
}


/*#########################################################################
## SVGTransformable
#########################################################################*/


/**
 *
 */
SVGAnimatedTransformList &SVGTransformable::getTransform()
{
    return transforms;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGTransformable::SVGTransformable() {}

/**
 *
 */
SVGTransformable::SVGTransformable(const SVGTransformable &other) : SVGLocatable(other)
{
    transforms = other.transforms;
}

/**
 *
 */
SVGTransformable::~SVGTransformable()
{
}







/*#########################################################################
## SVGTests
#########################################################################*/


/**
 *
 */
SVGStringList &SVGTests::getRequiredFeatures()
{
    return requiredFeatures;
}

/**
 *
 */
SVGStringList &SVGTests::getRequiredExtensions()
{
    return requiredExtensions;
}

/**
 *
 */
SVGStringList &SVGTests::getSystemLanguage()
{
    return systemLanguage;
}


/**
 *
 */
bool SVGTests::hasExtension (const DOMString& /*extension*/)
{
    return false;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGTests::SVGTests()
{
}

/**
 *
 */
SVGTests::SVGTests(const SVGTests &other)
{
    requiredFeatures   = other.requiredFeatures;
    requiredExtensions = other.requiredExtensions;
    systemLanguage     = other.systemLanguage;
}

/**
 *
 */
SVGTests::~SVGTests()
{
}



/*#########################################################################
## SVGLangSpace
#########################################################################*/


/**
 *
 */
DOMString SVGLangSpace::getXmllang()
{
    return xmlLang;
}

/**
 *
 */
void SVGLangSpace::setXmllang(const DOMString &val) throw (DOMException)
{
    xmlLang = val;
}

/**
 *
 */
DOMString SVGLangSpace::getXmlspace()
{
    return xmlSpace;
}

/**
 *
 */
void SVGLangSpace::setXmlspace(const DOMString &val)
                                 throw (DOMException)
{
    xmlSpace = val;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGLangSpace::SVGLangSpace()
{
}

/**
 *
 */
SVGLangSpace::SVGLangSpace(const SVGLangSpace &other)
{
    xmlLang  = other.xmlLang;
    xmlSpace = other.xmlSpace;
}

/**
 *
 */
SVGLangSpace::~SVGLangSpace()
{
}



/*#########################################################################
## SVGExternalResourcesRequired
#########################################################################*/

/**
 *
 */
SVGAnimatedBoolean SVGExternalResourcesRequired::getExternalResourcesRequired()
{
    return required;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGExternalResourcesRequired::SVGExternalResourcesRequired()
{
}


/**
 *
 */
SVGExternalResourcesRequired::SVGExternalResourcesRequired(
               const SVGExternalResourcesRequired &other)
{
    required = other.required;
}

/**
 *
 */
SVGExternalResourcesRequired::~SVGExternalResourcesRequired() {}


/*#########################################################################
## SVGPreserveAspectRatio
#########################################################################*/

/**
 *
 */
unsigned short SVGPreserveAspectRatio::getAlign()
{
    return align;
}

/**
 *
 */
void SVGPreserveAspectRatio::setAlign(unsigned short val) throw (DOMException)
{
    align = val;
}

/**
 *
 */
unsigned short SVGPreserveAspectRatio::getMeetOrSlice()
{
    return meetOrSlice;
}

/**
 *
 */
void SVGPreserveAspectRatio::setMeetOrSlice(unsigned short val) throw (DOMException)
{
    meetOrSlice = val;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGPreserveAspectRatio::SVGPreserveAspectRatio()
{
    align       = SVG_PRESERVEASPECTRATIO_UNKNOWN;
    meetOrSlice = SVG_MEETORSLICE_UNKNOWN;
}

/**
 *
 */
SVGPreserveAspectRatio::SVGPreserveAspectRatio(const SVGPreserveAspectRatio &other)
{
    align       = other.align;
    meetOrSlice = other.meetOrSlice;
}

/**
 *
 */
SVGPreserveAspectRatio::~SVGPreserveAspectRatio()
{
}



/*#########################################################################
## SVGFitToViewBox
#########################################################################*/


/**
 *
 */
SVGAnimatedRect SVGFitToViewBox::getViewBox()
{
    return viewBox;
}

/**
 *
 */
SVGAnimatedPreserveAspectRatio SVGFitToViewBox::getPreserveAspectRatio()
{
    return preserveAspectRatio;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGFitToViewBox::SVGFitToViewBox()
{
}

/**
 *
 */

SVGFitToViewBox::SVGFitToViewBox(const SVGFitToViewBox &other)
{
    viewBox = other.viewBox;
    preserveAspectRatio = other.preserveAspectRatio;
}

/**
 *
 */
SVGFitToViewBox::~SVGFitToViewBox()
{
}

/*#########################################################################
## SVGZoomAndPan
#########################################################################*/

/**
 *
 */
unsigned short SVGZoomAndPan::getZoomAndPan()
{
    return zoomAndPan;
}

/**
 *
 */
void SVGZoomAndPan::setZoomAndPan(unsigned short val) throw (DOMException)
{
    zoomAndPan = val;
}


//##################
//# Non-API methods
//##################

/**
 *
 */
SVGZoomAndPan::SVGZoomAndPan()
{
    zoomAndPan = SVG_ZOOMANDPAN_UNKNOWN;
}

/**
 *
 */
SVGZoomAndPan::SVGZoomAndPan(const SVGZoomAndPan &other)
{
    zoomAndPan = other.zoomAndPan;
}

/**
 *
 */
SVGZoomAndPan::~SVGZoomAndPan()
{
}


/*#########################################################################
## SVGViewSpec
#########################################################################*/

/**
 *
 */
SVGTransformList SVGViewSpec::getTransform()
{
    return transform;
}

/**
 *
 */
SVGElementPtr SVGViewSpec::getViewTarget()
{
    return viewTarget;
}

/**
 *
 */
DOMString SVGViewSpec::getViewBoxString()
{
    DOMString ret;
    return ret;
}

/**
 *
 */
DOMString SVGViewSpec::getPreserveAspectRatioString()
{
    DOMString ret;
    return ret;
}

/**
 *
 */
DOMString SVGViewSpec::getTransformString()
{
    DOMString ret;
    return ret;
}

/**
 *
 */
DOMString SVGViewSpec::getViewTargetString()
{
    DOMString ret;
    return ret;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGViewSpec::SVGViewSpec()
{
    viewTarget = NULL;
}

/**
 *
 */
SVGViewSpec::SVGViewSpec(const SVGViewSpec &other) : SVGZoomAndPan(other), SVGFitToViewBox(other)
{
    viewTarget = other.viewTarget;
    transform  = other.transform;
}

/**
 *
 */
SVGViewSpec::~SVGViewSpec()
{
}



/*#########################################################################
## SVGURIReference
#########################################################################*/


/**
 *
 */
SVGAnimatedString SVGURIReference::getHref()
{
    return href;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGURIReference::SVGURIReference()
{
}

/**
 *
 */
SVGURIReference::SVGURIReference(const SVGURIReference &other)
{
    href = other.href;
}

/**
 *
 */
SVGURIReference::~SVGURIReference()
{
}



/*#########################################################################
## SVGCSSRule
#########################################################################*/




/*#########################################################################
## SVGRenderingIntent
#########################################################################*/





/*#########################################################################
## SVGPathSeg
#########################################################################*/

static const char *pathSegLetters[] =
{
    '@', // PATHSEG_UNKNOWN,
    'z', // PATHSEG_CLOSEPATH
    'M', // PATHSEG_MOVETO_ABS
    'm', // PATHSEG_MOVETO_REL,
    'L', // PATHSEG_LINETO_ABS
    'l', // PATHSEG_LINETO_REL
    'C', // PATHSEG_CURVETO_CUBIC_ABS
    'c', // PATHSEG_CURVETO_CUBIC_REL
    'Q', // PATHSEG_CURVETO_QUADRATIC_ABS,
    'q', // PATHSEG_CURVETO_QUADRATIC_REL
    'A', // PATHSEG_ARC_ABS
    'a', // PATHSEG_ARC_REL,
    'H', // PATHSEG_LINETO_HORIZONTAL_ABS,
    'h', // PATHSEG_LINETO_HORIZONTAL_REL
    'V', // PATHSEG_LINETO_VERTICAL_ABS
    'v', // PATHSEG_LINETO_VERTICAL_REL
    'S', // PATHSEG_CURVETO_CUBIC_SMOOTH_ABS
    's', // PATHSEG_CURVETO_CUBIC_SMOOTH_REL
    'T', // PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS
    't'  // PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL
};



/**
 *
 */
unsigned short getPathSegType()
{
    return type;
}

/**
 *
 */
DOMString getPathSegTypeAsLetter()
{
    int typ = type;
    if (typ<0 || typ>PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL)
        typ = PATHSEG_UNKNOWN;
    char const ch = pathSegLetters[typ];
    DOMString letter = ch;
    return letter;
}


/**
 *
 */
unsigned short getPathSegType()
{
    return type;
}

/**
 *
 */
DOMString getPathSegTypeAsLetter()
{
    int typ = type;
    if (typ<0 || typ>PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL)
        typ = PATHSEG_UNKNOWN;
    char const *ch = pathSegLetters[typ];
    DOMString letter = ch;
    return letter;
}

/**
 * From the various subclasses
 */

/**
 *
 */
double SVGPathSeg::getX()
{
    return x;
}

/**
 *
 */
void SVGPathSeg::setX(double val) throw (DOMException)
{
    x = val;
}

/**
 *
 */
double SVGPathSeg::getX1()
{
    return x;
}

/**
 *
 */
void SVGPathSeg::setX1(double val) throw (DOMException)
{
    x = val;
}

/**
 *
 */
double SVGPathSeg::getX2()
{
    return x;
}

/**
 *
 */
void SVGPathSeg::setX2(double val) throw (DOMException)
{
    x = val;
}

/**
 *
 */
double SVGPathSeg::getY()
{
    return y;
}

/**
 *
 */
void SVGPathSeg::setY(double val) throw (DOMException)
{
    y = val;
}

/**
 *
 */
double SVGPathSeg::getY1()
{
    return y;
}

/**
 *
 */
void SVGPathSeg::setY1(double val) throw (DOMException)
{
    y = val;
}

/**
 *
 */
double SVGPathSeg::getY2()
{
    return y;
}

/**
 *
 */
void SVGPathSeg::setY2(double val) throw (DOMException)
{
    y = val;
}

/**
 *
 */
double SVGPathSeg::getR1()
{
    return r1;
}

/**
 *
 */
void SVGPathSeg::setR1(double val) throw (DOMException)
{
    r1 = val;
}

/**
 *
 */
double SVGPathSeg::getR2()
{
    return r2;
}

/**
 *
 */
void SVGPathSeg::setR2(double val) throw (DOMException)
{
    r2 = val;
}

/**
 *
 */
double SVGPathSeg::getAngle()
{
    return angle;
}

/**
 *
 */
void SVGPathSeg::setAngle(double val) throw (DOMException)
{
    angle = val;
}

/**
 *
 */
bool SVGPathSeg::getLargeArcFlag()
{
    return largeArcFlag;
}

/**
 *
 */
void SVGPathSeg::setLargeArcFlag(bool val) throw (DOMException)
{
    largeArcFlag = val;
}

/**
 *
 */
bool SVGPathSeg::getSweepFlag()
{
    return sweepFlag;
}

/**
 *
 */
void SVGPathSeg::setSweepFlag(bool val) throw (DOMException)
{
    sweepFlag = val;
}




//##################
//# Non-API methods
//##################

/**
 *
 */
SVGPathSeg::SVGPathSeg()
{
    init();
}

/**
 *
 */
SVGPathSeg::SVGPathSeg(const SVGPathSeg &other)
{
    assign(other);
}

/**
 *
 */
SVGPathSeg &operator=(const SVGPathSeg &other)
{
    assign(other);
    return *this;
}

/**
 *
 */
void SVGPathSeg::init()
{
    type = PATHSEG_UNKNOWN;
    x = y = x1 = y1 = x2 = y2 = 0.0;
    r1 = r2 = 0.0;
    angle = 0.0;
    largeArcFlag = false;
    sweepFlag    = false;
}
    
/**
 *
 */
void SVGPathSeg::assign(const SVGPathSeg &other)
{
    type         = other.type;
    x            = other.x;
    y            = other.y;
    x1           = other.x1;
    y1           = other.y1;
    x2           = other.x2;
    y2           = other.y2;
    r1           = other.r1;
    r2           = other.r2;
    angle        = other.angle;
    largeArcFlag = other.largeArcFlag;
    sweepFlag    = other.sweepFlag;
}


/**
 *
 */
SVGPathSeg::~SVGPathSeg()
{
}




/*#########################################################################
## SVGPaint
#########################################################################*/


/**
 *
 */
unsigned short SVGPaint::getPaintType()
{ return paintType; }

/**
 *
 */
DOMString SVGPaint::getUri()
{ return uri; }

/**
 *
 */
void SVGPaint::setUri(const DOMString& uriArg)
{
    uri = uriArg;
}

/**
 *
 */
void SVGPaint::setPaint (unsigned short paintTypeArg,
                         const DOMString& uriArg,
                         const DOMString& /*rgbColor*/,
                         const DOMString& /*iccColor*/)
                         throw (SVGException)
{
    paintType = paintTypeArg;
    uri       = uriArg;
    //do something with rgbColor
    //do something with iccColor;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGPaint::SVGPaint()
{
    uri       = "";
    paintType = SVG_PAINTTYPE_UNKNOWN;
}

/**
 *
 */
SVGPaint::SVGPaint(const SVGPaint &other) : css::CSSValue(other), SVGColor(other)
{
    uri       = "";
    paintType = SVG_PAINTTYPE_UNKNOWN;
}

/**
 *
 */
SVGPaint::~SVGPaint() {}


/*#########################################################################
## SVGColorProfileRule
#########################################################################*/


/**
 *
 */
DOMString SVGColorProfileRule::getSrc()
{ return src; }

/**
 *
 */
void SVGColorProfileRule::setSrc(const DOMString &val) throw (DOMException)
{ src = val; }

/**
 *
 */
DOMString SVGColorProfileRule::getName()
{ return name; }

/**
 *
 */
void SVGColorProfileRule::setName(const DOMString &val) throw (DOMException)
{ name = val; }

/**
 *
 */
unsigned short SVGColorProfileRule::getRenderingIntent()
{ return renderingIntent; }

/**
 *
 */
void SVGColorProfileRule::setRenderingIntent(unsigned short val) throw (DOMException)
{ renderingIntent = val; }


//##################
//# Non-API methods
//##################

/**
 *
 */
SVGColorProfileRule::SVGColorProfileRule()
{
}

/**
 *
 */
SVGColorProfileRule::SVGColorProfileRule(const SVGColorProfileRule &other)
           : SVGCSSRule(other), SVGRenderingIntent(other)
{
    renderingIntent = other.renderingIntent;
    src             = other.src;
    name            = other.name;
}

/**
 *
 */
SVGColorProfileRule::~SVGColorProfileRule()
{
}


/*#########################################################################
## SVGFilterPrimitiveStandardAttributes
#########################################################################*/

/**
 *
 */
SVGAnimatedLength SVGFilterPrimitiveStandardAttributes::getX()
{ return x; }

/**
 *
 */
SVGAnimatedLength SVGFilterPrimitiveStandardAttributes::getY()
{ return y; }

/**
 *
 */
SVGAnimatedLength SVGFilterPrimitiveStandardAttributes::getWidth()
{ return width; }

/**
 *
 */
SVGAnimatedLength SVGFilterPrimitiveStandardAttributes::getHeight()
{ return height; }

/**
 *
 */
SVGAnimatedString SVGFilterPrimitiveStandardAttributes::getResult()
{ return result; }



//##################
//# Non-API methods
//##################


/**
 *
 */
SVGFilterPrimitiveStandardAttributes::SVGFilterPrimitiveStandardAttributes()
{
}

/**
 *
 */
SVGFilterPrimitiveStandardAttributes::SVGFilterPrimitiveStandardAttributes(
                   const SVGFilterPrimitiveStandardAttributes &other)
                             : SVGStylable(other)
{
    x      = other.x;
    y      = other.y;
    width  = other.width;
    height = other.height;
    result = other.result;
}

/**
 *
 */
SVGFilterPrimitiveStandardAttributes::~SVGFilterPrimitiveStandardAttributes()
{
}


/*#########################################################################
## SVGEvent
#########################################################################*/

/**
 *
 */
SVGEvent:SVGEvent()
{
}

/**
 *
 */
SVGEvent:SVGEvent(const SVGEvent &other) : events::Event(other)
{
}

/**
 *
 */
SVGEvent::~SVGEvent()
{
}


/*#########################################################################
## SVGZoomEvent
#########################################################################*/

/**
 *
 */
SVGRect SVGZoomEvent::getZoomRectScreen()
{
    return zoomRectScreen;
}

/**
 *
 */
double SVGZoomEvent::getPreviousScale()
{
    return previousScale;
}

/**
 *
 */
SVGPoint SVGZoomEvent::getPreviousTranslate()
{
    return previousTranslate;
}

/**
 *
 */
double SVGZoomEvent::getNewScale()
{
    return newScale;
}

/**
 *
 */
SVGPoint SVGZoomEvent::getNewTranslate()
{
    return newTranslate;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGZoomEvent::SVGZoomEvent()
{
}

/**
 *
 */
SVGZoomEvent::SVGZoomEvent(const SVGZoomEvent &other) :
                        events::Event(other), events::UIEvent(other)
{
    zoomRectScreen    = other.zoomRectScreen;
    previousScale     = other.previousScale;
    previousTranslate = other.previousTranslate;
    newScale          = other.newScale;
    newTranslate      = other.newTranslate;
}

/**
 *
 */
SVGZoomEvent::~SVGZoomEvent()
{
}


/*#########################################################################
## SVGElementInstance
#########################################################################*/


/**
 *
 */
SVGElementPtr SVGElementInstance::getCorrespondingElement()
{
    return correspondingElement;
}

/**
 *
 */
SVGUseElementPtr SVGElementInstance::getCorrespondingUseElement()
{
    return correspondingUseElement;
}

/**
 *
 */
SVGElementInstance SVGElementInstance::getParentNode()
{
    SVGElementInstance ret;
    return ret;
}

/**
 *  Since we are using stack types and this is a circular definition,
 *  we will instead implement this as a global function below:
 *   SVGElementInstanceList getChildNodes(const SVGElementInstance instance);
 */
//SVGElementInstanceList getChildNodes();

/**
 *
 */
SVGElementInstance SVGElementInstance::getFirstChild()
{
    SVGElementInstance ret;
    return ret;
}

/**
 *
 */
SVGElementInstance SVGElementInstance::getLastChild()
{
    SVGElementInstance ret;
    return ret;
}

/**
 *
 */
SVGElementInstance SVGElementInstance::getPreviousSibling()
{
    SVGElementInstance ret;
    return ret;
}

/**
 *
 */
SVGElementInstance SVGElementInstance::getNextSibling()
{
    SVGElementInstance ret;
    return ret;
}


//##################
//# Non-API methods
//##################

/**
 *
 */
SVGElementInstance::SVGElementInstance()
{
}

/**
 *
 */
SVGElementInstance::SVGElementInstance(const SVGElementInstance &other)
                    : events::EventTarget(other)
{
}

/**
 *
 */
SVGElementInstance::~SVGElementInstance()
{
}


/*#########################################################################
## SVGElementInstanceList
#########################################################################*/

/**
 *
 */
unsigned long SVGElementInstanceList::getLength()
{ return items.size(); }

/**
 *
 */
SVGElementInstance SVGElementInstanceList::item(unsigned long index)
{
    if (index >= items.size())
        {
        SVGElementInstance ret;
        return ret;
        }
    return items[index];
}

/**
 *  This static method replaces the circular definition of:
 *        SVGElementInstanceList SVGElementInstance::getChildNodes()
 *
 */
static SVGElementInstanceList SVGElementInstanceList::getChildNodes(const SVGElementInstance &/*instance*/)
{
    SVGElementInstanceList list;
    return list;
}


//##################
//# Non-API methods
//##################

/**
 *
 */
SVGElementInstanceList::SVGElementInstanceList()
{
}

/**
 *
 */
SVGElementInstanceList::SVGElementInstanceList(const SVGElementInstanceList &other)
{
    items = other.items;
}

/**
 *
 */
SVGElementInstanceList::~SVGElementInstanceList()
{
}




/*#########################################################################
## SVGValue
#########################################################################*/

/**
 * Constructor
 */
SVGValue()
{
    init();
}

/**
 * Copy constructor
 */
SVGValue(const SVGValue &other)
{
    assign(other);
}

/**
 * Assignment
 */
SVGValue &operator=(const SVGValue &other)
{
    assign(other);
    return *this;
}

/**
 *
 */
~SVGValue()
{
}

//###########################
//  TYPES
//###########################

/**
 *  Angle
 */
SVGValue::SVGValue(const SVGAngle &v)
{
   type = SVG_ANGLE;
   angleval = v;
}

SVGAngle SVGValue::angleValue()
{
    return algleval;
}

/**
 * Boolean
 */
SVGValue::SVGValue(bool v)
{
   type = SVG_BOOLEAN;
   bval = v;
}

bool SVGValue::booleanValue()
{
    return bval;
}


/**
 * Enumeration
 */
SVGValue::SVGValue(short v)
{
   type = SVG_ENUMERATION;
   eval = v;
}

short SVGValue::enumerationValue()
{
    return eval;
}

/**
 * Integer
 */
SVGValue::SVGValue(long v)
{
   type = SVG_INTEGER;
   ival = v;
}

long SVGValue::integerValue()
{
    return ival;
}

/**
 * Length
 */
SVGValue::SVGValue(const SVGLength &v)
{
   type = SVG_LENGTH;
   lengthval = v;
}

SVGLength SVGValue::lengthValue()
{
    return lengthval;
}

/**
 * Number
 */
SVGValue::SVGValue(double v)
{
   type = SVG_NUMBER;
   dval = v;
}

double SVGValue::numberValue()
{
    return dval;
}

/**
 * Points
 */
SVGValue::SVGValue(const SVGPointList &v)
{
   type = SVG_POINTS;
   plistval = v;
}

SVGPointList SVGValue::pointListValue()
{
    return plistval;
}


/**
 * PreserveAspectRatio
 */
SVGValue::SVGValue(const SVGPreserveAspectRatio &v)
{
   type = SVG_PRESERVE_ASPECT_RATIO;
   parval = v;
}

SVGPreserveAspectRatio SVGValue::preserveAspectRatioValue()
{
   return parval;
}

/**
 * Rect
 */
SVGValue::SVGValue(const SVGRect &v)
{
   type = SVG_RECT;
   rectval = v;
}

SVGRect SVGValue::rectValue()
{
    return rectval;
}

/**
 * String
 */
SVGValue::SVGValue(const DOMString &v)
{
   type = SVG_STRING;
   sval = v;
}

DOMString SVGValue::stringValue()
{
    return sval;
}


void SVGValue::init()
{
    type    = SVG_NUMBER;
    bval    = false;          
    eval    = 0;          
    ival    = 0;          
    dval    = 0.0;          
}

void SVGValue::assign(const SVGValue &other)
{
    type           = other.type; 
    angleval       = other.angleval;      
    bval           = other.bval;          
    eval           = other.eval;          
    ival           = other.ival;          
    lengthval      = other.lengthval;     
    dval           = other.dval;          
    parval         = other.parval;        
    rval           = other.rval;          
    sval           = other.sval;          
}


/*#########################################################################
## SVGTransformList
#########################################################################*/


/*#########################################################################
## SVGStringList
#########################################################################*/


/*#########################################################################
## SVGNumberList
#########################################################################*/


/*#########################################################################
## SVGLengthList
#########################################################################*/


/*#########################################################################
## SVGPointList
#########################################################################*/

/*#########################################################################
## SVGPathSegList
#########################################################################*/

/*#########################################################################
## SVGValueList
#########################################################################*/


/**
 *
 */
unsigned long SVGValueList::getNumberOfItems()
{
    return items.size();
}

/**
 *
 */
void SVGValueList::clear() throw (DOMException)
{
    items.clear();
}

/**
 *
 */
SVGValue SVGValueList::initialize(const SVGValue& newItem)
                throw (DOMException, SVGException)
{
    items.clear();
    items.push_back(newItem);
    return newItem;
}

/**
 *
 */
SVGValue SVGValueList::getItem(unsigned long index) throw (DOMException)
{
    if (index >= items.size())
        return "";
    return items[index];
}

/**
 *
 */
SVGValue SVGValueList::insertItemBefore(const SVGValue& newItem,
                                          unsigned long index)
                                          throw (DOMException, SVGException)
{
    if (index>=items.size())
        {
        items.push_back(newItem);
        }
    else
        {
        std::vector<SVGValue>::iterator iter = items.begin() + index;
        items.insert(iter, newItem);
        }
    return newItem;
}

/**
 *
 */
SVGValue SVGValueList::replaceItem (const SVGValue& newItem,
                                unsigned long index)
                            throw (DOMException, SVGException)
{
    if (index>=items.size())
        return "";
    std::vector<SVGValue>::iterator iter = items.begin() + index;
    *iter = newItem;
    return newItem;
}

/**
 *
 */
SVGValue SVGValueList::removeItem (unsigned long index)
                throw (DOMException)
{
    if (index>=items.size())
        return "";
    std::vector<SVGValue>::iterator iter = items.begin() + index;
    SVGValue oldval = *iter;
    items.erase(iter);
    return oldval;
}

/**
 *
 */
SVGValue SVGValueList::appendItem (const SVGValue& newItem)
                throw (DOMException, SVGException)
{
    items.push_back(newItem);
    return newItem;
}


/**
 * Matrix
 */
SVGValue SVGValueList::createSVGTransformFromMatrix(const SVGValue &matrix)
{
}

/**
 * Matrix
 */
SVGValue SVGValueList::consolidate()
{
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGValueList::SVGValueList()
{
}

/**
 *
 */
SVGValueList::SVGValueList(const SVGValueList &other)
{
    items = other.items;
}

/**
 *
 */
SVGValueList::~SVGValueList()
{
}





/*#########################################################################
## SVGAnimatedValue
#########################################################################*/




/**
 *
 */
SVGValue &SVGAnimatedValue::getBaseVal()
{
    return baseVal;
}

/**
 *
 */
void SVGAnimatedValue::setBaseVal(const SVGValue &val) throw (DOMException)
{
   baseVal = val;
}

/**
 *
 */
SVGValue &SVGAnimatedValue::getAnimVal()
{
    return animVal;
}


/**
 *
 */
SVGAnimatedValue::SVGAnimatedValue()
{
    init();
}


/**
 *
 */
SVGAnimatedValue::SVGAnimatedValue(const SVGValue &v)
{
    init();
    baseVal = v;
}


/**
 *
 */
SVGAnimatedValue::SVGAnimatedValue(const SVGValue &bv, const SVGValue &av)
{
    init();
    baseVal = bv;
    animVal = av;
}


/**
 *
 */
SVGAnimatedValue::SVGAnimatedValue(const SVGAnimatedValue &other)
{
    assign(other);
}


/**
 *
 */
SVGAnimatedValue &SVGAnimatedValue::operator=(const SVGAnimatedValue &other)
{
    assign(other);
    return *this;
}


/**
 *
 */
SVGAnimatedValue &SVGAnimatedValue::operator=(const SVGValue &bv)
{
    init();
    baseVal = bv;
}


/**
 *
 */
SVGAnimatedValue::~SVGAnimatedValue()
{
}



void SVGAnimatedValue::init()
{
}


void SVGAnimatedValue::assign(const SVGAnimatedValue &other)
{
    baseVal = other.baseVal;
    animVal = other.animVal;
}





















//########################################################################
//########################################################################
//########################################################################
//#   D O M
//########################################################################
//########################################################################
//########################################################################







/*#########################################################################
## SVGElement
#########################################################################*/


//####################################################################
//# BASE METHODS FOR SVGElement
//####################################################################

/**
 * Get the value of the id attribute on the given element.
 */
DOMString getId()
{
}

/**
 * Set the value of the id attribute on the given element.
 */
void setId(const DOMString &val) throw (DOMException)
{
}


/**
 * Corresponds to attribute xml:base on the given element.
 */
DOMString getXmlBase()
{
}


/**
 * Corresponds to attribute xml:base on the given element.
 */
void setXmlBase(const DOMString &val) throw (DOMException)
{
}

/**
 * The nearest ancestor 'svg' element. Null if the given element is the
 *      outermost 'svg' element.
 */
SVGElementPtr getOwnerSVGElement()
{
}

/**
 * The element which established the current viewport. Often, the nearest
 * ancestor 'svg' element. Null if the given element is the outermost 'svg'
 * element.
 */
SVGElementPtr getViewportElement()
{
}


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
unsigned short getUnitType()
{
}

/**
 *
 */
double getValue()
{
}

/**
 *
 */
void setValue(double val) throw (DOMException)
{
}

/**
 *
 */
double getValueInSpecifiedUnits()
{
}

/**
 *
 */
void setValueInSpecifiedUnits(double /*val*/) throw (DOMException)
{
}

/**
 *
 */
DOMString getValueAsString()
{
}

/**
 *
 */
void setValueAsString(const DOMString &/*val*/) throw (DOMException)
{
}


/**
 *
 */
void newValueSpecifiedUnits(unsigned short /*unitType*/,
                            double /*valueInSpecifiedUnits*/)
{
}

/**
 *
 */
void convertToSpecifiedUnits(unsigned short /*unitType*/)
{
}

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
SVGValue &getBaseVal()
{
    return baseVal();
}

/**
 *
 */
void setBaseVal(const SVGValue &val) throw (DOMException)
{
    baseVal = val;
}

/**
 *
 */
SVGValue &getAnimVal()
{
    return animVal;
}



//####################################################################
//# SVGColor                    
//####################################################################

/**
 * From CSSValue 
 * A code defining the type of the value as defined above.
 */
unsigned short getCssValueType()
{
}

/**
 * From CSSValue 
 * A string representation of the current value.
 */
DOMString getCssText()
{
}

/**
 * From CSSValue 
 * A string representation of the current value.
 * Note that setting implies parsing.     
 */
void setCssText(const DOMString &val) throw (dom::DOMException)
{
}


/**
 *
 */
unsigned short getColorType()
{
}

/**
 *
 */
css::RGBColor getRgbColor()
{
}

/**
 *
 */
SVGICCColor getIccColor()
{
}


/**
 *
 */
void setRGBColor(const DOMString& /*rgbColor*/) throw (SVGException)
{
}

/**
 *
 */
void setRGBColorICCColor(const DOMString& /*rgbColor*/,
                         const DOMString& /*iccColor*/)
                         throw (SVGException)
{
}

/**
 *
 */
void setColor(unsigned short /*colorType*/,
                       const DOMString& /*rgbColor*/,
                       const DOMString& /*iccColor*/)
                       throw (SVGException)
{
}

//####################################################################
//# SVGCSSRule                  
//####################################################################

/**
 * From CSSRule    
 * The type of the rule, as defined above. The expectation is that 
 * binding-specific casting methods can be used to cast down from an instance of 
 * the CSSRule interface to the specific derived interface implied by the type.
 */
unsigned short getType()
{
}

/**
 * From CSSRule    
 * The parsable textual representation of the rule. This reflects the current 
 * state of the rule and not its initial value.
 */
DOMString getCssText()
{
}

/**
 * From CSSRule    
 * The parsable textual representation of the rule. This reflects the current 
 * state of the rule and not its initial value.
 * Note that setting involves reparsing.     
 */
void setCssText(const DOMString &val) throw (DOMException)
{
}

/**
 * From CSSRule    
 * The style sheet that contains this rule.
 */
css::CSSStyleSheet *getParentStyleSheet()
{
}

/**
 * From CSSRule    
 * If this rule is contained inside another rule(e.g. a style rule inside an 
 * @media block), this is the containing rule. If this rule is not nested inside 
 * any other rules, this returns null.
 */
css::CSSRule *getParentRule()
{
}

//####################################################################
//# SVGExternalResourcesRequired
//####################################################################

/**
 *
 */
SVGAnimatedBoolean getExternalResourcesRequired()
{
}

//####################################################################
//# SVGFitToViewBox             
//####################################################################

/**
 *
 */
SVGAnimatedRect getViewBox()
{
}

/**
 *
 */
SVGAnimatedPreserveAspectRatio getPreserveAspectRatio()
{
}

//####################################################################
//# SVGICCColor                 
//####################################################################

/**
 *
 */
DOMString getColorProfile()
{
}

/**
 *
 */
void setColorProfile(const DOMString &val) throw (DOMException)
{
}

/**
 *
 */
SVGNumberList &getColors()
{
}

//####################################################################
//# SVGLangSpace                
//####################################################################

/**
 *
 */
DOMString getXmllang()
{
}

/**
 *
 */
void setXmllang(const DOMString &val) throw (DOMException)
{
}

/**
 *
 */
DOMString getXmlspace()
{
}

/**
 *
 */
void setXmlspace(const DOMString &val) throw (DOMException)
{
}

//####################################################################
//# SVGLength                   
//####################################################################

/**
 *
 */
unsigned short getUnitType()
{
}

/**
 *
 */
double getValue()
{
}

/**
 *
 */
void setValue(double val) throw (DOMException)
{
}

/**
 *
 */
double getValueInSpecifiedUnits()
{
}

/**
 *
 */
void setValueInSpecifiedUnits(double /*val*/) throw (DOMException)
{
}

/**
 *
 */
DOMString getValueAsString()
{
}

/**
 *
 */
void setValueAsString(const DOMString& /*val*/) throw (DOMException)
{
}


/**
 *
 */
void newValueSpecifiedUnits(unsigned short /*unitType*/, double /*val*/)
{
}

/**
 *
 */
void convertToSpecifiedUnits(unsigned short /*unitType*/)
{
}


//####################################################################
//## SVGLengthList - see SVGValueList
//####################################################################



//####################################################################
//# SVGLocatable                
//####################################################################

/**
 *
 */
SVGElementPtr getNearestViewportElement()
{
}

/**
 *
 */
SVGElement *getFarthestViewportElement()
{
}

/**
 *
 */
SVGRect getBBox()
{
}

/**
 *
 */
SVGMatrix getCTM()
{
}

/**
 *
 */
SVGMatrix getScreenCTM()
{
}

/**
 *
 */
SVGMatrix getTransformToElement(const SVGElement &/*element*/)
                                throw (SVGException)
{
}

//####################################################################
//# SVGNumber                   
//####################################################################

/**
 *
 */
double getValue()
{
}

/**
 *
 */
void setValue(double val) throw (DOMException)
{
}

//####################################################################
//# SVGNumberList - see SVGValueList      
//####################################################################


//####################################################################
//# SVGRect                     
//####################################################################

/**
 *
 */
double getX()
{
}

/**
 *
 */
void setX(double val) throw (DOMException)
{
}

/**
 *
 */
double getY()
{
}

/**
 *
 */
void setY(double val) throw (DOMException)
{
}

/**
 *
 */
double getWidth()
{
}

/**
 *
 */
void setWidth(double val) throw (DOMException)
{
}

/**
 *
 */
double getHeight()
{
}

/**
 *
 */
void setHeight(double val) throw (DOMException)
{
}

//####################################################################
//# SVGRenderingIntent          
//####################################################################

//####################################################################
//# SVGStringList - see SVGValueList
//####################################################################

//####################################################################
//# SVGStylable                 
//####################################################################

/**
 *
 */
SVGAnimatedString getClassName()
{
}

/**
 *
 */
css::CSSStyleDeclaration getStyle()
{
}

/**
 *
 */
css::CSSValue getPresentationAttribute(const DOMString& /*name*/)
{
}

//####################################################################
//# SVGTests                    
//####################################################################

/**
 *
 */
SVGValueList &getRequiredFeatures()
{
}

/**
 *
 */
SVGValueList &getRequiredExtensions()
{
}

/**
 *
 */
SVGValueList &getSystemLanguage()
{
}

/**
 *
 */
bool hasExtension(const DOMString& /*extension*/)
{
}

//####################################################################
//# SVGTransformable            
//####################################################################

/**
 *
 */
SVGAnimatedList &getTransform()
{
}

//####################################################################
//# SVGUnitTypes                
//####################################################################

//####################################################################
//# SVGURIReference             
//####################################################################

/**
 *
 */
SVGAnimatedValue getHref()
{
}

//####################################################################
//## SVGValueList - consolidation of other lists
//####################################################################

/**
 *
 */
unsigned long SVGElement::getNumberOfItems()
{
    return items.size();
}

/**
 *
 */
void SVGElement::clear() throw (DOMException)
{
    items.clear();
}

/**
 *
 */
SVGValue SVGElement::initialize(const SVGValue& newItem)
                throw (DOMException, SVGException)
{
    items.clear();
    items.push_back(newItem);
    return newItem;
}

/**
 *
 */
SVGValue SVGElement::getItem(unsigned long index) throw (DOMException)
{
    if (index >= items.size())
        return "";
    return items[index];
}

/**
 *
 */
SVGValue SVGElement::insertItemBefore(const SVGValue& newItem,
                                          unsigned long index)
                                          throw (DOMException, SVGException)
{
    if (index>=items.size())
        {
        items.push_back(newItem);
        }
    else
        {
        std::vector<SVGValue>::iterator iter = items.begin() + index;
        items.insert(iter, newItem);
        }
    return newItem;
}

/**
 *
 */
SVGValue SVGElement::replaceItem (const SVGValue& newItem,
                                unsigned long index)
                            throw (DOMException, SVGException)
{
    if (index>=items.size())
        return "";
    std::vector<SVGValue>::iterator iter = items.begin() + index;
    *iter = newItem;
    return newItem;
}

/**
 *
 */
SVGValue SVGElement::removeItem (unsigned long index)
                throw (DOMException)
{
    if (index>=items.size())
        return "";
    std::vector<SVGValue>::iterator iter = items.begin() + index;
    SVGValue oldval = *iter;
    items.erase(iter);
    return oldval;
}

/**
 *
 */
SVGValue SVGElement::appendItem (const SVGValue& newItem)
                throw (DOMException, SVGException)
{
    items.push_back(newItem);
    return newItem;
}


/**
 * Matrix
 */
SVGValue SVGElement::createSVGTransformFromMatrix(const SVGValue &matrix)
{
}

/**
 * Matrix
 */
SVGValue SVGElement::consolidate()
{
}


//####################################################################
//# SVGViewSpec                 
//####################################################################

/**
 *
 */
//SVGTransformList getTransform()
//{
//}

/**
 *
 */
SVGElementPtr getViewTarget()
{
}

/**
 *
 */
DOMString getViewBoxString()
{
}

/**
 *
 */
DOMString getPreserveAspectRatioString()
{
}

/**
 *
 */
DOMString getTransformString()
{
}

/**
 *
 */
DOMString getViewTargetString()
{
}

//####################################################################
//# SVGZoomAndPan               
//####################################################################

/**
 *
 */
unsigned short getZoomAndPan()
{
}

/**
 *
 */
void setZoomAndPan(unsigned short val) throw (DOMException)
{
}

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
SVGAnimatedString getTarget()
{
}



//####################################################################
//# SVGAltGlyphElement
//####################################################################


/**
 * Get the attribute glyphRef on the given element.
 */
DOMString getGlyphRef()
{
}

/**
 * Set the attribute glyphRef on the given element.
 */
void setGlyphRef(const DOMString &val) throw (DOMException)
{
}

/**
 * Get the attribute format on the given element.
 */
DOMString getFormat()
{
}

/**
 * Set the attribute format on the given element.
 */
void setFormat(const DOMString &val) throw (DOMException)
{
}


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
SVGElementPtr getTargetElement()
{
}

/**
 *
 */
double getStartTime()
{
}

/**
 *
 */
double getCurrentTime()
{
}

/**
 *
 */
double getSimpleDuration() throw (DOMException)
{
}



//####################################################################
//# SVGCircleElement
//####################################################################

/**
 * Corresponds to attribute cx on the given 'circle' element.
 */
SVGAnimatedLength getCx()
{
}

/**
 * Corresponds to attribute cy on the given 'circle' element.
 */
SVGAnimatedLength getCy()
{
}

/**
 * Corresponds to attribute r on the given 'circle' element.
 */
SVGAnimatedLength getR()
{
}

//####################################################################
//# SVGClipPathElement
//####################################################################


/**
 * Corresponds to attribute clipPathUnits on the given 'clipPath' element.
 *      Takes one of the constants defined in SVGUnitTypes.
 */
SVGAnimatedEnumeration getClipPathUnits()
{
}



//####################################################################
//# SVGColorProfileElement
//####################################################################


/**
 * Get the attribute local on the given element.
 */
DOMString getLocal()
{
}

/**
 * Set the attribute local on the given element.
 */
void setLocal(const DOMString &val) throw (DOMException)
{
}

/**
 * Get the attribute name on the given element.
 */
DOMString getName()
{
}

/**
 * Set the attribute name on the given element.
 */
void setName(const DOMString &val) throw (DOMException)
{
}

/**
 * Set the attribute rendering-intent on the given element.
 * The type of rendering intent, identified by one of the
 *      SVGRenderingIntent constants.
 */
unsigned short getRenderingIntent()
{
}

/**
 * Get the attribute rendering-intent on the given element.
 */
void setRenderingIntent(unsigned short val) throw (DOMException)
{
}


//####################################################################
//# SVGComponentTransferFunctionElement
//####################################################################

/**
 * Corresponds to attribute type on the given element. Takes one
 *      of the Component Transfer Types.
 */
SVGAnimatedEnumeration getType()
{
}

/**
 * Corresponds to attribute tableValues on the given element.
 */
SVGAnimatedNumberList getTableValues()
{
}

/**
 * Corresponds to attribute slope on the given element.
 */
SVGAnimatedNumber getSlope()
{
}

/**
 * Corresponds to attribute intercept on the given element.
 */
SVGAnimatedNumber getIntercept()
{
}

/**
 * Corresponds to attribute amplitude on the given element.
 */
SVGAnimatedNumber getAmplitude()
{
}

/**
 * Corresponds to attribute exponent on the given element.
 */
SVGAnimatedNumber getExponent()
{
}

/**
 * Corresponds to attribute offset on the given element.
 */
SVGAnimatedNumber getOffset()
{
}

//####################################################################
//# SVGCursorElement
//####################################################################

/**
 *
 */
SVGAnimatedLength getX()
{
}

/**
 *
 */
SVGAnimatedLength getY()
{
}


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
SVGAnimatedLength getCx()
{
}

/**
 * Corresponds to attribute cy on the given 'ellipse' element.
 */
SVGAnimatedLength getCy()
{
}

/**
 * Corresponds to attribute rx on the given 'ellipse' element.
 */
SVGAnimatedLength getRx()
{
}

/**
 * Corresponds to attribute ry on the given 'ellipse' element.
 */
SVGAnimatedLength getRy()
{
}


//####################################################################
//# SVGFEBlendElement
//####################################################################

/**
 * Corresponds to attribute in on the given 'feBlend' element.
 */
SVGAnimatedString getIn1()
{
}

/**
 * Corresponds to attribute in2 on the given 'feBlend' element.
 */
SVGAnimatedString getIn2()
{
}

/**
 * Corresponds to attribute mode on the given 'feBlend' element.
 *      Takes one of the Blend Mode Types.
 */
SVGAnimatedEnumeration getMode()
{
}


//####################################################################
//# SVGFEColorMatrixElement
//####################################################################

/**
 * Corresponds to attribute in on the given 'feColorMatrix' element.
 */
SVGAnimatedString getIn1()
{
}

/**
 * Corresponds to attribute type on the given 'feColorMatrix' element.
 *      Takes one of the Color Matrix Types.
 */
SVGAnimatedEnumeration getType()
{
}

/**
 * Corresponds to attribute values on the given 'feColorMatrix' element.
 * Provides access to the contents of the values attribute.
 */
SVGAnimatedNumberList getValues()
{
}


//####################################################################
//# SVGFEComponentTransferElement
//####################################################################


/**
 * Corresponds to attribute in on the given 'feComponentTransfer'  element.
 */
SVGAnimatedString getIn1()
{
}

//####################################################################
//# SVGFECompositeElement
//####################################################################

/**
 * Corresponds to attribute in on the given 'feComposite' element.
 */
SVGAnimatedString getIn1()
{
}

/**
 * Corresponds to attribute in2 on the given 'feComposite' element.
 */
SVGAnimatedString getIn2()
{
}

/**
 * Corresponds to attribute operator on the given 'feComposite' element.
 *      Takes one of the Composite Operators.
 */
SVGAnimatedEnumeration getOperator()
{
}

/**
 * Corresponds to attribute k1 on the given 'feComposite' element.
 */
SVGAnimatedNumber getK1()
{
}

/**
 * Corresponds to attribute k2 on the given 'feComposite' element.
 */
SVGAnimatedNumber getK2()
{
}

/**
 * Corresponds to attribute k3 on the given 'feComposite' element.
 */
SVGAnimatedNumber getK3()
{
}

/**
 * Corresponds to attribute k4 on the given 'feComposite' element.
 */
SVGAnimatedNumber getK4()
{
}


//####################################################################
//# SVGFEConvolveMatrixElement
//####################################################################


/**
 * Corresponds to attribute order on the given 'feConvolveMatrix'  element.
 */
SVGAnimatedInteger getOrderX()
{
}

/**
 * Corresponds to attribute order on the given 'feConvolveMatrix'  element.
 */
SVGAnimatedInteger getOrderY()
{
}

/**
 * Corresponds to attribute kernelMatrix on the given element.
 */
SVGAnimatedNumberList getKernelMatrix()
{
}

/**
 * Corresponds to attribute divisor on the given 'feConvolveMatrix' element.
 */
SVGAnimatedNumber getDivisor()
{
}

/**
 * Corresponds to attribute bias on the given 'feConvolveMatrix'  element.
 */
SVGAnimatedNumber getBias()
{
}

/**
 * Corresponds to attribute targetX on the given 'feConvolveMatrix'  element.
 */
SVGAnimatedInteger getTargetX()
{
}

/**
 * Corresponds to attribute targetY on the given 'feConvolveMatrix'  element.
 */
SVGAnimatedInteger getTargetY()
{
}

/**
 * Corresponds to attribute edgeMode on the given 'feConvolveMatrix'
 *      element. Takes one of the Edge Mode Types.
 */
SVGAnimatedEnumeration getEdgeMode()
{
}

/**
 * Corresponds to attribute kernelUnitLength on the
 *      given 'feConvolveMatrix'  element.
 */
SVGAnimatedLength getKernelUnitLengthX()
{
}

/**
 * Corresponds to attribute kernelUnitLength on the given
 *      'feConvolveMatrix'  element.
 */
SVGAnimatedLength getKernelUnitLengthY()
{
}

/**
 * Corresponds to attribute preserveAlpha on the
 *      given 'feConvolveMatrix'  element.
 */
SVGAnimatedBoolean getPreserveAlpha()
{
}



//####################################################################
//# SVGFEDiffuseLightingElement
//####################################################################


/**
 * Corresponds to attribute in on the given 'feDiffuseLighting'  element.
 */
SVGAnimatedString getIn1()
{
}

/**
 * Corresponds to attribute surfaceScale on the given
 *      'feDiffuseLighting'  element.
 */
SVGAnimatedNumber getSurfaceScale()
{
}

/**
 * Corresponds to attribute diffuseConstant on the given
 *      'feDiffuseLighting'  element.
 */
SVGAnimatedNumber getDiffuseConstant()
{
}

/**
 * Corresponds to attribute kernelUnitLength on the given
 *      'feDiffuseLighting'  element.
 */
SVGAnimatedNumber getKernelUnitLengthX()
{
}

/**
 * Corresponds to attribute kernelUnitLength on the given
 *      'feDiffuseLighting'  element.
 */
SVGAnimatedNumber getKernelUnitLengthY()
{
}




//####################################################################
//# SVGFEDisplacementMapElement
//####################################################################

/**
 *
 */
SVGAnimatedString getIn1()
{
}

/**
 *
 */
SVGAnimatedString getIn2()
{
}


/**
 *
 */
SVGAnimatedNumber getScale()
{
}

/**
 *
 */
SVGAnimatedEnumeration getXChannelSelector()
{
}

/**
 *
 */
SVGAnimatedEnumeration getYChannelSelector()
{
}

//####################################################################
//# SVGFEDistantLightElement
//####################################################################


/**
 * Corresponds to attribute azimuth on the given 'feDistantLight'  element.
 */
SVGAnimatedNumber getAzimuth()
{
}


/**
 * Corresponds to attribute elevation on the given 'feDistantLight'
 *    element
 */
SVGAnimatedNumber getElevation()
{
}


//####################################################################
//# SVGFEFloodElement
//####################################################################


/**
 *
 */
SVGAnimatedString getIn1()
{
}


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
SVGAnimatedString getIn1()
{
}


/**
 *
 */
SVGAnimatedNumber getStdDeviationX()
{
}

/**
 *
 */
SVGAnimatedNumber getStdDeviationY()
{
}


/**
 *
 */
void setStdDeviation(double stdDeviationX, double stdDeviationY)
{
}


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
 *
 */
SVGAnimatedString getIn1()
{
}


/**
 *
 */
SVGAnimatedEnumeration getOperator()
{
}

/**
 *
 */
SVGAnimatedLength getRadiusX()
{
}

/**
 *
 */
SVGAnimatedLength getRadiusY()
{
}

//####################################################################
//# SVGFEOffsetElement
//####################################################################

/**
 *
 */
SVGAnimatedString getIn1()
{
}

/**
 *
 */
SVGAnimatedLength getDx()
{
}

/**
 *
 */
SVGAnimatedLength getDy()
{
}


//####################################################################
//# SVGFEPointLightElement
//####################################################################

/**
 * Corresponds to attribute x on the given 'fePointLight' element.
 */
SVGAnimatedNumber getX()
{
}

/**
 * Corresponds to attribute y on the given 'fePointLight' element.
 */
SVGAnimatedNumber getY()
{
}

/**
 * Corresponds to attribute z on the given 'fePointLight' element.
 */
SVGAnimatedNumber getZ()
{
}

//####################################################################
//# SVGFESpecularLightingElement
//####################################################################


/**
 *
 */
SVGAnimatedString getIn1()
{
}

/**
 *
 */
SVGAnimatedNumber getSurfaceScale()
{
}

/**
 *
 */
SVGAnimatedNumber getSpecularConstant()
{
}

/**
 *
 */
SVGAnimatedNumber getSpecularExponent()
{
}


//####################################################################
//# SVGFESpotLightElement
//####################################################################

/**
 * Corresponds to attribute x on the given 'feSpotLight' element.
 */
SVGAnimatedNumber getX()
{
}

/**
 * Corresponds to attribute y on the given 'feSpotLight' element.
 */
SVGAnimatedNumber getY()
{
}

/**
 * Corresponds to attribute z on the given 'feSpotLight' element.
 */
SVGAnimatedNumber getZ()
{
}

/**
 * Corresponds to attribute pointsAtX on the given 'feSpotLight' element.
 */
SVGAnimatedNumber getPointsAtX()
{
}

/**
 * Corresponds to attribute pointsAtY on the given 'feSpotLight' element.
 */
SVGAnimatedNumber getPointsAtY()
{
}

/**
 * Corresponds to attribute pointsAtZ on the given 'feSpotLight' element.
 */
SVGAnimatedNumber getPointsAtZ()
{
}

/**
 * Corresponds to attribute specularExponent on the
 *      given 'feSpotLight'  element.
 */
SVGAnimatedNumber getSpecularExponent()
{
}

/**
 * Corresponds to attribute limitingConeAngle on the
 *      given 'feSpotLight'  element.
 */
SVGAnimatedNumber getLimitingConeAngle()
{
}


//####################################################################
//# SVGFETileElement
//####################################################################


/**
 *
 */
SVGAnimatedString getIn1()
{
}


//####################################################################
//# SVGFETurbulenceElement
//####################################################################


/**
 *
 */
SVGAnimatedNumber getBaseFrequencyX()
{
}

/**
 *
 */
SVGAnimatedNumber getBaseFrequencyY()
{
}

/**
 *
 */
SVGAnimatedInteger getNumOctaves()
{
}

/**
 *
 */
SVGAnimatedNumber getSeed()
{
}

/**
 *
 */
SVGAnimatedEnumeration getStitchTiles()
{
}

/**
 *
 */
SVGAnimatedEnumeration getType()
{
}



//####################################################################
//# SVGFilterElement
//####################################################################


/**
 * Corresponds to attribute filterUnits on the given 'filter' element. Takes one
 * of the constants defined in SVGUnitTypes.
 */
SVGAnimatedEnumeration getFilterUnits()
{
}

/**
 * Corresponds to attribute primitiveUnits on the given 'filter' element. Takes
 * one of the constants defined in SVGUnitTypes.
 */
SVGAnimatedEnumeration getPrimitiveUnits()
{
}

/**
 *
 */
SVGAnimatedLength getX()
{
}

/**
 * Corresponds to attribute x on the given 'filter' element.
 */
SVGAnimatedLength getY()
{
}

/**
 * Corresponds to attribute y on the given 'filter' element.
 */
SVGAnimatedLength getWidth()
{
}

/**
 * Corresponds to attribute height on the given 'filter' element.
 */
SVGAnimatedLength getHeight()
{
}


/**
 * Corresponds to attribute filterRes on the given 'filter' element.
 *      Contains the X component of attribute filterRes.
 */
SVGAnimatedInteger getFilterResX()
{
}

/**
 * Corresponds to attribute filterRes on the given 'filter' element.
 * Contains the Y component(possibly computed automatically)
 *      of attribute filterRes.
 */
SVGAnimatedInteger getFilterResY()
{
}

/**
 * Sets the values for attribute filterRes.
 */
void setFilterRes(unsigned long filterResX, unsigned long filterResY)
{
}


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
SVGAnimatedLength getX()
{
}

/**
 *
 */
SVGAnimatedLength getY()
{
}

/**
 *
 */
SVGAnimatedLength getWidth()
{
}

/**
 *
 */
SVGAnimatedLength getHeight()
{
}



//####################################################################
//# SVGGlyphRefElement
//####################################################################


/**
 * Get the attribute glyphRef on the given element.
 */
DOMString getGlyphRef()
{
}

/**
 * Set the attribute glyphRef on the given element.
 */
void setGlyphRef(const DOMString &val) throw (DOMException)
{
}

/**
 * Get the attribute format on the given element.
 */
DOMString getFormat()
{
}

/**
 * Set the attribute format on the given element.
 */
void setFormat(const DOMString &val) throw (DOMException)
{
}

/**
 * Get the attribute x on the given element.
 */
double getX()
{
}

/**
 * Set the attribute x on the given element.
 */
void setX(double val) throw (DOMException)
{
}

/**
 * Get the attribute y on the given element.
 */
double getY()
{
}

/**
 * Set the attribute y on the given element.
 */
void setY(double val) throw (DOMException)
{
}

/**
 * Get the attribute dx on the given element.
 */
double getDx()
{
}

/**
 * Set the attribute dx on the given element.
 */
void setDx(double val) throw (DOMException)
{
}

/**
 * Get the attribute dy on the given element.
 */
double getDy()
{
}

/**
 * Set the attribute dy on the given element.
 */
void setDy(double val) throw (DOMException)
{
}


//####################################################################
//# SVGGradientElement
//####################################################################

/**
 * Corresponds to attribute gradientUnits on the given element.
 *      Takes one of the constants defined in SVGUnitTypes.
 */
SVGAnimatedEnumeration getGradientUnits()
{
}

/**
 * Corresponds to attribute gradientTransform on the given element.
 */
SVGAnimatedTransformList getGradientTransform()
{
}

/**
 * Corresponds to attribute spreadMethod on the given element.
 *      One of the Spread Method Types.
 */
SVGAnimatedEnumeration getSpreadMethod()
{
}



//####################################################################
//# SVGHKernElement
//####################################################################

//####################################################################
//# SVGImageElement
//####################################################################

/**
 * Corresponds to attribute x on the given 'image' element.
 */
SVGAnimatedLength getX()
{
}

/**
 * Corresponds to attribute y on the given 'image' element.
 */
SVGAnimatedLength getY()
{
}

/**
 * Corresponds to attribute width on the given 'image' element.
 */
SVGAnimatedLength getWidth()
{
}

/**
 * Corresponds to attribute height on the given 'image' element.
 */
SVGAnimatedLength getHeight()
{
}


/**
 * Corresponds to attribute preserveAspectRatio on the given element.
 */
SVGAnimatedPreserveAspectRatio getPreserveAspectRatio()
{
}

//####################################################################
//# SVGLinearGradientElement
//####################################################################

/**
 * Corresponds to attribute x1 on the given 'linearGradient'  element.
 */
SVGAnimatedLength getX1()
{
}

/**
 * Corresponds to attribute y1 on the given 'linearGradient'  element.
 */
SVGAnimatedLength getY1()
{
}

/**
 * Corresponds to attribute x2 on the given 'linearGradient'  element.
 */
SVGAnimatedLength getX2()
{
}

/**
 * Corresponds to attribute y2 on the given 'linearGradient'  element.
 */
SVGAnimatedLength getY2()
{
}



//####################################################################
//# SVGLineElement
//####################################################################

/**
 * Corresponds to attribute x1 on the given 'line' element.
 */
SVGAnimatedLength getX1()
{
}

/**
 * Corresponds to attribute y1 on the given 'line' element.
 */
SVGAnimatedLength getY1()
{
}

/**
 * Corresponds to attribute x2 on the given 'line' element.
 */
SVGAnimatedLength getX2()
{
}

/**
 * Corresponds to attribute y2 on the given 'line' element.
 */
SVGAnimatedLength getY2()
{
}


//####################################################################
//# SVGMarkerElement
//####################################################################


/**
 * Corresponds to attribute refX on the given 'marker' element.
 */
SVGAnimatedLength getRefX()
{
}

/**
 * Corresponds to attribute refY on the given 'marker' element.
 */
SVGAnimatedLength getRefY()
{
}

/**
 * Corresponds to attribute markerUnits on the given 'marker' element.
 *      One of the Marker Units Types defined above.
 */
SVGAnimatedEnumeration getMarkerUnits()
{
}

/**
 * Corresponds to attribute markerWidth on the given 'marker' element.
 */
SVGAnimatedLength getMarkerWidth()
{
}

/**
 * Corresponds to attribute markerHeight on the given 'marker' element.
 */
SVGAnimatedLength getMarkerHeight()
{
}

/**
 * Corresponds to attribute orient on the given 'marker' element.
 *      One of the Marker Orientation Types defined above.
 */
SVGAnimatedEnumeration getOrientType()
{
}

/**
 * Corresponds to attribute orient on the given 'marker' element.
 * If markerUnits is SVG_MARKER_ORIENT_ANGLE, the angle value for
 * attribute orient ; otherwise, it will be set to zero.
 */
SVGAnimatedAngle getOrientAngle()
{
}


/**
 * Sets the value of attribute orient to 'auto'.
 */
void setOrientToAuto()
{
}

/**
 * Sets the value of attribute orient to the given angle.
 */
void setOrientToAngle(const SVGAngle &angle)
{
}


//####################################################################
//# SVGMaskElement
//####################################################################


/**
 * Corresponds to attribute maskUnits on the given 'mask' element. Takes one of
 * the constants defined in SVGUnitTypes.
 */
SVGAnimatedEnumeration getMaskUnits()
{
}

/**
 * Corresponds to attribute maskContentUnits on the given 'mask' element. Takes
 * one of the constants defined in SVGUnitTypes.
 */
SVGAnimatedEnumeration getMaskContentUnits()
{
}

/**
 * Corresponds to attribute x on the given 'mask' element.
 */
SVGAnimatedLength getX()
{
}

/**
 * Corresponds to attribute y on the given 'mask' element.
 */
SVGAnimatedLength getY()
{
}

/**
 * Corresponds to attribute width on the given 'mask' element.
 */
SVGAnimatedLength getWidth()
{
}

/**
 * Corresponds to attribute height on the given 'mask' element.
 */
SVGAnimatedLength getHeight()
{
}

//####################################################################
//# SVGMetadataElement
//####################################################################

//####################################################################
//# SVGMissingGlyphElement
//####################################################################

//####################################################################
//# SVGMPathElement
//####################################################################

//####################################################################
//# SVGPathElement
//####################################################################

/**
 * Corresponds to attribute pathLength on the given 'path' element.
 */
SVGAnimatedNumber getPathLength()
{
}

/**
 * Returns the user agent's computed value for the total length of the path using
 * the user agent's distance-along-a-path algorithm, as a distance in the current
 * user coordinate system.
 */
double getTotalLength()
{
}

/**
 * Returns the(x,y) coordinate in user space which is distance units along the
 * path, utilizing the user agent's distance-along-a-path algorithm.
 */
SVGPoint getPointAtLength(double distance)
{
}

/**
 * Returns the index into pathSegList which is distance units along the path,
 * utilizing the user agent's distance-along-a-path algorithm.
 */
unsigned long getPathSegAtLength(double distance)
{
}

/**
 * Returns a stand-alone, parentless SVGPathSegClosePath object.
 */
SVGPathSeg createSVGPathSegClosePath()
{
    SVGPathSeg seg(PATHSEG_CLOSEPATH);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegMovetoAbs object.
 */
SVGPathSeg createSVGPathSegMovetoAbs(double x, double y)
{
    SVGPathSeg seg(PATHSEG_MOVETO_ABS);
    seg.setX(x);
    seg.setY(y);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegMovetoRel object.
 */
SVGPathSeg createSVGPathSegMovetoRel(double x, double y)
{
    SVGPathSeg seg(PATHSEG_MOVETO_REL);
    seg.setX(x);
    seg.setY(y);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegLinetoAbs object.
 */
SVGPathSeg createSVGPathSegLinetoAbs(double x, double y)
{
    SVGPathSeg seg(PATHSEG_LINETO_ABS);
    seg.setX(x);
    seg.setY(y);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegLinetoRel object.
 */
SVGPathSeg createSVGPathSegLinetoRel(double x, double y)
{
    SVGPathSeg seg(PATHSEG_LINETO_REL);
    seg.setX(x);
    seg.setY(y);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegCurvetoCubicAbs object.
 */
SVGPathSeg createSVGPathSegCurvetoCubicAbs(double x, double y,
                    double x1, double y1, double x2, double y2)
{
    SVGPathSeg seg(PATHSEG_CURVETO_CUBIC_ABS);
    seg.setX(x);
    seg.setY(y);
    seg.setX1(x1);
    seg.setY1(y1);
    seg.setX2(x2);
    seg.setY2(y2);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegCurvetoCubicRel object.
 */
SVGPathSeg createSVGPathSegCurvetoCubicRel(double x, double y,
                    double x1, double y1, double x2, double y2)
{
    SVGPathSeg seg(PATHSEG_CURVETO_CUBIC_REL);
    seg.setX(x);
    seg.setY(y);
    seg.setX1(x1);
    seg.setY1(y1);
    seg.setX2(x2);
    seg.setY2(y2);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegCurvetoQuadraticAbs object.
 */
SVGPathSeg createSVGPathSegCurvetoQuadraticAbs(double x, double y,
                     double x1, double y1)
{
    SVGPathSeg seg(PATHSEG_CURVETO_QUADRATIC_ABS);
    seg.setX(x);
    seg.setY(y);
    seg.setX1(x1);
    seg.setY1(y1);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegCurvetoQuadraticRel object.
 */
SVGPathSeg createSVGPathSegCurvetoQuadraticRel(double x, double y,
                     double x1, double y1)
{
    SVGPathSeg seg(PATHSEG_CURVETO_QUADRATIC_REL);
    seg.setX(x);
    seg.setY(y);
    seg.setX1(x1);
    seg.setY1(y1);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegArcAbs object.
 */
SVGPathSeg createSVGPathSegArcAbs(double x, double y,
                     double r1, double r2, double angle,
                     bool largeArcFlag, bool sweepFlag)
{
    SVGPathSeg seg(PATHSEG_ARC_ABS);
    seg.setX(x);
    seg.setY(y);
    seg.setR1(r1);
    seg.setR2(r2);
    seg.setAngle(angle);
    seg.setLargeArcFlag(largeArcFlag);
    seg.setSweepFlag(sweepFlag);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegArcRel object.
 */
SVGPathSeg createSVGPathSegArcRel(double x, double y, double r1,
                     double r2, double angle, bool largeArcFlag,
                     bool sweepFlag)
{
    SVGPathSeg seg(PATHSEG_ARC_REL);
    seg.setX(x);
    seg.setY(y);
    seg.setR1(r1);
    seg.setR2(r2);
    seg.setAngle(angle);
    seg.setLargeArcFlag(largeArcFlag);
    seg.setSweepFlag(sweepFlag);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegLinetoHorizontalAbs object.
 */
SVGPathSeg createSVGPathSegLinetoHorizontalAbs(double x)
{
    SVGPathSeg seg(PATHSEG_LINETO_HORIZONTAL_ABS);
    seg.setX(x);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegLinetoHorizontalRel object.
 */
SVGPathSeg createSVGPathSegLinetoHorizontalRel(double x)
{
    SVGPathSeg seg(PATHSEG_LINETO_HORIZONTAL_REL);
    seg.setX(x);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegLinetoVerticalAbs object.
 */
SVGPathSeg createSVGPathSegLinetoVerticalAbs(double y)
{
    SVGPathSeg seg(PATHSEG_LINETO_VERTICAL_ABS);
    seg.setY(y);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegLinetoVerticalRel object.
 */
SVGPathSeg createSVGPathSegLinetoVerticalRel(double y)
{
    SVGPathSeg seg(PATHSEG_LINETO_VERTICAL_REL);
    seg.setY(y);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegCurvetoCubicSmoothAbs object.
 */
SVGPathSeg createSVGPathSegCurvetoCubicSmoothAbs(double x, double y,
                                         double x2, double y2)
{
    SVGPathSeg seg(PATHSEG_CURVETO_CUBIC_SMOOTH_ABS);
    seg.setX(x);
    seg.setY(y);
    seg.setX2(x2);
    seg.setY2(y2);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegCurvetoCubicSmoothRel object.
 */
SVGPathSeg createSVGPathSegCurvetoCubicSmoothRel(double x, double y,
                                                  double x2, double y2)
{
    SVGPathSeg seg(PATHSEG_CURVETO_CUBIC_SMOOTH_REL);
    seg.setX(x);
    seg.setY(y);
    seg.setX2(x2);
    seg.setY2(y2);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegCurvetoQuadraticSmoothAbs
 *      object.
 */
SVGPathSeg createSVGPathSegCurvetoQuadraticSmoothAbs(double x, double y)
{
    SVGPathSeg seg(PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS);
    seg.setX(x);
    seg.setY(y);
    return seg;
}

/**
 * Returns a stand-alone, parentless SVGPathSegCurvetoQuadraticSmoothRel
 *      object.
 */
SVGPathSeg createSVGPathSegCurvetoQuadraticSmoothRel(double x, double y)
{
    SVGPathSeg seg(PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL);
    seg.setX(x);
    seg.setY(y);
    return seg;
}


//####################################################################
//# SVGPatternElement
//####################################################################

/**
 * Corresponds to attribute patternUnits on the given 'pattern' element.
 * Takes one of the constants defined in SVGUnitTypes.
 */
SVGAnimatedEnumeration getPatternUnits()
{
}

/**
 * Corresponds to attribute patternContentUnits on the given 'pattern'
 *      element. Takes one of the constants defined in SVGUnitTypes.
 */
SVGAnimatedEnumeration getPatternContentUnits()
{
}

/**
 * Corresponds to attribute patternTransform on the given 'pattern' element.
 */
SVGAnimatedTransformList getPatternTransform()
{
}

/**
 * Corresponds to attribute x on the given 'pattern' element.
 */
SVGAnimatedLength getX()
{
}

/**
 *
 */
SVGAnimatedLength getY()
{
}

/**
 * Corresponds to attribute width on the given 'pattern' element.
 */
SVGAnimatedLength getWidth()
{
}

/**
 * Corresponds to attribute height on the given 'pattern' element.
 */
SVGAnimatedLength getHeight()
{
}


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
SVGAnimatedLength getCx()
{
}


/**
 * Corresponds to attribute cy on the given 'radialGradient'  element.
 */
SVGAnimatedLength getCy()
{
}


/**
 * Corresponds to attribute r on the given 'radialGradient'  element.
 */
SVGAnimatedLength getR()
{
}


/**
 * Corresponds to attribute fx on the given 'radialGradient'  element.
 */
SVGAnimatedLength getFx()
{
}


/**
 * Corresponds to attribute fy on the given 'radialGradient'  element.
 */
SVGAnimatedLength getFy()
{
}


//####################################################################
//# SVGRectElement
//####################################################################

/**
 * Corresponds to attribute x on the given 'rect' element.
 */
SVGAnimatedLength getX()
{
}

/**
 * Corresponds to attribute y on the given 'rect' element.
 */
SVGAnimatedLength getY()
{
}

/**
 * Corresponds to attribute width on the given 'rect' element.
 */
SVGAnimatedLength getWidth()
{
}

/**
 * Corresponds to attribute height on the given 'rect' element.
 */
SVGAnimatedLength getHeight()
{
}


/**
 * Corresponds to attribute rx on the given 'rect' element.
 */
SVGAnimatedLength getRx()
{
}

/**
 * Corresponds to attribute ry on the given 'rect' element.
 */
SVGAnimatedLength getRy()
{
}


//####################################################################
//# SVGScriptElement
//####################################################################

/**
 *
 */
DOMString getType()
{
}

/**
 *
 */
void setType(const DOMString &val) throw (DOMException)
{
}

//####################################################################
//# SVGSetElement
//####################################################################

//####################################################################
//# SVGStopElement
//####################################################################


/**
 * Corresponds to attribute offset on the given 'stop' element.
 */
SVGAnimatedNumber getOffset()
{
}


//####################################################################
//# SVGStyleElement
//####################################################################

/**
 * Get the attribute xml:space on the given element.
 */
DOMString getXmlspace()
{
}

/**
 * Set the attribute xml:space on the given element.
 */
void setXmlspace(const DOMString &val) throw (DOMException)
{
}

/**
 * Get the attribute type on the given 'style' element.
 */
DOMString getType()
{
}

/**
 * Set the attribute type on the given 'style' element.
 */
void setType(const DOMString &val) throw (DOMException)
{
}

/**
 * Get the attribute media on the given 'style' element.
 */
DOMString getMedia()
{
}

/**
 * Set the attribute media on the given 'style' element.
 */
void setMedia(const DOMString &val) throw (DOMException)
{
}

/**
 * Get the attribute title on the given 'style' element.
 */
DOMString getTitle()
{
}

/**
 * Set the attribute title on the given 'style' element.
 */
void setTitle(const DOMString &val) throw (DOMException)
{
}

//####################################################################
//# SVGSymbolElement
//####################################################################

//####################################################################
//# SVGSVGElement
//####################################################################

/**
 * Corresponds to attribute x on the given 'svg' element.
 */
SVGAnimatedLength getX()
{
}

/**
 * Corresponds to attribute y on the given 'svg' element.
 */
SVGAnimatedLength getY()
{
}

/**
 * Corresponds to attribute width on the given 'svg' element.
 */
SVGAnimatedLength getWidth()
{
}

/**
 * Corresponds to attribute height on the given 'svg' element.
 */
SVGAnimatedLength getHeight()
{
}

/**
 * Get the attribute contentScriptType on the given 'svg' element.
 */
DOMString getContentScriptType()
{
}

/**
 * Set the attribute contentScriptType on the given 'svg' element.
 */
void setContentScriptType(const DOMString &val) throw (DOMException)
{
}


/**
 * Get the attribute contentStyleType on the given 'svg' element.
 */
DOMString getContentStyleType()
{
}

/**
 * Set the attribute contentStyleType on the given 'svg' element.
 */
void setContentStyleType(const DOMString &val) throw (DOMException)
{
}

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
 */
SVGRect getViewport()
{
}

/**
 * Size of a pixel units(as defined by CSS2) along the x-axis of the viewport,
 * which represents a unit somewhere in the range of 70dpi to 120dpi, and, on
 * systems that support this, might actually match the characteristics of the
 * target medium. On systems where it is impossible to know the size of a pixel,
 * a suitable default pixel size is provided.
 */
double getPixelUnitToMillimeterX()
{
}

/**
 * Corresponding size of a pixel unit along the y-axis of the viewport.
 */
double getPixelUnitToMillimeterY()
{
}

/**
 * User interface(UI) events in DOM Level 2 indicate the screen positions at
 * which the given UI event occurred. When the user agent actually knows the
 * physical size of a "screen unit", this attribute will express that information
{
}
 *  otherwise, user agents will provide a suitable default value such as .28mm.
 */
double getScreenPixelToMillimeterX()
{
}

/**
 * Corresponding size of a screen pixel along the y-axis of the viewport.
 */
double getScreenPixelToMillimeterY()
{
}


/**
 * The initial view(i.e., before magnification and panning) of the current
 * innermost SVG document fragment can be either the "standard" view(i.e., based
 * on attributes on the 'svg' element such as fitBoxToViewport) or to a "custom"
 * view(i.e., a hyperlink into a particular 'view' or other element - see
 * Linking into SVG content: URI fragments and SVG views). If the initial view is
 * the "standard" view, then this attribute is false. If the initial view is a
 * "custom" view, then this attribute is true.
 */
bool getUseCurrentView()
{
}

/**
 * Set the value above
 */
void setUseCurrentView(bool val) throw (DOMException)
{
}

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
SVGViewSpec getCurrentView()
{
}


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
double getCurrentScale()
{
}

/**
 *  Set the value above.
 */
void setCurrentScale(double val) throw (DOMException)
{
}

/**
 * The corresponding translation factor that takes into account
 *      user "magnification".
 */
SVGPoint getCurrentTranslate()
{
}

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
unsigned long suspendRedraw(unsigned long max_wait_milliseconds)
{
}

/**
 * Cancels a specified suspendRedraw() by providing a unique suspend_handle_id.
 */
void unsuspendRedraw(unsigned long suspend_handle_id) throw (DOMException)
{
}

/**
 * Cancels all currently active suspendRedraw() method calls. This method is most
 * useful at the very end of a set of SVG DOM calls to ensure that all pending
 * suspendRedraw() method calls have been cancelled.
 */
void unsuspendRedrawAll()
{
}

/**
 * In rendering environments supporting interactivity, forces the user agent to
 * immediately redraw all regions of the viewport that require updating.
 */
void forceRedraw()
{
}

/**
 * Suspends(i.e., pauses) all currently running animations that are defined
 * within the SVG document fragment corresponding to this 'svg' element, causing
 * the animation clock corresponding to this document fragment to stand still
 * until it is unpaused.
 */
void pauseAnimations()
{
}

/**
 * Unsuspends(i.e., unpauses) currently running animations that are defined
 * within the SVG document fragment, causing the animation clock to continue from
 * the time at which it was suspended.
 */
void unpauseAnimations()
{
}

/**
 * Returns true if this SVG document fragment is in a paused state.
 */
bool animationsPaused()
{
}

/**
 * Returns the current time in seconds relative to the start time for
 *      the current SVG document fragment.
 */
double getCurrentTime()
{
}

/**
 * Adjusts the clock for this SVG document fragment, establishing
 *      a new current time.
 */
void setCurrentTime(double seconds)
{
}

/**
 * Returns the list of graphics elements whose rendered content intersects the
 * supplied rectangle, honoring the 'pointer-events' property value on each
 * candidate graphics element.
 */
NodeList getIntersectionList(const SVGRect &rect,
                             const SVGElementPtr referenceElement)
{
}

/**
 * Returns the list of graphics elements whose rendered content is entirely
 * contained within the supplied rectangle, honoring the 'pointer-events'
 * property value on each candidate graphics element.
 */
NodeList getEnclosureList(const SVGRect &rect,
                          const SVGElementPtr referenceElement)
{
}

/**
 * Returns true if the rendered content of the given element intersects the
 * supplied rectangle, honoring the 'pointer-events' property value on each
 * candidate graphics element.
 */
bool checkIntersection(const SVGElementPtr element, const SVGRect &rect)
{
}

/**
 * Returns true if the rendered content of the given element is entirely
 * contained within the supplied rectangle, honoring the 'pointer-events'
 * property value on each candidate graphics element.
 */
bool checkEnclosure(const SVGElementPtr element, const SVGRect &rect)
{
}

/**
 * Unselects any selected objects, including any selections of text
 *      strings and type-in bars.
 */
void deselectAll()
{
}

/**
 * Creates an SVGNumber object outside of any document trees. The object
 *      is initialized to a value of zero.
 */
SVGNumber createSVGNumber()
{
}

/**
 * Creates an SVGLength object outside of any document trees. The object
 *      is initialized to the value of 0 user units.
 */
SVGLength createSVGLength()
{
}

/**
 * Creates an SVGAngle object outside of any document trees. The object
 *      is initialized to the value 0 degrees(unitless).
 */
SVGAngle createSVGAngle()
{
}

/**
 * Creates an SVGPoint object outside of any document trees. The object
 * is initialized to the point(0,0) in the user coordinate system.
 */
SVGPoint createSVGPoint()
{
}

/**
 * Creates an SVGMatrix object outside of any document trees. The object
 *      is initialized to the identity matrix.
 */
SVGMatrix createSVGMatrix()
{
}

/**
 * Creates an SVGRect object outside of any document trees. The object
 *      is initialized such that all values are set to 0 user units.
 */
SVGRect createSVGRect()
{
}

/**
 * Creates an SVGTransform object outside of any document trees.
 * The object is initialized to an identity matrix transform
 *     (SVG_TRANSFORM_MATRIX).
 */
SVGTransform createSVGTransform()
{
}

/**
 * Creates an SVGTransform object outside of any document trees.
 * The object is initialized to the given matrix transform
 *     (i.e., SVG_TRANSFORM_MATRIX).
 */
SVGTransform createSVGTransformFromMatrix(const SVGMatrix &matrix)
{
}

/**
 * Searches this SVG document fragment(i.e., the search is restricted to a
 * subset of the document tree) for an Element whose id is given by elementId. If
 * an Element is found, that Element is returned. If no such element exists,
 * returns null. Behavior is not defined if more than one element has this id.
 */
ElementPtr getElementById(const DOMString& elementId)
{
}


//####################################################################
//# SVGTextElement
//####################################################################


//####################################################################
//# SVGTextContentElement
//####################################################################


/**
 * Corresponds to attribute textLength on the given element.
 */
SVGAnimatedLength getTextLength()
{
}


/**
 * Corresponds to attribute lengthAdjust on the given element. The value must be
 * one of the length adjust constants specified above.
 */
SVGAnimatedEnumeration getLengthAdjust()
{
}


/**
 * Returns the total number of characters to be rendered within the current
 * element. Includes characters which are included via a 'tref' reference.
 */
long getNumberOfChars()
{
}

/**
 * The total sum of all of the advance values from rendering all of the
 * characters within this element, including the advance value on the glyphs
 *(horizontal or vertical), the effect of properties 'kerning', 'letter-spacing'
 * and 'word-spacing' and adjustments due to attributes dx and dy on 'tspan'
 * elements. For non-rendering environments, the user agent shall make reasonable
 * assumptions about glyph metrics.
 */
double getComputedTextLength()
{
}

/**
 * The total sum of all of the advance values from rendering the specified
 * substring of the characters, including the advance value on the glyphs
 *(horizontal or vertical), the effect of properties 'kerning', 'letter-spacing'
 * and 'word-spacing' and adjustments due to attributes dx and dy on 'tspan'
 * elements. For non-rendering environments, the user agent shall make reasonable
 * assumptions about glyph metrics.
 */
double getSubStringLength(unsigned long charnum, unsigned long nchars)
                                 throw (DOMException)
{
}

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
SVGPoint getStartPositionOfChar(unsigned long charnum) throw (DOMException)
{
}

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
SVGPoint getEndPositionOfChar(unsigned long charnum) throw (DOMException)
{
}

/**
 * Returns a tightest rectangle which defines the minimum and maximum X and Y
 * values in the user coordinate system for rendering the glyph(s) that
 * correspond to the specified character. The calculations assume that all glyphs
 * occupy the full standard glyph cell for the font. If multiple consecutive
 * characters are rendered inseparably(e.g., as a single glyph or a sequence of
 * glyphs), then each of the inseparable characters will return the same extent.
 */
SVGRect getExtentOfChar(unsigned long charnum) throw (DOMException)
{
}

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
double getRotationOfChar(unsigned long charnum) throw (DOMException)
{
}

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
long getCharNumAtPosition(const SVGPoint &point)
{
}

/**
 * Causes the specified substring to be selected just as if the user
 *      selected the substring interactively.
 */
void selectSubString(unsigned long charnum, unsigned long nchars)
                              throw (DOMException)
{
}





//####################################################################
//# SVGTextPathElement
//####################################################################


/**
 * Corresponds to attribute startOffset on the given 'textPath' element.
 */
SVGAnimatedLength getStartOffset()
{
}

/**
 * Corresponds to attribute method on the given 'textPath' element. The value
 * must be one of the method type constants specified above.
 */
SVGAnimatedEnumeration getMethod()
{
}

/**
 * Corresponds to attribute spacing on the given 'textPath' element.
 *  The value must be one of the spacing type constants specified above.
 */
SVGAnimatedEnumeration getSpacing()
{
}


//####################################################################
//# SVGTextPositioningElement
//####################################################################


/**
 * Corresponds to attribute x on the given element.
 */
SVGAnimatedLength getX()
{
}

/**
 * Corresponds to attribute y on the given element.
 */
SVGAnimatedLength getY()
{
}

/**
 * Corresponds to attribute dx on the given element.
 */
SVGAnimatedLength getDx()
{
}

/**
 * Corresponds to attribute dy on the given element.
 */
SVGAnimatedLength getDy()
{
}


/**
 * Corresponds to attribute rotate on the given element.
 */
SVGAnimatedNumberList getRotate()
{
}


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
SVGAnimatedLength getX()
{
}

/**
 * Corresponds to attribute y on the given 'use' element.
 */
SVGAnimatedLength getY()
{
}

/**
 * Corresponds to attribute width on the given 'use' element.
 */
SVGAnimatedLength getWidth()
{
}

/**
 * Corresponds to attribute height on the given 'use' element.
 */
SVGAnimatedLength getHeight()
{
}

/**
 * The root of the "instance tree". See description of SVGElementInstance for
 * a discussion on the instance tree.
 *      */
SVGElementInstance getInstanceRoot()
{
}

/**
 * If the 'href' attribute is being animated, contains the current animated root
 * of the "instance tree". If the 'href' attribute is not currently being
 * animated, contains the same value as 'instanceRoot'. The root of the "instance
 * tree". See description of SVGElementInstance for a discussion on the instance
 * tree.
 */
SVGElementInstance getAnimatedInstanceRoot()
{
}


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
SVGElement::~SVGElement()
{
}




/*#########################################################################
## SVGDocument
#########################################################################*/


/**
 * The title of a document as specified by the title sub-element of the 'svg'
 * root element(i.e., <svg><title>Here is the title</title>...</svg>)
 */
DOMString SVGDocument::getTitle()
{
}

/**
 * Returns the URI of the page that linked to this page. The value is an empty
 * string if the user navigated to the page directly(not through a link, but,
 * for example, via a bookmark).
 */
DOMString SVGDocument::getReferrer()
{
}


/**
 * The domain name of the server that served the document, or a null string if
 * the server cannot be identified by a domain name.
 */
DOMString SVGDocument::getDomain()
{
}


/**
 * The complete URI of the document.
 */
DOMString SVGDocument::getURL()
{
}


/**
 * The root 'svg'  element in the document hierarchy.
 */
SVGElementPtr SVGDocument::getRootElement()
{
}


/**
 * Overloaded from Document
 * 
 */
ElementPtr SVGDocument::createElement(const DOMString &tagName)
{
    ElementPtr ptr;
    return ptr;
}


/**
 * Overloaded from Document
 * 
 */
ElementPtr SVGDocument::createElementNS(const DOMString &tagName,
                                        const DOMString &namespaceURI)
{
    ElementPtr ptr;
    return ptr;
}


/**
 * The root 'svg'  element in the document hierarchy.
 */
SVGElementPtr SVGDocument::getRootElement()
{
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SVGDocument::~SVGDocument()
{
}



/*#########################################################################
## GetSVGDocument
#########################################################################*/


/**
 * Returns the SVGDocument  object for the referenced SVG document.
 */
SVGDocumentPtr GetSVGDocument::getSVGDocument()
                throw (DOMException)
{
    SVGDocumentPtr ptr;
    return ptr;
}

//##################
//# Non-API methods
//##################

/**
 *
 */
GetSVGDocument::~GetSVGDocument()
{
}







}  //namespace svg
}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif // __SVG_H__
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

