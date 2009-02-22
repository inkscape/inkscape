#ifndef __ISINF_H__
#define __ISINF_H__

/*
 * Fix for missing std::isnormal with SOLARIS8/GCC3.2
 */
 
#if defined (SOLARIS)

	#include <ieeefp.h>
	#define isinf(x) ((fpclass(x) == FP_NINF) || (fpclass(x) == FP_PINF))
	
#elif defined(__APPLE__) && __GNUC__ == 3
#define isinf(x) __isinf(x)
#endif

#endif /* __ISINF_H__ */
