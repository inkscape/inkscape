#ifndef SEEN_UTEST_TEST_CASES_H
#define SEEN_UTEST_TEST_CASES_H

#include <utest/utest.h>


template<class FArg0, class FArg1, class ValidArg0>
struct Case2 {
    FArg0 f_arg0;   ///< First arg to pass to \a f.
    FArg1 f_arg1;   ///< Second arg to pass to \a f.
    ValidArg0 valid_arg0;  ///< First arg to pass to \a ValidP.
};

/** Often, ValidP can be std::equal_to\<FRet\> (#include \<functional\>), in which case ValidArg0
    should equal FRet. */
template<class FRet, class FArg0, class FArg1, class ValidArg0, class ValidP>
static void
test_2ary_cases(char const test_name[],
                FRet (*f)(FArg0, FArg1),
                unsigned const ncases, Case2<FArg0, FArg1, ValidArg0> const cases[])
{
    ValidP valid;
    UTEST_TEST(test_name) {
        for(unsigned i = 0; i < ncases; ++i) {
            Case2<FArg0, FArg1, ValidArg0> const &c = cases[i];
            FRet const f_ret(f(c.f_arg0, c.f_arg1));
            UTEST_ASSERT(valid(c.valid_arg0, f_ret));
        }
    }
}


#endif /* !SEEN_UTEST_TEST_CASES_H */

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
