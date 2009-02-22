#ifndef __NR_COMPOSE_REFERENCE_H__
#define __NR_COMPOSE_REFERENCE_H__

// Based on nr-pixblock.h
typedef enum {
	A8 = 0,
	R8G8B8,
	R8G8B8A8N,
	R8G8B8A8P,
	EMPTY = -1
} PIXEL_FORMAT;

/* FINAL DST SRC */

void nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha);
void nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha);
void nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha);
void nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha);

void nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha);
void nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha);
void nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha);
void nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha);

/* FINAL DST SRC MASK */

void nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_A8_ref (unsigned char *px, int w, int h, int rs,
					const unsigned char *spx, int srs,
					const unsigned char *mpx, int mrs);
void nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_A8_ref (unsigned char *px, int w, int h, int rs,
					const unsigned char *spx, int srs,
					const unsigned char *mpx, int mrs);
void nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_A8_ref (unsigned char *px, int w, int h, int rs,
					const unsigned char *spx, int srs,
					const unsigned char *mpx, int mrs);
void nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_A8_ref (unsigned char *px, int w, int h, int rs,
					const unsigned char *spx, int srs,
					const unsigned char *mpx, int mrs);

void nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_A8_ref (unsigned char *p, int w, int h, int rs,
					     const unsigned char *s, int srs,
					     const unsigned char *m, int mrs);
void nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_A8_ref (unsigned char *p, int w, int h, int rs,
					     const unsigned char *s, int srs,
					     const unsigned char *m, int mrs);
void nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_A8_ref (unsigned char *p, int w, int h, int rs,
					     const unsigned char *s, int srs,
					     const unsigned char *m, int mrs);
void nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_A8_ref (unsigned char *p, int w, int h, int rs,
					     const unsigned char *s, int srs,
					     const unsigned char *m, int mrs);

/* FINAL DST MASK COLOR */

void nr_R8G8B8A8_N_EMPTY_A8_RGBA32_ref (unsigned char *px, int w, int h, int rs, const unsigned char *mpx, int mrs, unsigned long rgba);
void nr_R8G8B8A8_P_EMPTY_A8_RGBA32_ref (unsigned char *px, int w, int h, int rs, const unsigned char *mpx, int mrs, unsigned long rgba);

void nr_R8G8B8_R8G8B8_A8_RGBA32_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned long rgba);
void nr_R8G8B8A8_N_R8G8B8A8_N_A8_RGBA32_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned long rgba);
void nr_R8G8B8A8_P_R8G8B8A8_P_A8_RGBA32_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned long rgba);

/* RGB */

void nr_R8G8B8_R8G8B8_R8G8B8A8_P_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha);
void nr_R8G8B8_R8G8B8_R8G8B8A8_N_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha);
void nr_R8G8B8_R8G8B8_R8G8B8A8_P_A8_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs);
void nr_R8G8B8_R8G8B8_R8G8B8A8_N_A8_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs);

#endif//__NR_COMPOSE_REFERENCE_H__
