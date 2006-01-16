/**
 * Define _gnuc_attribute(blah) as synonym for __attribute__(blah) on gcc,
 * or as nothing on other compilers.
 */
#ifndef _gnuc_attribute
# ifdef __GNUC__
#  define _gnuc_attribute(_attr) __attribute__(_attr)
# else
#  define _gnuc_attribute(_attr)
# endif
#endif
