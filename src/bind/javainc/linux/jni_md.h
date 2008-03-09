/**
 * This file should be replaced by the "official" jni_md.h
 * for linux
 */
#ifndef __JNI_MD_H__
#define __JNI_MD_H__

/**
 * Nothing special for these declspecs for Linux.  Leave alone.
 */
#define JNIEXPORT
#define JNIIMPORT
#define JNICALL

typedef signed char jbyte;
typedef int jint;

/* 64 bit? */
#ifdef _LP64
typedef long jlong;
#else
typedef long long jlong;
#endif


#endif /* __JNI_MD_H__ */
