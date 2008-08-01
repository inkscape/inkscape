#include <stdarg.h>
#include "../utest/utest.h"
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

int main(int /*argc*/, char */*argv*/[]) {
	Inkscape::GC::init();
	utest_start("List Container");
	UTEST_TEST("range constructor") {
		int const values[]={1,2,3,4};
		int const * const values_end=values+4;
		ListContainer<int> container(values, values_end);

		ListContainer<int>::iterator container_iter=container.begin();
		int const * values_iter=values;

		while ( values_iter != values_end && container_iter != container.end() ) {
			UTEST_ASSERT(*values_iter == *container_iter);
			++values_iter;
			++container_iter;
		}

		UTEST_ASSERT(values_iter == values_end);
		UTEST_ASSERT(container_iter == container.end());
	}
	UTEST_TEST("equality tests") {
		int const a[] = { 1, 2, 3, 4 };
		int const b[] = { 1, 2, 3, 4 };
		int const c[] = { 1, 2, 3 };
		int const d[] = { 1, 2, 3, 5 };
		ListContainer<int> c_a(ARRAY_RANGE(a));
		ListContainer<int> c_b(ARRAY_RANGE(b));
		ListContainer<int> c_c(ARRAY_RANGE(c));
		ListContainer<int> c_d(ARRAY_RANGE(d));

		UTEST_ASSERT(c_a == c_b);
		UTEST_ASSERT(!( c_a != c_b ));
		UTEST_ASSERT(!( c_a == c_c ));
		UTEST_ASSERT(c_a != c_c);
		UTEST_ASSERT(!( c_a == c_d ));
		UTEST_ASSERT(c_a != c_d);
	}
	UTEST_TEST("lessthan tests") {
		int const a[] = { 1, 2, 3, 4 };
		int const b[] = { 1, 2, 2, 4 };
		int const c[] = { 1, 2, 4, 4 };
		int const d[] = { 1, 2, 3 };
		ListContainer<int> c_a(ARRAY_RANGE(a));
		ListContainer<int> c_b(ARRAY_RANGE(b));
		ListContainer<int> c_c(ARRAY_RANGE(c));
		ListContainer<int> c_d(ARRAY_RANGE(d));
		UTEST_ASSERT(c_a >= c_b);
		UTEST_ASSERT(!( c_a < c_b ));
		UTEST_ASSERT(!( c_a >= c_c ));
		UTEST_ASSERT(c_a < c_c);
		UTEST_ASSERT(!( c_a < c_d ));
		UTEST_ASSERT(c_a >= c_d);
		UTEST_ASSERT(c_d < c_a);
	}
	UTEST_TEST("assignment operator") {
		int const a[] = { 1, 2, 3, 4 };
		ListContainer<int> c_a(ARRAY_RANGE(a));
		ListContainer<int> c_c;
		UTEST_ASSERT(c_a != c_c);
		c_c = c_a;
		UTEST_ASSERT(c_a == c_c);
		c_c = c_a;
		UTEST_ASSERT(c_a == c_c);
	}		
	UTEST_TEST("fill constructor") {
		ListContainer<int> filled((std::size_t)3, 2);
		UTEST_ASSERT(check_values(filled, 3, 2, 2, 2));
	}
	UTEST_TEST("container size") {
		ListContainer<int> empty;
		UTEST_ASSERT(empty.empty());
		UTEST_ASSERT(empty.size() == 0);
		int const a[] = { 1, 2, 3 };
		ListContainer<int> c_a(ARRAY_RANGE(a));
		UTEST_ASSERT(!c_a.empty());
		UTEST_ASSERT(c_a.size() == 3);

		UTEST_ASSERT(empty.max_size() > 0);
	}
	UTEST_TEST("appending") {
		ListContainer<int> c;
		c.push_back(1);
		UTEST_ASSERT(check_values(c, 1, 1));
		c.push_back(2);
		UTEST_ASSERT(check_values(c, 2, 1, 2));
		c.push_back(3);
		UTEST_ASSERT(check_values(c, 3, 1, 2, 3));
	}
	UTEST_TEST("bulk appending") {
		int const a[] = { 1, 2, 3, 4 };
		int const b[] = { 5, 6, 7 };
		ListContainer<int> c_a(ARRAY_RANGE(a));
		ListContainer<int> c_b(ARRAY_RANGE(b));
		c_a.insert(c_a.end(), c_b.begin(), c_b.end());
		UTEST_ASSERT(check_values(c_a, 7, 1, 2, 3, 4, 5, 6, 7));
	}
	UTEST_TEST("prepending") {
		ListContainer<int> c;
		c.push_front(1);
		UTEST_ASSERT(check_values(c, 1, 1));
		c.push_front(2);
		UTEST_ASSERT(check_values(c, 2, 2, 1));
		c.push_front(3);
		UTEST_ASSERT(check_values(c, 3, 3, 2, 1));
	}
	UTEST_TEST("single-value insertion") {
		ListContainer<int> c;

		c.insert(c.begin(), 1);
		UTEST_ASSERT(check_values(c, 1, 1));

		c.insert(c.end(), 2);
		UTEST_ASSERT(check_values(c, 2, 1, 2));

		c.insert(c.begin(), 3);
		UTEST_ASSERT(check_values(c, 3, 3, 1, 2));

		ListContainer<int>::iterator pos=c.begin();
		++pos;
		c.insert(pos, 4);
		UTEST_ASSERT(check_values(c, 4, 3, 4, 1, 2));
	}
	UTEST_TEST("single-value erasure") {
		int const values[] = { 1, 2, 3, 4 };
		ListContainer<int> c(ARRAY_RANGE(values));

		c.erase(c.begin());
		UTEST_ASSERT(check_values(c, 3, 2, 3, 4));

		ListContainer<int>::iterator pos=c.begin();
		++pos;
		c.erase(pos);
		UTEST_ASSERT(check_values(c, 2, 2, 4));

		pos=c.begin();
		++pos;
		c.erase(pos);
		UTEST_ASSERT(check_values(c, 1, 2));

		c.erase(c.begin());
		UTEST_ASSERT(check_values(c, 0));
	}
	UTEST_TEST("pop_front") {
		int const full_ary[] = { 1, 2, 3 };
		ListContainer<int> t(ARRAY_RANGE(full_ary));
		UTEST_ASSERT(check_values(t, 3,  1, 2, 3));
		UTEST_ASSERT(t.back() == 3);
		t.pop_front();
		UTEST_ASSERT(check_values(t, 2,  2, 3));
		UTEST_ASSERT(t.back() == 3);
		t.push_back(23);
		UTEST_ASSERT(check_values(t, 3,  2, 3, 23));
		UTEST_ASSERT(t.back() == 23);
		t.pop_front();
		UTEST_ASSERT(check_values(t, 2,  3, 23));
		UTEST_ASSERT(t.back() == 23);
		t.pop_front();
		UTEST_ASSERT(check_values(t, 1,  23));
		UTEST_ASSERT(t.back() == 23);
		t.pop_front();
		UTEST_ASSERT(check_values(t, 0));
		t.push_back(42);
		UTEST_ASSERT(check_values(t, 1,  42));
		UTEST_ASSERT(t.back() == 42);
	}		
	UTEST_TEST("erase_after") {
		int const full_ary[] = { 1, 2, 3, 4 };
		int const exp_ary[] = { 1, 3, 4 };
		ListContainer<int> full_list(ARRAY_RANGE(full_ary));
		ListContainer<int> exp_list(ARRAY_RANGE(exp_ary));
		UTEST_ASSERT(full_list != exp_list);
		full_list.erase_after(full_list.begin());
		UTEST_ASSERT(full_list == exp_list);
	}		
	return utest_end() ? 0 : 1;
}
