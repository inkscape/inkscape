/**
  @file uemf_safe.h
  
  @brief Defintions and prototype for function for converting EMF records between Big Endian and Little Endian byte orders.
*/

/*
File:      uemf_endian.h
Version:   0.0.2
Date:      26-MAR-2015
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2015 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _UEMF_SAFE_
#define _UEMF_SAFE_

#ifdef __cplusplus
extern "C" {
#endif

// prototypes
int U_emf_record_safe(const char *record);
int bitmapinfo_safe(const char *Bmi, const char *blimit);
//! \endcond

#ifdef __cplusplus
}
#endif

#endif /* _UEMF_SAFE_ */
