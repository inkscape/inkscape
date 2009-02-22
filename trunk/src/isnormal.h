#ifndef __ISNORMAL_H__
#define __ISNORMAL_H__

/*
 * Fix for missing std::isnormal with SOLARIS8/GCC3.2
 */
 
#if defined (SOLARIS)

	#include <ieeefp.h>
	#define isnormal(x) (fpclass(x) >= FP_NZERO)
  
#else

	using std::isnormal;
	
#endif

#endif /* __ISNORMAL_H__ */
