/**
  @file uemf_utf.c
  
  @brief Functions for manipulating UTF and various types of text.

  
  Compile with "U_VALGRIND" defined defined to enable code which lets valgrind check each record for
  uninitialized data.
  
  Compile with "SOL8" defined for Solaris 8 or 9 (Sparc).
*/

/*
File:      uemf_utf.c
Version:   0.0.5
Date:      29-JAN-2014
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2014 David Mathog and California Institute of Technology (Caltech)
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iconv.h>
#include <wchar.h>
#include <errno.h>
#include <string.h>
#include <limits.h> // for INT_MAX, INT_MIN
#include <math.h>   // for U_ROUND()
#include "uemf_utf.h"

//! \cond
/* Prototypes for functions used here and defined in uemf_endian.c, but which are not supposed
to be used in end user code. */

void U_swap2(void *ul, unsigned int count);
//! \endcond

/* ******************************************************************************************** */
      
/** \cond */
/* iconv() has a funny cast on some older systems, on most recent ones
   it is just char **.  This tries to work around the issue.  If you build this
   on another funky system this code may need to be modified, or define ICONV_CAST
   on the compile line(but it may be tricky).
*/
#if _LIBICONV_VERSION == 0x0109
# define ICONV_CAST (const char **)
#endif  // _LIBICONV_VERSION 0x0109
#ifdef SOL8
# define ICONV_CAST (const char **)
#endif  //SOL8
#if !defined(ICONV_CAST)
# define ICONV_CAST (char **)
#endif  //ICONV_CAST
/** \endcond */

/* **********************************************************************************************
These functions are used for development and debugging and should be be includied in production code.
*********************************************************************************************** */

/**
    \brief Dump a UTF8  string.  Not for use in production code.
    \param src string to examine
*/
void wchar8show(
      const char *src
   ){
   if(!src){
      printf("char show <NULL>\n");
   }
   else {
      printf("char show\n");
      size_t srclen = 0;
      while(*src){ printf("%d %d %x\n",(int) srclen,*src,*src); srclen++; src++; }
   }
}

/**
    \brief Dump a UTF16  string.  Not for use in production code.
    \param src string to examine
*/
void wchar16show(
      const uint16_t *src
   ){
   if(!src){
      printf("uint16_t show <NULL>\n");
   }
   else {
      printf("uint16_t show\n");
      size_t srclen = 0;
      while(*src){ printf("%d %d %x\n",(int) srclen,*src,*src); srclen++; src++; }
   }
}

/** 
    \brief Dump a UTF32 string.  Not for use in production code.
*/
void wchar32show(
      const uint32_t *src
   ){
   if(!src){
      printf("uint32_t show <NULL>\n");
   }
   else {
      printf("uint32_t show\n");
      size_t srclen = 0;
      while(*src){ printf("%d %d %x\n",(int) srclen,*src,*src); srclen++; src++; }
   }
}

/**
    \brief Dump a wchar_t string.  Not for use in production code.
    \param src string to examine
*/
void wchartshow(
      const wchar_t *src
   ){
   uint32_t val;
   if(!src){
      printf("wchar_t show <NULL>\n");
   }
   else {
      printf("wchar_t show\n");
      size_t srclen = 0;
      if(!src)return;
      while(*src){
         val = *src;  // because *src is wchar_t is not strictly an integer type, can cause warnings on next line
         printf("%d %d %x\n",(int) srclen,val,val); 
         srclen++; 
         src++;
      }
   }
}

/* **********************************************************************************************
These functions are used for character type conversions, Image conversions, and other
utility operations
*********************************************************************************************** */

/**
    \brief Find the number of (storage) characters in a 16 bit character string, not including terminator.
    \param src string to examine
*/
size_t wchar16len(
      const uint16_t *src
   ){
   size_t srclen = 0;
   if(src){
      while(*src){ srclen++; src++; }
   }
   return(srclen);
}

/**
    \brief Find the number of (storage) characters in a 32 bit character  string, not including terminator.
    \param src string to examine
*/
size_t wchar32len(
      const uint32_t *src
   ){
   size_t srclen = 0;
   if(src){
      while(*src){ srclen++; src++; }
   }
   return(srclen);
}

