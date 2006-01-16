#include "removeoverlap/remove_rectangle_overlap.h"
#include <unistd.h>  // for alarm()
#include <time.h>  // for srand seed and clock().
#include <glib/gmacros.h>
#include <glib/gmem.h>
#include <cstdlib>
#include <cmath>
#include "removeoverlap/generate-constraints.h"
#include "utest/utest.h"
using std::abs;
using std::rand;

static bool
possibly_eq(double const a, double const b)
{
    return abs(a - b) < 1e-13;
}

static bool
possibly_le(double const a, double const b)
{
    return a - b < 1e-13;
}

static void
show_rects(unsigned const n_rects, double const rect2coords[][4])
{
    for (unsigned i = 0; i < n_rects; ++i) {
        printf("{%g, %g, %g, %g},\n",
               rect2coords[i][0],
               rect2coords[i][1],
               rect2coords[i][2],
               rect2coords[i][3]);
    }
}

/**
 * Returns the signum of x, but erring towards returning 0 if x is "not too far" from 0.  ("Not too
 * far from 0" means [-0.9, 0.9] in current version.)
 */
static int
sgn0(double const x)
{
    if (x <= -0.9) {
        return -1;
    } else if (0.9 <= x) {
        return 1;
    } else {
        return 0;
    }
}

static void
test_case(unsigned const n_rects, double const rect2coords[][4])
{
    Rectangle **rs = (Rectangle **) g_malloc(sizeof(Rectangle*) * n_rects);
    for (unsigned i = 0; i < n_rects; ++i) {
        rs[i] = new Rectangle(rect2coords[i][0],
                              rect2coords[i][1],
                              rect2coords[i][2],
                              rect2coords[i][3]);
    }
    removeRectangleOverlap(rs, n_rects, 0.0, 0.0);
    for (unsigned i = 0; i < n_rects; ++i) {
        UTEST_ASSERT(possibly_eq(rs[i]->width(), (rect2coords[i][1] -
                                                  rect2coords[i][0]  )));
        UTEST_ASSERT(possibly_eq(rs[i]->height(), (rect2coords[i][3] -
                                                   rect2coords[i][2]  )));
        for (unsigned j = 0; j < i; ++j) {
            if (!( possibly_le(rs[i]->getMaxX(), rs[j]->getMinX()) ||
                   possibly_le(rs[j]->getMaxX(), rs[i]->getMinX()) ||
                   possibly_le(rs[i]->getMaxY(), rs[j]->getMinY()) ||
                   possibly_le(rs[j]->getMaxY(), rs[i]->getMinY())   )) {
                show_rects(n_rects, rect2coords);
                char buf[32];
                sprintf(buf, "[%u],[%u] of %u", j, i, n_rects);
                utest__fail("Found overlap among ", buf, " rectangles");
            }
        }

        /* Optimality test. */
        {
            bool found_block[2] = {false, false};
            int const desired_movement[2] = {sgn0(rect2coords[i][0] - rs[i]->getMinX()),
                                             sgn0(rect2coords[i][2] - rs[i]->getMinY())};
            for (unsigned j = 0; j < n_rects; ++j) {
                if (j == i)
                    continue;
                for (unsigned d = 0; d < 2; ++d) {
                    if ( ( desired_movement[d] < 0
                           ? abs(rs[j]->getMaxD(d) - rs[i]->getMinD(d))
                           : abs(rs[i]->getMaxD(d) - rs[j]->getMinD(d)) )
                         < .002 ) {
                        found_block[d] = true;
                    }
                }
            }

            for (unsigned d = 0; d < 2; ++d) {
                if ( !found_block[d]
                     && desired_movement[d] != 0 ) {
                    show_rects(n_rects, rect2coords);
                    char buf[32];
                    sprintf(buf, "%c in rectangle [%u] of %u", "XY"[d], i, n_rects);
                    utest__fail("Found clear non-optimality in ", buf, " rectangles");
                }
            }
        }
    }
    for (unsigned i = 0; i < n_rects; ++i) {
        delete rs[i];
    }
    g_free(rs);
}

