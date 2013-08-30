/**
  @file uemf_endian.h Prototype for function for converting EMF records between Big Endian and Little Endian
*/

/*
File:      uemf_endian.h
Version:   0.0.3
Date:      24-JUL-2012
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2012 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _UEMF_ENDIAN_
#define _UEMF_ENDIAN_

#ifdef __cplusplus
extern "C" {
#endif

/* There is no way for the preprocessor, in general, to figure out endianness.  So the command line must define
   WORDS_BIGENDIAN for a big endian machine.  Otherwise we assume is is little endian.  If it is something
   else this code won't work in any case.  */
#ifdef  WORDS_BIGENDIAN
#define U_BYTE_SWAP 1
#else
#define U_BYTE_SWAP 0
#endif

// prototypes
int U_emf_endian(char *contents, size_t length, int torev);

#ifdef __cplusplus
}
#endif

#endif /* _UEMF_ENDIAN_ */