/**
    \brief Strncpy for wchar16 (UTF16).
    \param dst destination (already allocated)  
    \param src source                           
    \param nchars number of characters to copy  
*/
void   wchar16strncpy(
      uint16_t       *dst,
      const uint16_t *src,
      size_t          nchars
   ){
   if(src){
      for(;nchars;nchars--,dst++,src++){
        *dst = *src;
        if(!*src)break;
      }
   }
}

/**
    \brief Fill the output string with N characters, if the input string is shorter than N, pad with nulls.
    \param dst destination (already allocated) 
    \param src source
    \param nchars number of characters to copy 
    
*/
void   wchar16strncpypad(
      uint16_t       *dst,
      const uint16_t *src,
      size_t          nchars
   ){
   if(src){
      for(;*src && nchars;nchars--,dst++,src++){ *dst = *src; }
      for(;nchars;nchars--,dst++){               *dst = 0;    }  // Pad the remainder
   }
}

/*  For the following converstion functions, remember that iconv() modifies ALL of its parameters,
    so save a pointer to the destination buffer!!!!
    It isn't clear that terminators are being
    copied properly, so be sure allocated space is a bit larger and cleared.
*/

/** 
    \brief Convert a UTF32LE string to a UTF16LE string.
    \returns pointer to new string or NULL if it fails
    \param src wchar_t string to convert
    \param max number of characters to convert, if 0, until terminator
    \param len number of characters in new string, NOT including terminator
*/
uint16_t *U_Utf32leToUtf16le(
      const uint32_t *src,
      size_t          max,
      size_t         *len
   ){
   char *dst,*dst2;
   char *src2 = (char *) src;
   size_t srclen,dstlen,status;

   if(!src)return(NULL);
   if(max){ srclen = 4*max; }
   else {   srclen = 4 + 4*wchar32len(src); } //include terminator, length in BYTES
   
   dstlen = 2 + srclen;                     // this will always work, but may waste space
   dst2  = dst = calloc(dstlen,1);          // so there will be at least one terminator
   if(dst){
      iconv_t conv = iconv_open("UTF-16LE", "UTF-32LE");
      if ( conv == (iconv_t) -1){
         free(dst2);
         dst2=NULL;
      }
      else {
         status = iconv(conv, ICONV_CAST &src2, &srclen, &dst, &dstlen);
         iconv_close(conv);
         if(status == (size_t) -1){
            free(dst2);
            dst2 = NULL;
         }
         else if(len){
            *len=wchar16len((uint16_t *)dst2);
         }
      }
   }
   return((uint16_t *)dst2);
}

/**
    \brief  Convert a UTF16LE string to a UTF32LE string.
    \return pointer to new string or NULL if it fails
    \param src UTF16LE string to convert
    \param max number of characters to convert, if 0, until terminator
    \param len number of characters in new string, NOT including terminator
*/
uint32_t *U_Utf16leToUtf32le(
      const uint16_t *src,
      size_t          max,
      size_t         *len
   ){
   char *dst,*dst2;
   char *src2 = (char *) src;
   size_t srclen,dstlen,status;

   if(!src)return(NULL);
   if(max){ srclen = 2*max; }
   else {   srclen = 2*wchar16len(src)+2; } // include terminator, length in BYTES
   dstlen = 2*(2 + srclen);                 // This should always work
   dst2 = dst = calloc(dstlen,1);
   if(dst){
      iconv_t conv = iconv_open("UTF-32LE",   "UTF-16LE");
      if ( conv == (iconv_t) -1){
         free(dst2);
         dst2=NULL;
      }
      else {
         status = iconv(conv, ICONV_CAST &src2, &srclen, &dst, &dstlen);
         iconv_close(conv);
         if(status == (size_t) -1){
            free(dst2);
            dst2 = NULL;
         }
         else if(len){
            *len=wchar32len((uint32_t *)dst2);
         }
      }
   }
   return((uint32_t *) dst2);
}

