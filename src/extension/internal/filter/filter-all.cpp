/*
 * Copyright (C) 2008 Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "filter.h"

/* Put your filter here */
#include "abc.h"
#include "blurs.h"
//#include "bumps.h"
#include "color.h"
#include "drop-shadow.h"
#include "image.h"
#include "morphology.h"
#include "shadows.h"
#include "snow.h"

#include "experimental.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Filter {


void
Filter::filters_all (void )
{
	// Here come the filters which are coded in C++ in order to present a parameters dialog
	DropShadow::init();
	DropGlow::init();
	Snow::init();

    /* Experimental custom predefined filters */

    // ABCs
    Blur::init();
    CleanEdges::init();
    ColorShift::init();
    DiffuseLight::init();
    Feather::init();
    MatteJelly::init();
    NoiseFill::init();
    Outline::init();
    Roughen::init();
    Silhouette::init();
    SpecularLight::init();

    // Blurs
    CrossBlur::init();
    
    // Bumps
//    SpecularBump::init();
    
    // Color
    Brightness::init();
    ChannelPaint::init();
    ChannelTransparency::init();
    Colorize::init();
    Duochrome::init();
    Electrize::init();
    Greyscale::init();
    Lightness::init();
    Quadritone::init();
    Solarize::init();
    Tritone::init();

    // Image
    EdgeDetect::init();

    // Morphology
    Crosssmooth::init();

    // Shadows and glows
    ColorizableDropShadow::init();

    // TDB
    Chromolitho::init();
    CrossEngraving::init();
    Drawing::init();
    NeonDraw::init();
    Posterize::init();
    PosterizeBasic::init();

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
