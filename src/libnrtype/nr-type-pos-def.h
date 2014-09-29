#ifndef __NR_TYPE_POS_DEF_H__
#define __NR_TYPE_POS_DEF_H__

#define NR_POS_STYLE_NORMAL		 0
#define NR_POS_STYLE_ITALIC		 1
#define NR_POS_STYLE_OBLIQUE		 2

#define NR_POS_WEIGHT_THIN		 32
#define NR_POS_WEIGHT_EXTRA_LIGHT	 64
#define NR_POS_WEIGHT_ULTRA_LIGHT	 64
#define NR_POS_WEIGHT_LIGHT		 96
#define NR_POS_WEIGHT_SEMILIGHT		 96
#define NR_POS_WEIGHT_BOOK		128
#define NR_POS_WEIGHT_NORMAL		128
#define NR_POS_WEIGHT_MEDIUM		144
#define NR_POS_WEIGHT_SEMIBOLD		160
#define NR_POS_WEIGHT_DEMIBOLD		160
#define NR_POS_WEIGHT_BOLD		192
#define NR_POS_WEIGHT_ULTRA_BOLD	224
#define NR_POS_WEIGHT_EXTRA_BOLD	224
#define NR_POS_WEIGHT_BLACK		255

#define NR_POS_STRETCH_ULTRA_CONDENSED	  48
#define NR_POS_STRETCH_EXTRA_CONDENSED	  48
#define NR_POS_STRETCH_CONDENSED		  88
#define NR_POS_STRETCH_SEMI_CONDENSED	 108
#define NR_POS_STRETCH_NORMAL	 	 	 128
#define NR_POS_STRETCH_SEMI_EXPANDED	 148
#define NR_POS_STRETCH_EXPANDED	             168
#define NR_POS_STRETCH_EXTRA_EXPANDED	 228
#define NR_POS_STRETCH_ULTRA_EXPANDED	 228

// This is an enumerate, rather than on/off property,
// for I sincerely hope the vocabulary of this property will be
// extended by the W3C in the future to allow for more fancy fonts
#define NR_POS_VARIANT_NORMAL	 	 	 0
#define NR_POS_VARIANT_SMALLCAPS 	 	 1

/* Mapping from CSS weight numbers.

   for i in `seq 9`; do
     if [ $i -le 4 ]; then w=$((32 * $i));
     elif [ $i = 5 ]; then w=144;
     elif [ $i -lt 9 ]; then w=$((32 * $(($i - 1))));
     else w=255;
     fi;
     printf '#define NR_POS_WEIGHT_CSS%d00\t\t%3d\n' $i $w;
   done

   This calculation approximately matches the old to-and-from-text code,
   I don't claim it to be reasonable.  ("approximately": some of the old
   code wrote strings like "semi" and "heavy" that weren't being parsed
   at the other end, and it had CSS100 darker than CSS200.)
 */
#define NR_POS_WEIGHT_CSS100		 32
#define NR_POS_WEIGHT_CSS200		 64
#define NR_POS_WEIGHT_CSS300		 96
#define NR_POS_WEIGHT_CSS400		128
#define NR_POS_WEIGHT_CSS500		144
#define NR_POS_WEIGHT_CSS600		160
#define NR_POS_WEIGHT_CSS700		192
#define NR_POS_WEIGHT_CSS800		224
#define NR_POS_WEIGHT_CSS900		255


class NRTypePosDef {
public:
	unsigned int italic : 1;
	unsigned int oblique : 1;
	unsigned int weight : 8;
	unsigned int stretch : 8;
	unsigned int variant : 8;
	/* These can probably be made sensible sizes rather than bitfields; for the moment we'll
	   keep the old definition. */

public:
	NRTypePosDef() :
	  italic(0),
	  oblique(0),
	  weight(NR_POS_WEIGHT_NORMAL),
	  stretch(NR_POS_STRETCH_NORMAL),
	  variant(NR_POS_VARIANT_NORMAL)
	  { }

	NRTypePosDef(char const *description);

	bool signature() {return this->italic + 
			this->oblique * 255 +
			this->weight * 255 * 255 +
			this->stretch * 255 * 255 * 255 + 
			this->variant * 255 * 255 * 255 * 255;};
};

int parse_name_for_style (char const *c);
int parse_name_for_weight (char const *c);
int parse_name_for_stretch (char const *c);
int parse_name_for_variant (char const *c);
const char *style_to_css (int style);
const char *weight_to_css (int weight);
const char *stretch_to_css (int stretch);
const char *variant_to_css (int variant);

#endif /* __NR_TYPE_POS_DEF_H__ */
