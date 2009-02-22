#ifndef __NR_BLIT_H__
#define __NR_BLIT_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-pixblock.h>

#define nr_blit_pixblock_pixblock(d,s) nr_blit_pixblock_pixblock_alpha (d, s, 255)

void nr_blit_pixblock_pixblock_alpha (NRPixBlock *d, NRPixBlock *s, unsigned int alpha);
void nr_blit_pixblock_pixblock_mask (NRPixBlock *d, NRPixBlock *s, NRPixBlock *m);
void nr_blit_pixblock_mask_rgba32 (NRPixBlock *d, NRPixBlock *m, unsigned long rgba32);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
