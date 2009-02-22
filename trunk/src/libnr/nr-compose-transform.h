#ifndef __NR_COMPOSE_TRANSFORM_H__
#define __NR_COMPOSE_TRANSFORM_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-forward.h>

/* FINAL DST SRC */

void nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_TRANSFORM (unsigned char *px, int w, int h, int rs,
					       const unsigned char *spx, int sw, int sh, int srs,
					       const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);
void nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_TRANSFORM (unsigned char *px, int w, int h, int rs,
					       const unsigned char *spx, int sw, int sh, int srs,
					       const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);
void nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_TRANSFORM (unsigned char *px, int w, int h, int rs,
					       const unsigned char *spx, int sw, int sh, int srs,
					       const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);
void nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_TRANSFORM (unsigned char *px, int w, int h, int rs,
					       const unsigned char *spx, int sw, int sh, int srs,
					       const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);

void nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_TRANSFORM (unsigned char *px, int w, int h, int rs,
						    const unsigned char *spx, int sw, int sh, int srs,
						    const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);
void nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_TRANSFORM (unsigned char *px, int w, int h, int rs,
						    const unsigned char *spx, int sw, int sh, int srs,
						    const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);
void nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM (unsigned char *px, int w, int h, int rs,
						    const unsigned char *spx, int sw, int sh, int srs,
						    const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);
void nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_TRANSFORM (unsigned char *px, int w, int h, int rs,
						    const unsigned char *spx, int sw, int sh, int srs,
						    const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);

#endif
