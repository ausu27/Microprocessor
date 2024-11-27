#ifndef TYPES_H
#define TYPES_H

/* Integer types used for FatFs API */

#if defined(_WIN32)		/* Windows VC++ (for development only) */
#define FF_INTDEF 2
#include <windows.h>
typedef unsigned __int64 QWORD;
#include <float.h>
#define isnan(v) _isnan(v)
#define isinf(v) (!_finite(v))

#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || defined(__cplusplus)	/* C99 or later */
#define FF_INTDEF 2
#include <stdint.h>
typedef unsigned int	UINT;	/* int must be 16-bit or 32-bit */
typedef unsigned char	BYTE;	/* char must be 8-bit */
typedef uint16_t		WORD;	/* 16-bit unsigned */
typedef uint32_t		DWORD;	/* 32-bit unsigned */
typedef uint64_t		QWORD;	/* 64-bit unsigned */
typedef WORD			WCHAR;	/* UTF-16 code unit */

#else  	/* Earlier than C99 */
#define FF_INTDEF 1
typedef unsigned int	UINT;	/* int must be 16-bit or 32-bit */
typedef unsigned char	BYTE;	/* char must be 8-bit */
typedef unsigned short	WORD;	/* short must be 16-bit */
typedef unsigned long	DWORD;	/* long must be 32-bit */
typedef WORD			WCHAR;	/* UTF-16 code unit */
#endif


typedef unsigned char	U08;			// data type definition
typedef   signed char	S08;
typedef unsigned short 	U16;
typedef   signed short	S16;
typedef unsigned int 	U32;
typedef   signed int	S32;



#endif // TYPES_H