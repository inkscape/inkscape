/**
  @file uemf_endian.h
  
  @brief Defintions and prototype for function for converting EMF records between Big Endian and Little Endian byte orders.
*/

/*
File:      uemf_endian.h
Version:   0.0.4
Date:      24-MAR-2015
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2015 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _UEMF_ENDIAN_
#define _UEMF_ENDIAN_

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup U_Endian Byte order identification

   There is no way for the preprocessor, in general, to figure out endianness.  So the command line must define
   WORDS_BIGENDIAN for a big endian machine.  Otherwise we assume is is little endian.  If it is something
   else this code won't work in any case.

  @{
*/

#ifdef  WORDS_BIGENDIAN
#define U_BYTE_SWAP 1 //!< byte swapping into metafile is required
#define U_IS_BE 1     //!< this machine is big endian
#define U_IS_LE 0     //!< this machine is not little endian
#else
#define U_BYTE_SWAP 0 //!<  byte swapping into metafile is not required
#define U_IS_BE 0     //!<  this machine is not big endian
#define U_IS_LE 1     //!<  this machine is little endian
#endif

#define U_XE    0 //!< do not rearrange endian for target 
#define U_LE    1 //!< target is Little Endian
#define U_BE    2 //!< target is Big   Endian
#define U_RP    4 //!< replicate first instance
#define U_XX 0xFF //!< may be used to terminate a list of these target entries
/** @} */

//! \cond
// prototypes
int U_emf_endian(char *contents, size_t length, int torev);
int U_emf_record_sizeok(const char *record, const char *blimit, uint32_t  *nSize, uint32_t  *iType, int torev);
//! \endcond

#ifdef __cplusplus
}
#endif

#endif /* _UEMF_ENDIAN_ */
