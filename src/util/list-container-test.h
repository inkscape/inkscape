#include <cxxtest/TestSuite.h>

#include <stdarg.h>
#include "util/list-container.h"

using Inkscape::Util::ListContainer;

#define ARRAY_RANGE(array) (array), (array)+sizeof((array))/sizeof((array)[0])

static bool check_values(ListContainer<int> const &c, unsigned n_values, ...) {
    bool ret = true;
    va_list args;
    va_start(args, n_values);
    ListContainer<int>::const_iterator iter(c.begin());
    while ( n_values && iter != c.end() ) {
        int const value = va_arg(args, int);
        if ( value != *iter ) {
            ret = false;
        }
        if ( n_values == 1 && &c.back() != &*iter ) {
            ret = false;
        }
        n_values--;
        ++iter;
    }
    va_end(args);
    return ret && n_values == 0 && iter == c.end();
}

class ListContainerTest : public CxxTest::TestSuite {
public:
    ListContainerTest()
    {
        Inkscape::GC::init();
    }
    virtual ~ListContainerTest() {}

    void testRangeConstructor()
    {
        int const values[]={1,2,3,4};
        int const * const values_end=values+4;
        ListContainer<int> container(values, values_end);

        ListContainer<int>::iterator container_iter=container.begin();
        int const * values_iter=values;

        while ( values_iter != values_end && container_iter != container.end() ) {
            TS_ASSERT_EQUALS(*values_iter , *container_iter);
            ++values_iter;
            ++container_iter;
        }

        TS_ASSERT_EQUALS(values_iter , values_end);
        TS_ASSERT_EQUALS(container_iter , container.end());
    }

    void testEqualityTests()
    {
        int const a[] = { 1, 2, 3, 4 };
        int const b[] = { 1, 2, 3, 4 };
        int const c[] = { 1, 2, 3 };
        int const d[] = { 1, 2, 3, 5 };
        ListContainer<int> c_a(ARRAY_RANGE(a));
        ListContainer<int> c_b(ARRAY_RANGE(b));
        ListContainer<int> c_c(ARRAY_RANGE(c));
        ListContainer<int> c_d(ARRAY_RANGE(d));

        TS_ASSERT(c_a == c_b);
        TS_ASSERT(!( c_a != c_b ));
        TS_ASSERT(!( c_a == c_c ));
        TS_ASSERT(c_a != c_c);
        TS_ASSERT(!( c_a == c_d ));
        TS_ASSERT(c_a != c_d);
    }

    void testLessThan()
    {
        int const a[] = { 1, 2, 3, 4 };
        int const b[] = { 1, 2, 2, 4 };
        int const c[] = { 1, 2, 4, 4 };
        int const d[] = { 1, 2, 3 };
        ListContainer<int> c_a(ARRAY_RANGE(a));
        ListContainer<int> c_b(ARRAY_RANGE(b));
        ListContainer<int> c_c(ARRAY_RANGE(c));
        ListContainer<int> c_d(ARRAY_RANGE(d));
        TS_ASSERT(c_a >= c_b);
        TS_ASSERT(!( c_a < c_b ));
        TS_ASSERT(!( c_a >= c_c ));
        TS_ASSERT(c_a < c_c);
        TS_ASSERT(!( c_a < c_d ));
        TS_ASSERT(c_a >= c_d);
        TS_ASSERT(c_d < c_a);
    }

    void testAssignmentOperator()
    {
        int const a[] = { 1, 2, 3, 4 };
        ListContainer<int> c_a(ARRAY_RANGE(a));
        ListContainer<int> c_c;
        TS_ASSERT(c_a != c_c);
        c_c = c_a;
        TS_ASSERT(c_a == c_c);
        c_c = c_a;
        TS_ASSERT(c_a == c_c);
    }		

    void testFillConstructor()
    {
        ListContainer<int> filled((std::size_t)3, 2);
        TS_ASSERT(check_values(filled, 3, 2, 2, 2));
    }

    void testContainerSize()
    {
        ListContainer<int> empty;
        TS_ASSERT(empty.empty());
        TS_ASSERT_EQUALS(empty.size() , 0);
        int const a[] = { 1, 2, 3 };
        ListContainer<int> c_a(ARRAY_RANGE(a));
        TS_ASSERT(!c_a.empty());
        TS_ASSERT_EQUALS(c_a.size() , 3);

        TS_ASSERT_LESS_THAN(0 , empty.max_size());
    }

