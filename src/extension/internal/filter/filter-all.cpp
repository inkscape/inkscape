/*
 * Copyright (C) 2008 Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "filter.h"

/* Put your filter here */
#include "apparition.h"
#include "bloom.h"
#include "clouds.h"
#include "crystal.h"
#include "cutout.h"
#include "drop-shadow.h"
#include "etched-glass.h"
#include "fire.h"
#include "frost.h"
#include "ink-bleed.h"
#include "jelly-bean.h"
#include "jigsaw-piece.h"
#include "leopard-fur.h"
#include "metal.h"
#include "motion-blur.h"
#include "oil-slick.h"
#include "patterned-glass.h"
#include "ridged-border.h"
#include "ripple.h"
#include "roughen.h"
#include "rubber-stamp.h"
#include "sepia.h"
#include "snow.h"
#include "speckle.h"
#include "zebra.h"


namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Filter {


void
Filter::filters_all (void )
{
	Apparition::init();
	Bloom::init();
	Clouds::init();
	Crystal::init();
	Cutout::init();
	DropShadow::init();
	EtchedGlass::init();
	Fire::init();
	Frost::init();
	InkBleed::init();
	JellyBean::init();
	JigsawPiece::init();
	LeopardFur::init();
	Metal::init();
	MotionBlur::init();
	OilSlick::init();
	PatternedGlass::init();
	RidgedBorder::init();
	Ripple::init();
	Roughen::init();
	RubberStamp::init();
	Sepia::init();
	Snow::init();
	Speckle::init();
	Zebra::init();


	/* This should always be last, don't put stuff below this
	 * line. */
	Filter::filters_all_files();

	return;
}

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
