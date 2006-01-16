#ifndef SEEN_UTEST_UTEST_H
#define SEEN_UTEST_UTEST_H

/* Ultra-minimal unit testing framework */
/* This file is in the public domain */

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
//#include <glib/gstrfuncs.h>  /* g_strdup_printf */
#ifdef __cplusplus
};
#endif

jmp_buf utest__jmp_buf;
int utest__tests;
int utest__passed;
int utest__running;
const char *utest__name;

/** \brief Initializes the framework for running a series of tests.
  * \param name A descriptive label for this series of tests.
  */
void utest_start(const char *name) {
	printf("Testing %s...\n", name);
	utest__name = name;
	utest__tests = utest__passed = 0;
	utest__running = 0;
}

void utest__pass(void) {
	utest__passed++;
	utest__running = 0;
	printf("OK\n");
}


/** \brief Write \a a, \a b, \a c, and exit the current block of tests.
 *
 *  In the current implementation, any of \a a, \a b, \a c may be NULL, considered equivalent to
 *  empty string; but don't rely on that unless you also change this documentation string.  (No
 *  callers use this functionality at the time of writing.)
 *
 *  No newline needed in the arguments.
 */
int
utest__fail(const char *a, const char *b, const char *c)
{
	utest__running = 0;
	fflush(stdout);
	fprintf (stderr, "%s%s%s\n",
		 (a ? a : ""),
		 (b ? b : ""),
		 (c ? c : ""));
	fflush(stderr);
	longjmp(utest__jmp_buf, 0);
	return 0;
}


/** \brief Marks a C block constituting a single test.
  * \param name A descriptive name for this test.
  *
  * The block effectively becomes a try statement; if code within the
  * block triggers an assertion, control will resume at the end of the
  * block.
  */
#define UTEST_TEST(name) if (!setjmp(utest__jmp_buf)&&utest__test((name)))

/** \brief Terminates the current test if \a cond evaluates to nonzero.
  * \param cond The condition to test.
  */
#define UTEST_ASSERT(cond) UTEST_NAMED_ASSERT( #cond, (cond))

/** \brief Terminates the current tests if \a _cond evaluates to nonzero,
  *        and prints a descriptive \a _name instead of the condition
  *        that caused it to fail.
  * \param _name The descriptive label to use.
  * \param _cond The condition to test.
  */
#define UTEST_NAMED_ASSERT(_name, _cond) static_cast<void>((_cond) || utest__fail("Assertion `", (_name), "' failed"))

#define UTEST_ASSERT_SHOW(_cond, _printf_args) \
  static_cast<void>((_cond)	\
		    || (utest__fail("\nAssertion `" #_cond "' failed; ", "",	\
				    g_strdup_printf _printf_args)))

int utest__test(const char *name) {
	utest__tests++;
	if (utest__running) {
		utest__pass();
	}
	printf("\t%s...", name);
	fflush(stdout);
	utest__running = 1;
	return 1;
}

/** \brief Ends a series of tests, reporting test statistics.
  *
  * Test statistics are printed to stdout or stderr, then the function returns
  * nonzero iff all the tests have passed, zero otherwise.
  */
int utest_end(void) {
	if (utest__running) {
		utest__pass();
	}
	if ( utest__passed == utest__tests ) {
		printf("%s: OK (all %d passed)\n",
		       utest__name, utest__tests);
		return 1;
	} else {
		fflush(stdout);
		fprintf(stderr, "%s: FAILED (%d/%d tests passed)\n",
		                utest__name, utest__passed, utest__tests);
		fflush(stderr);
		return 0;
	}
}


#endif /* !SEEN_UTEST_UTEST_H */

/*
  Local Variables:
  mode:c
  c-file-style:"linux"
  fill-column:99
  End:
*/
// vim: filetype=c:noexpandtab:shiftwidth=8:tabstop=8:encoding=utf-8:textwidth=99 :