/**
    \brief  Convert a Latin1 string to a UTF32LE string.
    \return pointer to new string or NULL if it fails
    \param src Latin1 string to convert
    \param max number of characters to convert, if 0, until terminator
    \param len number of characters in new string, NOT including terminator
    
    
    U_EMR_EXTTEXTOUTA records are "8 bit ASCII".  In theory that is ASCII in an 8
    bit character, but numerous applications store Latin1 in them, and some
    _may_ store UTF-8 in them.  Since very vew Latin1 strings are valid UTF-8 strings,
    call U_Utf8ToUtf32le first, and if it fails, then call this function.
*/
uint32_t *U_Latin1ToUtf32le(
      const char *src,
      size_t      max,
      size_t     *len
   ){
   char *dst,*dst2;
   char *src2 = (char *) src;
   size_t srclen,dstlen,status;

   if(!src)return(NULL);
   if(max){ srclen = max; }
   else {   srclen = strlen(src)+1; }       // include terminator, length in BYTES
   dstlen = sizeof(uint32_t)*(1 + srclen);  // This should always work but might waste some space
   dst2 = dst = calloc(dstlen,1);
   if(dst){
      iconv_t conv = iconv_open("UTF-32LE",   "LATIN1");
      if ( conv == (iconv_t) -1){
         free(dst2);
         dst2=NULL;
      }
      else {
         status = iconv(conv, ICONV_CAST &src2, &srclen, &dst, &dstlen);
         iconv_close(conv);
         if(status == (size_t) -1){
            free(dst2);
            dst2 = NULL;
         }
         else if(len){
            *len=wchar32len((uint32_t *)dst2);
         }
      }
   }
   return((uint32_t *) dst2);
}

/**
    \brief  Convert a UTF8 string to a UTF32LE string.
    \return pointer to new string or NULL if it fails
    \param src UTF8 string to convert
    \param max number of characters to convert, if 0, until terminator
    \param len number of characters in new string, NOT including terminator
*/
uint32_t *U_Utf8ToUtf32le(
      const char *src,
      size_t      max,
      size_t     *len
   ){
   char *dst,*dst2;
   char *src2 = (char *) src;
   size_t srclen,dstlen,status;

   if(!src)return(NULL);
   if(max){ srclen = max; }
   else {   srclen = strlen(src)+1; }       // include terminator, length in BYTES
   dstlen = sizeof(uint32_t)*(1 + srclen);  // This should always work but might waste some space
   dst2 = dst = calloc(dstlen,1);
   if(dst){
      iconv_t conv = iconv_open("UTF-32LE",   "UTF-8");
      if ( conv == (iconv_t) -1){
         free(dst2);
         dst2=NULL;
      }
      else {
         status = iconv(conv, ICONV_CAST &src2, &srclen, &dst, &dstlen);
         iconv_close(conv);
         if(status == (size_t) -1){
            free(dst2);
            dst2 = NULL;
         }
         else if(len){
            *len=wchar32len((uint32_t *)dst2);
         }
      }
   }
   return((uint32_t *) dst2);
}

/**
    \brief  Convert a UTF32LE string to a UTF8 string.
    \return pointer to new string or NULL if it fails
    \param src wchar_t string to convert                                       
    \param max number of characters to convert, if 0, until terminator         
    \param len number of characters in new string, NOT including terminator    
*/
char *U_Utf32leToUtf8(
      const uint32_t *src,
      size_t          max,
      size_t         *len
   ){
   char *dst,*dst2;
   char *src2 = (char *) src;
   size_t srclen,dstlen,status;

   if(!src)return(NULL);
   if(max){ srclen = 4*max; }
   else {   srclen = 4*(1 + wchar32len(src)); } //include terminator, length in BYTES
   dstlen = 1 + srclen;                         // This should always work but might waste some space
   dst2 = dst = calloc(dstlen,1);
   if(dst){
      iconv_t conv = iconv_open("UTF-8",   "UTF-32LE");
      if ( conv == (iconv_t) -1){
         free(dst2);
         dst2=NULL;
      }
      else {
         status = iconv(conv, ICONV_CAST &src2, &srclen, &dst, &dstlen);
         iconv_close(conv);
         if(status == (size_t) -1){
            free(dst2);
            dst2 = NULL;
         }
         else if(len){
            *len=strlen(dst2);
         }
      }
   }
   return(dst2);
}

