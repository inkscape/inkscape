#include "font-style-to-pos.h"
#include <style.h>

/* 'lighter' and 'darker' have to be resolved earlier */
/**
Given a style struct (CSS representation), sets the corresponding fields in a NRTypePosDef.
 */
NRTypePosDef
font_style_to_pos (SPStyle const &style)
{
	NRTypePosDef ret;

	switch (style.font_weight.computed) {
	case SP_CSS_FONT_WEIGHT_100:
		ret.weight = NR_POS_WEIGHT_CSS100;
		break;

	case SP_CSS_FONT_WEIGHT_200:
		ret.weight = NR_POS_WEIGHT_CSS200;
		break;

	case SP_CSS_FONT_WEIGHT_300:
		ret.weight = NR_POS_WEIGHT_CSS300;
		break;

	case SP_CSS_FONT_WEIGHT_400:
	case SP_CSS_FONT_WEIGHT_NORMAL:
		ret.weight = NR_POS_WEIGHT_CSS400;
		break;

	case SP_CSS_FONT_WEIGHT_500:
		ret.weight = NR_POS_WEIGHT_CSS500;
		break;

	case SP_CSS_FONT_WEIGHT_600:
		ret.weight = NR_POS_WEIGHT_CSS600;
		break;

	case SP_CSS_FONT_WEIGHT_700:
	case SP_CSS_FONT_WEIGHT_BOLD:
		ret.weight = NR_POS_WEIGHT_CSS700;
		break;

	case SP_CSS_FONT_WEIGHT_800:
		ret.weight = NR_POS_WEIGHT_CSS800;
		break;

	case SP_CSS_FONT_WEIGHT_900:
		ret.weight = NR_POS_WEIGHT_CSS900;
		break;

	case SP_CSS_FONT_WEIGHT_LIGHTER:
	case SP_CSS_FONT_WEIGHT_BOLDER:
	default:
		g_warning("Unrecognized font_weight.computed value");
		ret.weight = NR_POS_WEIGHT_NORMAL;
		break;
	}

	switch (style.font_style.computed) {
	case SP_CSS_FONT_STYLE_ITALIC:
		ret.italic = 1;
		break;

	case SP_CSS_FONT_STYLE_OBLIQUE:
		ret.oblique = 1;
		break;

	case SP_CSS_FONT_STYLE_NORMAL:
	default:
		ret.italic = 0;
		ret.oblique = 0;
		break;
	}

	switch (style.font_stretch.computed) {
	case SP_CSS_FONT_STRETCH_ULTRA_CONDENSED:
	case SP_CSS_FONT_STRETCH_EXTRA_CONDENSED:
		ret.stretch = NR_POS_STRETCH_EXTRA_CONDENSED;
		break;

	case SP_CSS_FONT_STRETCH_CONDENSED:
	case SP_CSS_FONT_STRETCH_NARROWER:
		ret.stretch = NR_POS_STRETCH_CONDENSED;
		break;

	case SP_CSS_FONT_STRETCH_SEMI_CONDENSED:
		ret.stretch = NR_POS_STRETCH_SEMI_CONDENSED;
		break;

	case SP_CSS_FONT_STRETCH_SEMI_EXPANDED:
		ret.stretch = NR_POS_STRETCH_SEMI_EXPANDED;
		break;

	case SP_CSS_FONT_STRETCH_EXPANDED:
	case SP_CSS_FONT_STRETCH_WIDER:
		ret.stretch = NR_POS_STRETCH_EXPANDED;
		break;

	case SP_CSS_FONT_STRETCH_EXTRA_EXPANDED:
	case SP_CSS_FONT_STRETCH_ULTRA_EXPANDED:
		ret.stretch = NR_POS_STRETCH_EXTRA_EXPANDED;
		break;

	default:
		ret.stretch = NR_POS_STRETCH_NORMAL;
		break;
	}

	switch (style.font_variant.computed) {
	case SP_CSS_FONT_VARIANT_SMALL_CAPS:
		ret.variant = NR_POS_VARIANT_SMALLCAPS;
		break;
	default:
		ret.variant = NR_POS_VARIANT_NORMAL;
		break;
	}

	return ret;
}