    void testAppending()
    {
        ListContainer<int> c;
        c.push_back(1);
        TS_ASSERT(check_values(c, 1, 1));
        c.push_back(2);
        TS_ASSERT(check_values(c, 2, 1, 2));
        c.push_back(3);
        TS_ASSERT(check_values(c, 3, 1, 2, 3));
    }

    void testBulkAppending()
    {
        int const a[] = { 1, 2, 3, 4 };
        int const b[] = { 5, 6, 7 };
        ListContainer<int> c_a(ARRAY_RANGE(a));
        ListContainer<int> c_b(ARRAY_RANGE(b));
        c_a.insert(c_a.end(), c_b.begin(), c_b.end());
        TS_ASSERT(check_values(c_a, 7, 1, 2, 3, 4, 5, 6, 7));
    }

    void testPrepending()
    {
        ListContainer<int> c;
        c.push_front(1);
        TS_ASSERT(check_values(c, 1, 1));
        c.push_front(2);
        TS_ASSERT(check_values(c, 2, 2, 1));
        c.push_front(3);
        TS_ASSERT(check_values(c, 3, 3, 2, 1));
    }

    void testSingleValueInsertion()
    {
        ListContainer<int> c;

        c.insert(c.begin(), 1);
        TS_ASSERT(check_values(c, 1, 1));

        c.insert(c.end(), 2);
        TS_ASSERT(check_values(c, 2, 1, 2));

        c.insert(c.begin(), 3);
        TS_ASSERT(check_values(c, 3, 3, 1, 2));

        ListContainer<int>::iterator pos=c.begin();
        ++pos;
        c.insert(pos, 4);
        TS_ASSERT(check_values(c, 4, 3, 4, 1, 2));
    }

    void testSingleValueErasure()
    {
        int const values[] = { 1, 2, 3, 4 };
        ListContainer<int> c(ARRAY_RANGE(values));

        c.erase(c.begin());
        TS_ASSERT(check_values(c, 3, 2, 3, 4));

        ListContainer<int>::iterator pos=c.begin();
        ++pos;
        c.erase(pos);
        TS_ASSERT(check_values(c, 2, 2, 4));

        pos=c.begin();
        ++pos;
        c.erase(pos);
        TS_ASSERT(check_values(c, 1, 2));

        c.erase(c.begin());
        TS_ASSERT(check_values(c, 0));
    }

    void testPopFront()
    {
        int const full_ary[] = { 1, 2, 3 };
        ListContainer<int> t(ARRAY_RANGE(full_ary));
        TS_ASSERT(check_values(t, 3,  1, 2, 3));
        TS_ASSERT_EQUALS(t.back() , 3);
        t.pop_front();
        TS_ASSERT(check_values(t, 2,  2, 3));
        TS_ASSERT_EQUALS(t.back() , 3);
        t.push_back(23);
        TS_ASSERT(check_values(t, 3,  2, 3, 23));
        TS_ASSERT_EQUALS(t.back() , 23);
        t.pop_front();
        TS_ASSERT(check_values(t, 2,  3, 23));
        TS_ASSERT_EQUALS(t.back() , 23);
        t.pop_front();
        TS_ASSERT(check_values(t, 1,  23));
        TS_ASSERT_EQUALS(t.back() , 23);
        t.pop_front();
        TS_ASSERT(check_values(t, 0));
        t.push_back(42);
        TS_ASSERT(check_values(t, 1,  42));
        TS_ASSERT_EQUALS(t.back() , 42);
    }

    void testEraseAfter()
    {
        int const full_ary[] = { 1, 2, 3, 4 };
        int const exp_ary[] = { 1, 3, 4 };
        ListContainer<int> full_list(ARRAY_RANGE(full_ary));
        ListContainer<int> exp_list(ARRAY_RANGE(exp_ary));
        TS_ASSERT(full_list != exp_list);
        full_list.erase_after(full_list.begin());
        TS_ASSERT(full_list == exp_list);
    }		
};

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
