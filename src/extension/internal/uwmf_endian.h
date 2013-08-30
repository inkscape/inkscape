/**
  @file uemf_endian.h Prototype for function for converting EMF records between Big Endian and Little Endian
*/

/*
File:      uwmf_endian.h
Version:   0.0.1
Date:      10-JAN-2013
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2013 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _UWMF_ENDIAN_
#define _UWMF_ENDIAN_

#ifdef __cplusplus
extern "C" {
#endif

#include "uemf_endian.h"

/* There is no way for the preprocessor, in general, to figure out endianness.  So the command line must define
   WORDS_BIGENDIAN for a big endian machine.  Otherwise we assume is is little endian.  If it is something
   else this code won't work in any case.  */
#ifdef  WORDS_BIGENDIAN
#define U_BYTE_SWAP 1
#else
#define U_BYTE_SWAP 0
#endif

// prototypes
int U_wmf_endian(char *contents, size_t length, int torev);

#ifdef __cplusplus
}
#endif

#endif /* _UWMF_ENDIAN_ */