/**
   \brief Convert a UTF-8 string to a UTF16-LE string.
   \return pointer to new string or NULL if it fails
   \param src UTF8 string to convert
   \param max number of characters to convert, if 0, until terminator
   \param len number of characters in new string, NOT including terminator
*/
uint16_t *U_Utf8ToUtf16le(
      const char   *src,
      size_t        max,
      size_t       *len
   ){
   char *dst,*dst2;
   char *src2 = (char *) src;
   size_t srclen,dstlen,status;

   if(!src)return(NULL);
   if(max){ srclen = max; }
   else {   srclen = strlen(src)+1; }       // include terminator, length in BYTES
   dstlen = 2 * (1 + srclen);               // this will always work, but may waste space
   dst2 = dst =calloc(dstlen,1);            // so there will always be a terminator
   if(dst){
      iconv_t conv = iconv_open("UTF-16LE", "UTF-8");
      if ( conv == (iconv_t) -1){
         free(dst2);
         dst2=NULL;
      }
      else {
         status = iconv(conv, ICONV_CAST &src2, &srclen, &dst, &dstlen);
         iconv_close(conv);
         if(status == (size_t) -1){
            free(dst2);
            dst2 = NULL;
         }
         else if(len){
            *len=wchar16len((uint16_t *)dst2);
         }
      }
   }
   return((uint16_t *)dst2);
}

/**
    \brief Convert a UTF16LE string to a UTF8 string.
    \return pointer to new UTF8 string or NULL if it fails
    \param src UTF16LE string to convert
    \param max number of characters to convert, if 0, until terminator
    \param len number of characters in new string, NOT including terminator
*/
char *U_Utf16leToUtf8(
      const uint16_t *src,
      size_t          max,
      size_t         *len
   ){
   char *dst, *dst2;
   char *src2 = (char *) src;
   size_t srclen,dstlen,status;

   if(!src)return(NULL);
   if(max){ srclen = 2*max; }
   else {   srclen = 2*(1 +wchar16len(src)); } //include terminator, length in BYTES
   dstlen = 1 + 2*srclen;                      // this will always work, but may waste space
                                               // worst case is all glyphs (==max) need 4 UTF-8 encoded bytes + terminator.
   dst2 = dst = (char *) calloc(dstlen,1);
   if(dst){
      iconv_t conv = iconv_open("UTF-8", "UTF-16LE");
      if ( conv == (iconv_t) -1){
         free(dst2);
         dst2=NULL;
      }
      else {
         status = iconv(conv, ICONV_CAST &src2, &srclen, &dst, &dstlen);
         iconv_close(conv);
         if(status == (size_t) -1){
            free(dst2);
            dst2 = NULL;
         }
         else if(len){
            *len=strlen(dst2);
            dst = dst2;
            dst2 = U_strdup(dst); // make a string of exactly the right size
            free(dst);            // free the one which was probably too big
         }
      }
   }
   return(dst2);
}

/**
    \brief Convert a UTF16LE string to a LATIN1 string.
    \return pointer to new UTF8 string or NULL if it fails
    \param src UTF16LE string to convert
    \param max number of characters to convert, if 0, until terminator
    \param len number of characters in new string, NOT including terminator
*/
char *U_Utf16leToLatin1(
      const uint16_t *src,
      size_t          max,
      size_t         *len
   ){
   char *dst, *dst2;
   char *src2 = (char *) src;
   size_t srclen,dstlen,status;

   if(!src)return(NULL);
   if(max){ srclen = 2*max; }
   else {   srclen = 2*(1 +wchar16len(src)); } //include terminator, length in BYTES
   dstlen = 1 + srclen;                        // this will always work as latin1 is always 1 byte/character
   dst2 = dst = (char *) calloc(dstlen,1);
   if(dst){
      iconv_t conv = iconv_open("LATIN1//TRANSLIT",   "UTF-16LE");
      if ( conv == (iconv_t) -1){
         free(dst2);
         dst2=NULL;
      }
      else {
         status = iconv(conv, ICONV_CAST &src2, &srclen, &dst, &dstlen);
         iconv_close(conv);
         if(status == (size_t) -1){
            free(dst2);
            dst2 = NULL;
         }
         else if(len){
            *len=strlen(dst2);
            dst = dst2;
            dst2 = U_strdup(dst); // make a string of exactly the right size
            free(dst);            // free the one which was probably too big
         }
      }
   }
   return(dst2);
}
/**
    \brief Put a single 16 bit character into UTF-16LE form.
    
    Used in conjunction with U_Utf16leEdit(), because the character
    representation would otherwise be dependent on machine Endianness.
  
    \return UTF16LE representation of the character.
    \param src 16 bit character
   
*/
uint16_t U_Utf16le(const uint16_t src){
    uint16_t dst=src;
#if U_BYTE_SWAP
    U_swap2(&dst,1);
#endif
    return(dst);
}

