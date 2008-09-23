/** @file
 * @brief Declaration of main internal remove-overlaps function.
 */
/* Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */

#ifndef REMOVE_RECTANGLE_OVERLAP_H_SEEN
#define REMOVE_RECTANGLE_OVERLAP_H_SEEN

namespace vpsc { 
	class Rectangle;
}

void removeRectangleOverlap(unsigned n, vpsc::Rectangle *rs[], double xBorder, double yBorder);


#endif /* !REMOVE_RECTANGLE_OVERLAP_H_SEEN */
