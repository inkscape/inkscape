/*
 * Declaration of main internal remove-overlaps function.
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

/**
 * Takes an array of n rectangles and moves them as little as possible
 * such that rectangles are separated by at least xBorder horizontally
 * and yBorder vertically
 *
 * Works in three passes: 
 * 1) removes some overlap horizontally
 * 2) removes remaining overlap vertically
 * 3) a last horizontal pass removes all overlap starting from original
 *    x-positions - this corrects the case where rectangles were moved 
 *    too much in the first pass.
 */
void removeRectangleOverlap(unsigned n, vpsc::Rectangle *rs[], double xBorder, double yBorder);


#endif /* !REMOVE_RECTANGLE_OVERLAP_H_SEEN */
