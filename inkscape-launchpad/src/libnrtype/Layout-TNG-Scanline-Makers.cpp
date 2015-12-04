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
#include "Layout-TNG-Scanline-Maker.h"
#include "livarot/Shape.h"
#include "livarot/float-line.h"

namespace Inkscape {
namespace Text {

// *********************** infinite version

Layout::InfiniteScanlineMaker::InfiniteScanlineMaker(double initial_x, double initial_y, Layout::Direction block_progression)
{
    _current_line_height.setZero();
    switch (block_progression) {
        case LEFT_TO_RIGHT:
        case RIGHT_TO_LEFT:
            _x = initial_y;
            _y = initial_x;
            break;
        default:
            _x = initial_x;
            _y = initial_y;
            break;
    }
    _negative_block_progression = block_progression == RIGHT_TO_LEFT || block_progression == BOTTOM_TO_TOP;
        
}

Layout::InfiniteScanlineMaker::~InfiniteScanlineMaker()
{
}

std::vector<Layout::ScanlineMaker::ScanRun> Layout::InfiniteScanlineMaker::makeScanline(Layout::FontMetrics const &line_height)
{
    std::vector<ScanRun> runs(1);
    runs[0].x_start = _x;
    runs[0].x_end = FLT_MAX;   // we could use DBL_MAX, but this just seems safer
    runs[0].y = _y;
    _current_line_height = line_height;
    return runs;
}

void Layout::InfiniteScanlineMaker::completeLine()
{
    if (_negative_block_progression)
        _y -= _current_line_height.emSize();
    else
        _y += _current_line_height.emSize();
    _current_line_height.setZero();
}

void Layout::InfiniteScanlineMaker::setNewYCoordinate(double new_y)
{
    _y = new_y;
}

bool Layout::InfiniteScanlineMaker::canExtendCurrentScanline(Layout::FontMetrics const &line_height)
{
    _current_line_height = line_height;
    return true;
}

void Layout::InfiniteScanlineMaker::setLineHeight(Layout::FontMetrics const &line_height)
{
    _current_line_height = line_height;
}

// *********************** real shapes version

Layout::ShapeScanlineMaker::ShapeScanlineMaker(Shape const *shape, Layout::Direction block_progression)
{
    if (block_progression == TOP_TO_BOTTOM) {
        _rotated_shape = const_cast<Shape*>(shape);
        _shape_needs_freeing = false;
    } else {
        Shape *temp_rotated_shape = new Shape;
        _shape_needs_freeing = true;
        temp_rotated_shape->Copy(const_cast<Shape*>(shape));
        switch (block_progression) {
            case BOTTOM_TO_TOP: temp_rotated_shape->Transform(Geom::Affine(1.0, 0.0, 0.0, -1.0, 0.0, 0.0)); break;  // reflect about x axis
            case LEFT_TO_RIGHT: temp_rotated_shape->Transform(Geom::Affine(0.0, 1.0, 1.0, 0.0, 0.0, 0.0)); break;   // reflect about y=x
            case RIGHT_TO_LEFT: temp_rotated_shape->Transform(Geom::Affine(0.0, -1.0, 1.0, 0.0, 0.0, 0.0)); break;  // reflect about y=-x
            default: break;
        }
        _rotated_shape = new Shape;
        _rotated_shape->ConvertToShape(temp_rotated_shape);
        delete temp_rotated_shape;
    }
    _rotated_shape->CalcBBox(true);
    _bounding_box_top = _rotated_shape->topY;
    _bounding_box_bottom = _rotated_shape->bottomY;
    _y = _rasterizer_y = _bounding_box_top;
    _current_rasterization_point = 0;
    _rotated_shape->BeginRaster(_y, _current_rasterization_point);
    _negative_block_progression = block_progression == RIGHT_TO_LEFT || block_progression == BOTTOM_TO_TOP;
}


Layout::ShapeScanlineMaker::~ShapeScanlineMaker()
{
    _rotated_shape->EndRaster();  
    if (_shape_needs_freeing)
        delete _rotated_shape;
}

std::vector<Layout::ScanlineMaker::ScanRun> Layout::ShapeScanlineMaker::makeScanline(Layout::FontMetrics const &line_height)
{
    if (_y > _bounding_box_bottom)
        return std::vector<ScanRun>();

    if (_y < _bounding_box_top)
        _y = _bounding_box_top;

    FloatLigne line_rasterization;
    FloatLigne line_decent_length_runs;
    float line_text_height = (float)(line_height.emSize());
    if (line_text_height == 0.0)
        line_text_height = 0.001;     // Scan() doesn't work for zero height so this will have to do

    _current_line_height = (float)line_height.emSize();

    // I think what's going on here is that we're moving the top of the scanline to the given position...
    _rotated_shape->Scan(_rasterizer_y, _current_rasterization_point, _y, line_text_height);
    // ...then actually retreiving the scanline (which alters the first two parameters)
    _rotated_shape->Scan(_rasterizer_y, _current_rasterization_point, _y + line_text_height , &line_rasterization, true, line_text_height);
    // sanitise the raw rasterisation, which could have weird overlaps 
    line_rasterization.Flatten();
    // line_rasterization.Affiche();
    // cut out runs that cover less than 90% of the line
    line_decent_length_runs.Over(&line_rasterization, 0.9 * line_text_height);

    if (line_decent_length_runs.runs.empty())
    {
        if (line_rasterization.runs.empty())
            return std::vector<ScanRun>();     // stop the flow
        // make up a pointless run: anything that's not an empty vector
        std::vector<ScanRun> result(1);
        result[0].x_start = line_rasterization.runs[0].st;
        result[0].x_end   = line_rasterization.runs[0].st;
        result[0].y = _negative_block_progression ? - _y : _y;
        return result;
    }

    // convert the FloatLigne to what we use: vector<ScanRun>
    std::vector<ScanRun> result(line_decent_length_runs.runs.size());
    for (unsigned i = 0 ; i < result.size() ; i++) {
        result[i].x_start = line_decent_length_runs.runs[i].st;
        result[i].x_end   = line_decent_length_runs.runs[i].en;
        result[i].y = _negative_block_progression ? - _y : _y;
    }

    return result;
}

void Layout::ShapeScanlineMaker::completeLine()
{
    _y += _current_line_height;
}

double Layout::ShapeScanlineMaker::yCoordinate()
{
    if (_negative_block_progression) return - _y;
    return _y;
}

void Layout::ShapeScanlineMaker::setNewYCoordinate(double new_y)
{
    _y = (float)new_y;
    if (_negative_block_progression) _y = - _y;
    // what will happen with the rasteriser if we move off the shape?
    // it's not an important question because <flowSpan> doesn't have a y attribute
}

bool Layout::ShapeScanlineMaker::canExtendCurrentScanline(Layout::FontMetrics const &/*line_height*/)
{
    //we actually could return true if only the leading changed, but that's too much effort for something that rarely happens
    return false;
}

void Layout::ShapeScanlineMaker::setLineHeight(Layout::FontMetrics const &line_height)
{
    _current_line_height = line_height.emSize();
}

}//namespace Text
}//namespace Inkscape
