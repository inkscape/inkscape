/*
 * Copyright (C) 2008 Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "filter.h"

/* Put your filter here */
#include "blurs.h"
#include "bumps.h"
#include "color.h"
#include "distort.h"
#include "image.h"
#include "morphology.h"
#include "overlays.h"
#include "paint.h"
#include "protrusions.h"
#include "shadows.h"
#include "textures.h"
#include "transparency.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Filter {


void
Filter::filters_all (void )
{
	// Here come the filters which are coded in C++ in order to present a parameters dialog

    /* Experimental custom predefined filters */

    // Blurs
    Blur::init();
    CleanEdges::init();
    CrossBlur::init();
    Feather::init();
    ImageBlur::init();
    
    // Bumps
    Bump::init();
    DiffuseLight::init();
    MatteJelly::init();
    SpecularLight::init();
    WaxBump::init();

    // Color
    Brilliance::init();
    ChannelPaint::init();
    ColorShift::init();
    Colorize::init();
    Duochrome::init();
    ExtractChannel::init();
    Greyscale::init();
    Invert::init();
    Lightness::init();
    Nudge::init();
    Quadritone::init();
    Solarize::init();
    Tritone::init();

    // Distort
    FeltFeather::init();
    Roughen::init();

    // Image effect
    EdgeDetect::init();

    // Image paint and draw
    Chromolitho::init();
    CrossEngraving::init();
    Drawing::init();
    Electrize::init();
    NeonDraw::init();
    PointEngraving::init();
    Posterize::init();
    PosterizeBasic::init();

    // Morphology
    Crosssmooth::init();
    Outline::init();

    // Overlays
    NoiseFill::init();

    // Protrusions
    Snow::init();

    // Shadows and glows
    ColorizableDropShadow::init();

    // Textures
    InkBlot::init();
    
    // Fill and transparency
    Blend::init();
    ChannelTransparency::init();
    Silhouette::init();

	// Here come the rest of the filters that are read from SVG files in share/filters and
	// .config/Inkscape/filters
	/* This should always be last, don't put stuff below this
	 * line. */
	Filter::filters_all_files();

	return;
}

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
