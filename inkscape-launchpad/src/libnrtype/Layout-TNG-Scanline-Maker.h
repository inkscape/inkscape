/*
 * Inkscape::Text::Layout::ScanlineMaker - text layout engine shape measurers
 *
 * Authors:
 *   Richard Hughes <cyreve@users.sf.net>
 *
 * Copyright (C) 2005 Richard Hughes
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef __LAYOUT_TNG_SCANLINE_MAKER_H__
#define __LAYOUT_TNG_SCANLINE_MAKER_H__

#include <vector>
#include <cmath>
#include "libnrtype/Layout-TNG.h"

class Shape;

namespace Inkscape {
namespace Text {

/** \brief private to Layout. Generates lists of chunks within a shape.

This is the abstract base class for taking a given shape and scanning through
it line-by-line to get the horizontal extents of each chunk for a line of a
given height. There are two specialisations: One for real shapes and one that
turns off wrapping by simulating an infinite shape. In due course there will
be a further specialisation to optimise for the common case where the shape
is a rectangle.
*/
class Layout::ScanlineMaker
{
public:
    virtual ~ScanlineMaker() {}

    struct ScanRun {
        double y;  /// that's the top of the scan run, not the baseline
        double x_start;    // these are not flipped according to the text direction
        double x_end;
        inline double width() const {return std::abs(x_start - x_end);}
    };

    /** Returns a list of chunks on the current line which can fit text with
    the given properties. It is up to the caller to discard any chunks which
    are too narrow for its needs. This function may change the y coordinate
    between calls if the new height too big to fit in the space remaining in
    this shape. Returns an empty vector if there is no space left in the
    current shape. */
    virtual std::vector<ScanRun> makeScanline(Layout::FontMetrics const &line_height) =0;

    /** Indicates that the caller has successfully filled the current line
    and hence that the next call to makeScanline() should return lines on
    the next lower line. There is no error return, the next call to
    makeScanline() will give an error if there is no more space. */
    virtual void completeLine() =0;

    /** Returns the y coordinate of the top of the scanline that will be
    returned by the next call to makeScanline(). */
    virtual double yCoordinate() = 0;
    
    /** Forces an arbitrary change in the stored y coordinate of the object.
    The next call to makeScanline() will return runs whose top is at
    the new coordinate. */
    virtual void setNewYCoordinate(double new_y) =0;

    /** Tests whether the caller can fit a new line with the given metrics
    into exactly the space returned by the previous call to makeScanline().
    This saves the caller from having to discard its wrapping solution and
    starting at the beginning of the line again when a larger font is seen.
    The metrics given here are considered to be the ones that are being
    used now, and hence is the line advance height used by completeLine().
    */
    virtual bool canExtendCurrentScanline(Layout::FontMetrics const &line_height) =0;

    /** Sets current line block height. Call before completeLine() to correct for
    actually used line height (in case some chunks with larger font-size rolled back).
    */
    virtual void setLineHeight(Layout::FontMetrics const &line_height) =0;
};

/** \brief private to Layout. Generates infinite scanlines for when you don't want wrapping

This is a 'fake' scanline maker which will always return infinite results,
effectively turning off wrapping. It's a very simple implementation.

It does have the curious property, however, that the input coordinates are
'real' x and y, but the outputs are rotated according to the
\a block_progression.
*/
class Layout::InfiniteScanlineMaker : public Layout::ScanlineMaker
{
public:
    InfiniteScanlineMaker(double initial_x, double initial_y, Layout::Direction block_progression);
    virtual ~InfiniteScanlineMaker();

    /** Returns a single infinite run at the current location */
    virtual std::vector<ScanRun> makeScanline(Layout::FontMetrics const &line_height);

    /** Increments the current y by the current line height */
    virtual void completeLine();

    virtual double yCoordinate()
        {return _y;}

    /** Just changes y */
    virtual void setNewYCoordinate(double new_y);

    /** Always true, but has to save the new height */
    virtual bool canExtendCurrentScanline(Layout::FontMetrics const &line_height);

    /** Sets current line block height. Call before completeLine() to correct for
    actually used line height (in case some chunks with larger font-size rolled back).
    */
    virtual void setLineHeight(Layout::FontMetrics const &line_height);

private:
    double _x, _y;
    Layout::FontMetrics _current_line_height;
    bool _negative_block_progression;     /// if true, indicates that completeLine() should decrement rather than increment, ie block-progression is either rl or bt
};

/** \brief private to Layout. Generates scanlines inside an arbitrary shape

This is the 'perfect', and hence slowest, implementation of a
Layout::ScanlineMaker, which will return exact bounds for any given
input shape.
*/
class Layout::ShapeScanlineMaker : public Layout::ScanlineMaker
{
public:
    ShapeScanlineMaker(Shape const *shape, Layout::Direction block_progression);
    virtual ~ShapeScanlineMaker();

    virtual std::vector<ScanRun> makeScanline(Layout::FontMetrics const &line_height);

    virtual void completeLine();

    virtual double yCoordinate();

    virtual void setNewYCoordinate(double new_y);

    /** never true */
    virtual bool canExtendCurrentScanline(Layout::FontMetrics const &line_height);

    /** Sets current line block height. Call before completeLine() to correct for
    actually used line height (in case some chunks with larger font-size rolled back).
    */
    virtual void setLineHeight(Layout::FontMetrics const &line_height);

private:
    /** To generate scanlines for top-to-bottom text it is easiest if we
    simply rotate the given shape by a multiple of 90 degrees. This stores
    that. If no rotation was needed we can simply store the pointer we were
    given and set shape_needs_freeing appropriately. */
    Shape *_rotated_shape;

    /// see #rotated_shape;
    bool _shape_needs_freeing;

    // Shape::BeginRaster() needs floats rather than doubles
    float _bounding_box_top, _bounding_box_bottom;
    float _y;
    float _rasterizer_y;
    int _current_rasterization_point;
    float _current_line_height;

    bool _negative_block_progression;     /// if true, indicates that completeLine() should decrement rather than increment, ie block-progression is either rl or bt
};

}//namespace Text
}//namespace Inkscape

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
