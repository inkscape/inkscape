/*
 * Inkscape::Text::Layout - text layout engine misc
 *
 * Authors:
 *   Richard Hughes <cyreve@users.sf.net>
 *
 * Copyright (C) 2005 Richard Hughes
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include "Layout-TNG.h"

namespace Inkscape {
namespace Text {

const gunichar Layout::UNICODE_SOFT_HYPHEN = 0x00AD;
const double Layout::LINE_HEIGHT_NORMAL = 1.25;

Layout::Layout() :
    _input_truncated(0),
    _path_fitted(NULL)
{
      textLength._set = false;
      textLengthMultiplier = 1;
      textLengthIncrement = 0;
      lengthAdjust = LENGTHADJUST_SPACING;
}

Layout::~Layout()
{
    clear();
}

void Layout::clear()
{
    _clearInputObjects();
    _clearOutputObjects();

     textLength._set = false;
     textLengthMultiplier = 1;
     textLengthIncrement = 0;
     lengthAdjust = LENGTHADJUST_SPACING;
}

bool Layout::_directions_are_orthogonal(Direction d1, Direction d2)
{
    if (d1 == BOTTOM_TO_TOP) d1 = TOP_TO_BOTTOM;
    if (d2 == BOTTOM_TO_TOP) d2 = TOP_TO_BOTTOM;
    if (d1 == RIGHT_TO_LEFT) d1 = LEFT_TO_RIGHT;
    if (d2 == RIGHT_TO_LEFT) d2 = LEFT_TO_RIGHT;
    return d1 != d2;
}

}//namespace Text
}//namespace Inkscape