int main()
{
    srand(time(NULL));

    /* Ensure that the program doesn't run for more than 30 seconds. */
    alarm(30);

    utest_start("removeRectangleOverlap(zero gaps)");

    /* Derived from Bulia's initial test case.  This used to crash. */
    UTEST_TEST("eg0") {
        double case0[][4] = {
            {-180.5, 69.072, 368.071, 629.071},
            {99.5, 297.644, 319.5, 449.071},
            {199.5, 483.358, 450.929, 571.929},
            {168.071, 277.644, 462.357, 623.357},
            {99.5, 99.751, 479.5, 674.786},
            {-111.929, 103.358, 453.786, 611.929},
            {-29.0714, 143.358, 273.786, 557.643},
            {122.357, 269.072, 322.357, 531.929},
            {256.643, 357.644, 396.643, 520.5}
        };
        test_case(G_N_ELEMENTS(case0), case0);
    }

#if 0 /* This involves a zero-height rect, so we'll ignore for the moment. */
    UTEST_TEST("eg1") {
        double case1[][4] = {
            {5, 14, 9, 14},
            {6, 13, 6, 8},
            {11, 12, 5, 5},
            {5, 8, 5, 7},
            {12, 14, 14, 15},
            {12, 14, 1, 14},
            {1, 15, 14, 15},
            {5, 6, 13, 13}
        };
        test_case(G_N_ELEMENTS(case1), case1);
    }
#endif

    /* The next few examples used to result in overlaps. */
    UTEST_TEST("eg2") {
        double case2[][4] = {
            {3, 4, 6, 13},
            {0, 1, 0, 5},
            {0, 4, 1, 6},
            {2, 5, 0, 6},
            {0, 10, 9, 13},
            {5, 11, 1, 13},
            {1, 2, 3, 8}
        };
        test_case(G_N_ELEMENTS(case2), case2);
    }

    UTEST_TEST("eg3") {
        double case3[][4] = {
            {0, 5, 0, 3},
            {1, 2, 1, 3},
            {3, 7, 4, 7},
            {0, 9, 4, 5},
            {3, 7, 0, 3}
        };
        test_case(G_N_ELEMENTS(case3), case3);
    }

    UTEST_TEST("eg4") {
        double case4[][4] = {
            {0, 1, 2, 3},
            {0, 4, 0, 4},
            {1, 6, 0, 4},
            {2, 3, 4, 5},
            {0, 5, 4, 6}
        };
        test_case(G_N_ELEMENTS(case4), case4);
    }

    UTEST_TEST("eg5") {
        double case5[][4] = {
            {1, 5, 1, 2},
            {1, 6, 5, 7},
            {6, 8, 1, 2},
            {2, 3, 1, 4},
            {5, 8, 2, 6}
        };
        test_case(G_N_ELEMENTS(case5), case5);
    }

    /* This one causes overlap in 2005-12-19 04:00 UTC version. */
    UTEST_TEST("olap6") {
        double case6[][4] = {
            {7, 22, 39, 54},
            {7, 33, 0, 59},
            {3, 26, 16, 56},
            {7, 17, 18, 20},
            {1, 59, 11, 26},
            {19, 20, 13, 49},
            {1, 10, 0, 4},
            {47, 52, 1, 3}
        };
        test_case(G_N_ELEMENTS(case6), case6);
    }

    /* The next two examples caused loops in the version at 2005-12-07 04:00 UTC. */
    UTEST_TEST("loop0") {
        double loop0[][4] = {
            {13, 16, 6, 27},
            {0, 6, 0, 12},
            {11, 14, 1, 10},
            {12, 39, 5, 24},
            {14, 34, 4, 7},
            {1, 30, 20, 27},
            {1, 6, 1, 2},
            {19, 28, 10, 24},
            {4, 34, 15, 21},
            {7, 13, 13, 34}
        };
        test_case(G_N_ELEMENTS(loop0), loop0);
    }

    UTEST_TEST("loop1") {
        double loop1[][4] = {
            {6, 18, 9, 16},
            {8, 26, 10, 13},
            {3, 10, 0, 14},
            {0, 5, 16, 22},
            {1, 8, 11, 21},
            {1, 5, 0, 13},
            {24, 25, 0, 2}
        };
        test_case(G_N_ELEMENTS(loop1), loop1);
    }

    UTEST_TEST("loop2") {
        double loop2[][4] = {
            {16, 22, 9, 16},
            {8, 9, 14, 19},
            {17, 25, 8, 13},
            {10, 26, 26, 29},
            {14, 19, 9, 19},
            {0, 18, 3, 12},
            {7, 8, 14, 22},
            {14, 20, 25, 29}
        };
        test_case(G_N_ELEMENTS(loop2), loop2);
    }

    /* Random cases of up to 10 rectangles, with small non-neg int coords. */
    for (unsigned n = 0; n <= 10; ++n) {
        char buf[64];
        sprintf(buf, "random ints with %u rectangles", n);
        UTEST_TEST(buf) {
            unsigned const fld_size = 8 * n;
            double (*coords)[4] = (double (*)[4]) g_malloc(n * 4 * sizeof(double));
            clock_t const clock_stop = clock() + CLOCKS_PER_SEC;
            for (unsigned repeat = (n == 0 ? 1
                                    : n == 1 ? 36
                                    : (1 << 16)  ); repeat--;) {
                for (unsigned i = 0; i < n; ++i) {
                    for (unsigned d = 0; d < 2; ++d) {
                        //unsigned const start = rand() % fld_size;
                        //unsigned const end = start + rand() % (fld_size - start);
                        unsigned const end = 1 + (rand() % (fld_size - 1));
                        unsigned const start = rand() % end;
                        coords[i][2 * d] = start;
                        coords[i][2 * d + 1] = end;
                    }
                }
                test_case(n, coords);
                if (clock() >= clock_stop) {
                    break;
                }
            }
            g_free(coords);
        }
    }

    return ( utest_end()
             ? EXIT_SUCCESS
             : EXIT_FAILURE );
}


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
