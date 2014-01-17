/**
  @file uemf_utf.h
  
  @brief Prototypes for functions that manipulate UTF and various types of text.

*/

/*
File:      uemf_utf.h
Version:   0.0.1
Date:      04-DEC-2012
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2012 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _UEMF_UTF_
#define _UEMF_UTF_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "uemf_endian.h"

void      wchar8show(const char *src);
void      wchar16show(const uint16_t *src);
void      wchar32show(const uint32_t *src);
void      wchartshow(const wchar_t *src);

size_t    wchar16len(const uint16_t *src);
size_t    wchar32len(const uint32_t *src);
void      wchar16strncpy(uint16_t *dst, const uint16_t *src, size_t nchars);
void      wchar16strncpypad(uint16_t *dst, const uint16_t *src, size_t nchars);
uint16_t *U_Utf8ToUtf16le( const char *src, size_t max, size_t *len );
uint32_t *U_Utf8ToUtf32le( const char *src, size_t max, size_t *len );
uint32_t *U_Latin1ToUtf32le( const char *src, size_t max, size_t *len );
uint16_t *U_Utf32leToUtf16le( const uint32_t *src, size_t max, size_t *len );
char     *U_Utf32leToUtf8( const uint32_t *src, size_t max, size_t *len );
uint32_t *U_Utf16leToUtf32le( const uint16_t *src, size_t max, size_t *len );
char     *U_Utf16leToUtf8( const uint16_t *src, size_t max, size_t *len );
char     *U_Utf16leToLatin1( const uint16_t *src, size_t max, size_t *len );
char     *U_Utf8ToLatin1( const char *src, size_t max, size_t *len );
char     *U_Latin1ToUtf8( const char *src, size_t max, size_t *len );
uint16_t  U_Utf16le(const uint16_t src);
int       U_Utf16leEdit( uint16_t *src, uint16_t find, uint16_t replace );
char     *U_strdup(const char *s);

#ifdef __cplusplus
}
#endif

#endif /* _UEMF_UTF_ */
