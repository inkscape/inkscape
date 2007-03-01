#ifndef __ISNORMAL_H__
#define __ISNORMAL_H__

/*
 * Fix for missing std::isnormal with SOLARIS8/GCC3.2
 */
 
#if defined (SOLARIS_2_8) && __GNUC__ == 3 && __GNUC_MINOR__ == 2

	#include <ieeefp.h>
	#define isnormal(x) (fpclass(x) >= FP_NZERO)
  
#else

	using std::isnormal;
	
#endif

#endif /* __ISNORMAL_H__ */
