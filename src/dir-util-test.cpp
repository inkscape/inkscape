#include "utest/test-2ary-cases.h"
#include "dir-util.h"
#include "streq.h"

struct streq_functor {
    bool operator()(char const *a, char const *b)
    {
        return streq(a, b);
    }
};

static bool
test_sp_relative_path_from_path()
{
    utest_start("sp_relative_path_from_path");
    struct Case2<char const *, char const *, char const *> cases[] = {
        {"/foo/bar", "/foo", "bar"},
        {"/foo/barney", "/foo/bar", "/foo/barney"},
        {"/foo/bar/baz", "/foo/", "bar/baz"},
        {"/foo/bar/baz", "/", "foo/bar/baz"},
        {"/foo/bar/baz", "/foo/qux", "/foo/bar/baz"},
        {"/foo", NULL, "/foo"}
    };
    test_2ary_cases<char const *, char const *, char const *, char const *, streq_functor>
        ("sp_relative_path_from_path",
         sp_relative_path_from_path,
         G_N_ELEMENTS(cases), cases);
    return utest_end();
}

int main()
{
    return ( test_sp_relative_path_from_path()
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
