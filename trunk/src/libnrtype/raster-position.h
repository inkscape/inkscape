#ifndef SEEN_LIBNRTYPE_RASTER_POSITION_H
#define SEEN_LIBNRTYPE_RASTER_POSITION_H

#include <vector>

#include <libnr/nr-forward.h>
#include <libnrtype/nrtype-forward.h>
#include <livarot/livarot-forward.h>

// one subpixel position
// it's basically a set of trapezoids (=float_ligne_run) representing the black areas of the glyph
// all trapezoids are in the same array, hence the run_on_line array to give the number of 
// trapezoids on each line
// trapezoids store the x-positions as float, and are shifted to the x blit position
// so it's "exact" in the x direction and subpixel in the y direction
class raster_position {
public:
    int               top, bottom; // baseline is y=0
	                       // top is the first pixel, bottom is the last
    int*              run_on_line; // array of size (bottom-top+1): run_on_line[i] gives the number of runs on line top+i
    int               nbRun;
    float_ligne_run*  runs;

public:
    raster_position();
    virtual ~raster_position();

		// stuff runs into the structure
    void AppendRuns(std::vector<float_ligne_run> const &r, int y);
		// blits the trapezoids.
    void Blit(float ph, int pv, NRPixBlock &over);
};


#endif /* !SEEN_LIBNRTYPE_RASTER_POSITION_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
