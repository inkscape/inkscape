/**
  @file uwmf_endian.h
  
  @brief Prototypes for functions for converting WMF records between Big Endian and Little Endian
*/

/*
File:      uwmf_endian.h
Version:   0.0.3
Date:      28-APR-2015
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2015 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _UWMF_ENDIAN_
#define _UWMF_ENDIAN_

#ifdef __cplusplus
extern "C" {
#endif

#include "uemf_endian.h"

//! \cond
// prototypes
int U_wmf_endian(char *contents, size_t length, int torev, int onerec);
//! \endcond

#ifdef __cplusplus
}
#endif

#endif /* _UWMF_ENDIAN_ */
