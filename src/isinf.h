#ifndef __ISINF_H__
#define __ISINF_H__

/*
 * Fix for missing std::isnormal with SOLARIS8/GCC3.2
 */
 
#if defined (SOLARIS_2_8) && __GNUC__ == 3 && __GNUC_MINOR__ == 2

	#include <ieeefp.h>
	#define isinf(x) ((fpclass(x) == FP_NINF) || (fpclass(x) == FP_PINF))
	
#endif

#endif /* __ISINF_H__ */
