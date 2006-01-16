#include <cassert>
#include <functional>
#include <glib/gmacros.h>

#include <utest/test-1ary-cases.h>
#include <utest/utest.h>
#include <round.h>

static Case1<double, double> const nonneg_round_cases[] = {
    { 5.0, 5.0 },
    { 0.0, 0.0 },
    { 5.4, 5.0 },
    { 5.6, 6.0 },
    { 1e-7, 0.0 },
    { 1e7 + .49, 1e7 },
    { 1e7 + .51, 1e7 + 1 },
    { 1e12 + .49, 1e12 },
    { 1e12 + .51, 1e12 + 1 },
    { 1e40, 1e40 }
};

static Case1<double, double> nonpos_round_cases[G_N_ELEMENTS(nonneg_round_cases)];

static void fill_nonpos_round_cases()
{
    assert(G_N_ELEMENTS(nonneg_round_cases) == G_N_ELEMENTS(nonpos_round_cases));
    for(unsigned i = 0; i < G_N_ELEMENTS(nonpos_round_cases); ++i) {
        nonpos_round_cases[i].f_arg0 = -nonneg_round_cases[i].f_arg0;
        nonpos_round_cases[i].valid_arg0 = -nonneg_round_cases[i].valid_arg0;
    }
}

static bool
test_round()
{
    utest_start("round");
    test_1ary_cases<double, double, double, std::equal_to<double> >
        ("non-neg round",
         Inkscape::round,
         G_N_ELEMENTS(nonneg_round_cases), nonneg_round_cases);

    fill_nonpos_round_cases();

    test_1ary_cases<double, double, double, std::equal_to<double> >
        ("non-pos round",
         Inkscape::round,
         G_N_ELEMENTS(nonpos_round_cases), nonpos_round_cases);

#if 0
    for(unsigned i = 0; i < G_N_ELEMENTS(round_cases); ++i) {
        RoundCase const &c = round_cases[i];
        double const got = Inkscape::round(c.unrounded);
        UTEST_ASSERT( got == c.exp_rounded );

        double const neg_got = Inkscape::round(-c.unrounded);
        UTEST_ASSERT( neg_got = -c.exp_rounded );
    }
#endif
    return utest_end();
}

#if 0
/* Deliberately down here just to ensure that correct behaviour of Inkscape::round doesn't depend
   on #including decimal-round.h.  (decimal-round.h already #includes round.h, so there's no point
   checking the other way around.) */
#include <decimal-round.h>

static void
test_decimal_round()
{
}
#endif

int main()
{
    int const ret = ( test_round()
                      ? EXIT_SUCCESS
                      : EXIT_FAILURE );
    return ret;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
