#ifndef __COMMON__MACROS__H__
#define __COMMON__MACROS__H__

#define UNUSED_ARG(a) (void)a

#if !defined(max)
	#define max(a, b)  ((a) > (b) ? (a) : (b))
#endif

#if !defined(min)
	#define min(a, b)  ((a) < (b) ? (a) : (b))
#endif

#endif