/**
    \brief  Convert a UTF8 string to a Latin1 string.
    \return pointer to new string or NULL if it fails
    \param src Latin1 string to convert
    \param max number of characters to convert, if 0, until terminator
    \param len number of characters in new string, NOT including terminator
    
    
    WMF uses latin1, others UTF-8, only some utf-8 can be converted to latin1.
    
*/
char *U_Utf8ToLatin1(
      const char *src,
      size_t      max,
      size_t     *len
   ){
   char *dst,*dst2;
   char *src2 = (char *) src;
   size_t srclen,dstlen,status;
   if(max){ srclen = max; }
   else {   srclen = strlen(src)+1; }       // include terminator, length in BYTES
   dstlen = (1 + srclen);                   // This should always work but might waste some space
   dst2 = dst = calloc(dstlen,1);
   if(dst){
      iconv_t conv = iconv_open("LATIN1//TRANSLIT",   "UTF-8"); // translate what can be, fill in with something close for the rest
      if ( conv == (iconv_t) -1){
         free(dst2);
         dst2=NULL;
      }
      else {
         status = iconv(conv, ICONV_CAST &src2, &srclen, &dst, &dstlen);
         iconv_close(conv);
         if(status == (size_t) -1){
            free(dst2);
            dst2 = NULL;
         }
         else if(len){
            *len=strlen(dst2);
         }
      }
   }
   return((char *) dst2);
}

/**
    \brief  Convert a Latin1 string to a UTF8 string.
    \return pointer to new string or NULL if it fails
    \param src Latin1 string to convert
    \param max number of characters to convert, if 0, until terminator
    \param len number of characters in new string, NOT including terminator
    
    
    WMF uses latin1, others UTF-8, all Latin1 should be able to convert to utf-8.
    
*/
char *U_Latin1ToUtf8(
      const char *src,
      size_t      max,
      size_t     *len
   ){
   char *dst,*dst2;
   char *src2 = (char *) src;
   size_t srclen,dstlen,status;
   if(max){ srclen = max; }
   else {   srclen = strlen(src)+1; }       // include terminator, will waste some space
   dstlen = (1 + 2*srclen);                 // This should always work because all latin1 convert to 1 or 2 byte UTF8, it might waste some space
   dst2 = dst = calloc(dstlen,1);
   if(dst){
      iconv_t conv = iconv_open("UTF-8", "LATIN1"); // everything should translate
      if ( conv == (iconv_t) -1){
         free(dst2);
         dst2=NULL;
      }
      else {
         status = iconv(conv, ICONV_CAST &src2, &srclen, &dst, &dstlen);
         iconv_close(conv);
         if(status == (size_t) -1){
            free(dst2);
            dst2 = NULL;
         }
         else if(len){
            *len=strlen(dst2);
         }
      }
   }
   return((char *) dst2);
}

/**
    \brief Single character replacement in a UTF-16LE string.
    
    Used solely for the Description field which contains
    embedded nulls, which makes it difficult to manipulate.  Use some other character and then swap it.
  
    \return number of substitutions, or -1 if src is not defined 
    \param src UTF16LE string to edit
    \param find character to replace
    \param replace replacestitute character
   
*/
int U_Utf16leEdit(
      uint16_t *src,
      uint16_t  find,
      uint16_t  replace
   ){
   int count=0;
   if(!src)return(-1);
   while(*src){ 
     if(*src == find){ *src = replace; count++; } 
     src++;
   }
   return(count);
}

/**
    \brief strdup for when strict C99 compliance is enforced
    \returns duplicate string or NULL on error
    \param s string to duplicate
*/
char *U_strdup(const char *s){
   char   *news=NULL;
   size_t  slen;
   if(s){
      slen = strlen(s) + 1; //include the terminator!
      news = malloc(slen);
      if(news){
         memcpy(news,s,slen);
      }
   }
   return(news);
   
}


#ifdef __cplusplus
}
#endif
