#ifndef __NR_FILTER_TURBULENCE_H__
#define __NR_FILTER_TURBULENCE_H__

/*
 * feTurbulence filter primitive renderer
 *
 * Authors:
 *   Felipe Sanches <felipe.sanches@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "libnr/nr-rect-l.h"

namespace NR {

enum FilterTurbulenceType {
    TURBULENCE_FRACTALNOISE,
    TURBULENCE_TURBULENCE,
    TURBULENCE_ENDTYPE
};

struct StitchInfo
{
  int nWidth; // How much to subtract to wrap for stitching.
  int nHeight;
  int nWrapX; // Minimum value to wrap.
  int nWrapY;
};

/* Produces results in the range [1, 2**31 - 2].
Algorithm is: r = (a * r) mod m
where a = 16807 and m = 2**31 - 1 = 2147483647
See [Park & Miller], CACM vol. 31 no. 10 p. 1195, Oct. 1988
To test: the algorithm should produce the result 1043618065
as the 10,000th generated number if the original seed is 1.
*/
#define RAND_m 2147483647 /* 2**31 - 1 */
#define RAND_a 16807 /* 7**5; primitive root of m */
#define RAND_q 127773 /* m / a */
#define RAND_r 2836 /* m % a */
#define BSize 0x100
#define BM 0xff
#define PerlinN 0x1000
#define NP 12 /* 2^PerlinN */
#define NM 0xfff
#define s_curve(t) ( t * t * (3. - 2. * t) )
#define turb_lerp(t, a, b) ( a + t * (b - a) )

class FilterTurbulence : public FilterPrimitive {
public:
    FilterTurbulence();
    static FilterPrimitive *create();
    virtual ~FilterTurbulence();

    virtual int render(FilterSlot &slot, FilterUnits const &units);
    void update_pixbuffer(IRect &area, FilterUnits const &units);
    void render_area(NRPixBlock *pix, IRect &full_area, FilterUnits const &units);

    void set_baseFrequency(int axis, double freq);
    void set_numOctaves(int num);
    void set_seed(double s);
    void set_stitchTiles(bool st);
    void set_type(FilterTurbulenceType t);
    void set_updated(bool u);
    virtual FilterTraits get_input_traits();
private:

    long Turbulence_setup_seed(long lSeed);
    long TurbulenceRandom(long lSeed);
    void TurbulenceInit(long lSeed);
    double TurbulenceNoise2(int nColorChannel, double vec[2], StitchInfo *pStitchInfo);
    double turbulence(int nColorChannel, double *point);

    double XbaseFrequency, YbaseFrequency;
    int numOctaves;
    double seed;
    bool stitchTiles;
    FilterTurbulenceType type;
    bool updated;
    IRect updated_area;
    NRPixBlock *pix;
    unsigned char *pix_data;

    int uLatticeSelector[BSize + BSize + 2];
    double fGradient[4][BSize + BSize + 2][2];

    double fTileWidth;
    double fTileHeight;

    double fTileX;
    double fTileY;
};

} /* namespace NR */

#endif /* __NR_FILTER_TURBULENCE_H__ */
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
