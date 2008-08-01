#include "../utest/utest.h"
#include <libnr/nr-types.h>
#include <libnr/nr-point-fns.h>
#include <cmath>
using NR::Point;
using NR::X;
using NR::Y;


int main(int /*argc*/, char */*argv*/[]) {
	utest_start("Basic NR::Point operations");

	UTEST_TEST("X,Y values") {
		UTEST_ASSERT(X == 0);
		UTEST_ASSERT(Y == 1);
	}

	NR::Point const a(1.5, 2.0);
	UTEST_TEST("x,y constructor and operator[] const") {
		UTEST_ASSERT(a[X] == 1.5);
		UTEST_ASSERT(a[Y] == 2.0);
	}

	NR::Point const b(-2.0, 3.0);

	UTEST_TEST("copy constructor") {
		NR::Point a_copy(a);
		UTEST_ASSERT(a == a_copy);
		UTEST_ASSERT(!(a != a_copy));
	}

	UTEST_TEST("non-const operator[]") {
		NR::Point a_copy(a);
		a_copy[X] = -2.0;
		UTEST_ASSERT(a_copy != a);
		UTEST_ASSERT(a_copy != b);
		a_copy[Y] = 3.0;
		UTEST_ASSERT(a_copy == b);
	}

	NR::Point const ab(-0.5, 5.0);
	UTEST_TEST("binary +, -") {
		UTEST_ASSERT(a != b);
		UTEST_ASSERT(a + b == ab);
		UTEST_ASSERT(ab - a == b);
		UTEST_ASSERT(ab - b == a);
		UTEST_ASSERT(ab + a != b);
	}

	UTEST_TEST("unary-") {
		UTEST_ASSERT(-a == Point(-a[X], -a[Y]));
	}

	UTEST_TEST("scale, divide") {
		UTEST_ASSERT(-a == -1.0 * a);
		UTEST_ASSERT(a + a + a == 3.0 * a);
		UTEST_ASSERT(a / .5 == 2.0 * a);
	}

	UTEST_TEST("dot") {
		UTEST_ASSERT( dot(a, b) == ( a[X] * b[X]  +
					     a[Y] * b[Y] ) );
		UTEST_ASSERT( dot(a, NR::rot90(a)) == 0.0 );
		UTEST_ASSERT( dot(-a, NR::rot90(a)) == 0.0 );
	}

	double const small = pow(2.0, -1070);

	Point const small_left(-small, 0.0);
	Point const smallish_3_neg4(3.0 * small, -4.0 * small);

	UTEST_TEST("L1, L2, LInfty norms") {
		UTEST_ASSERT(L1(small_left) == small);
		UTEST_ASSERT(L2(small_left) == small);
		UTEST_ASSERT(LInfty(small_left) == small);

		UTEST_ASSERT(L1(smallish_3_neg4) == 7.0 * small);
		UTEST_ASSERT(L2(smallish_3_neg4) == 5.0 * small);
		UTEST_ASSERT(LInfty(smallish_3_neg4) == 4.0 * small);
	}

	UTEST_TEST("operator+=") {
		Point x(a);
		x += b;
		UTEST_ASSERT(x == ab);
	}

	UTEST_TEST("operator/=") {
		Point x(a);
		x /= .5;
		UTEST_ASSERT(x == a + a);
	}

	UTEST_TEST("normalize") {
		Point x(small_left);
		x.normalize();
		UTEST_ASSERT(x == Point(-1.0, 0.0));

		x = smallish_3_neg4;
		x.normalize();
		UTEST_ASSERT(x == Point(0.6, -0.8));
	}

	return utest_end() ? 0 : 1;
}
