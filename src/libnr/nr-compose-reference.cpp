
// This is a reference implementation of the compositing functions in nr-compose.cpp.

#include "nr-compose-reference.h"

#define NR_RGBA32_R(v) (unsigned char) (((v) >> 24) & 0xff)
#define NR_RGBA32_G(v) (unsigned char) (((v) >> 16) & 0xff)
#define NR_RGBA32_B(v) (unsigned char) (((v) >> 8) & 0xff)
#define NR_RGBA32_A(v) (unsigned char) ((v) & 0xff)

static inline unsigned int DIV_ROUND(unsigned int v, unsigned int divisor) { return (v+divisor/2)/divisor; }

static unsigned int pixelSize[] = { 1, 3, 4, 4 };

// Computes :
//   dc' = (1 - alpha*sa) * dc + alpha*sc
//   da' = 1 - (1 - alpha*sa) * (1 - da)
// Assuming premultiplied color values
template<PIXEL_FORMAT resultFormat, PIXEL_FORMAT backgroundFormat, PIXEL_FORMAT foregroundFormat>
static void composePixel(unsigned char *d, const unsigned char *s, unsigned int alpha);

template<> static void composePixel<R8G8B8, R8G8B8, R8G8B8A8N>(unsigned char *d, const unsigned char *s, unsigned int alpha) {
	d[0] = DIV_ROUND((255*255 - alpha*s[3]) * d[0] + alpha*s[3]*s[0], 255*255);
	d[1] = DIV_ROUND((255*255 - alpha*s[3]) * d[1] + alpha*s[3]*s[1], 255*255);
	d[2] = DIV_ROUND((255*255 - alpha*s[3]) * d[2] + alpha*s[3]*s[2], 255*255);
}

template<> static void composePixel<R8G8B8, R8G8B8, R8G8B8A8P>(unsigned char *d, const unsigned char *s, unsigned int alpha) {
	d[0] = DIV_ROUND((255*255 - alpha*s[3]) * d[0] + 255*alpha*s[0], 255*255);
	d[1] = DIV_ROUND((255*255 - alpha*s[3]) * d[1] + 255*alpha*s[1], 255*255);
	d[2] = DIV_ROUND((255*255 - alpha*s[3]) * d[2] + 255*alpha*s[2], 255*255);
}

template<> static void composePixel<R8G8B8A8N, EMPTY, R8G8B8A8N>(unsigned char *d, const unsigned char *s, unsigned int alpha) {
	unsigned int newa = 255*255 - (255*255 - alpha*s[3]);
	d[0] = s[0];//newa == 0 ? 0 : DIV_ROUND(alpha*s[3]*s[0], newa);
	d[1] = s[1];//newa == 0 ? 0 : DIV_ROUND(alpha*s[3]*s[1], newa);
	d[2] = s[2];//newa == 0 ? 0 : DIV_ROUND(alpha*s[3]*s[2], newa);
	d[3] = DIV_ROUND(newa, 255);
}

template<> static void composePixel<R8G8B8A8N, EMPTY, R8G8B8A8P>(unsigned char *d, const unsigned char *s, unsigned int alpha) {
	unsigned int newa = 255*255 - (255*255 - alpha*s[3]);
	d[0] = s[3] == 0 ? 0 : DIV_ROUND(255*s[0], s[3]);//newa == 0 ? 0 : DIV_ROUND(255*alpha*s[0], newa);
	d[1] = s[3] == 0 ? 0 : DIV_ROUND(255*s[1], s[3]);//newa == 0 ? 0 : DIV_ROUND(255*alpha*s[1], newa);
	d[2] = s[3] == 0 ? 0 : DIV_ROUND(255*s[2], s[3]);//newa == 0 ? 0 : DIV_ROUND(255*alpha*s[2], newa);
	d[3] = DIV_ROUND(newa, 255);
}

template<> static void composePixel<R8G8B8A8N, R8G8B8A8N, R8G8B8A8N>(unsigned char *d, const unsigned char *s, unsigned int alpha) {
	if ( d[3] == 0 ) {
		composePixel<R8G8B8A8N, EMPTY, R8G8B8A8N>(d, s, alpha);
	} else if ( alpha*s[3] == 0 ) {
		/* NOP */
	} else {
		unsigned int newa = 255*255*255 - (255*255 - alpha*s[3]) * (255 - d[3]);
		d[0] = DIV_ROUND((255*255 - alpha*s[3]) * d[3]*d[0] + 255 * alpha*s[3]*s[0], newa);
		d[1] = DIV_ROUND((255*255 - alpha*s[3]) * d[3]*d[1] + 255 * alpha*s[3]*s[1], newa);
		d[2] = DIV_ROUND((255*255 - alpha*s[3]) * d[3]*d[2] + 255 * alpha*s[3]*s[2], newa);
		d[3] = DIV_ROUND(newa, 255*255);
	}
}

template<> static void composePixel<R8G8B8A8N, R8G8B8A8N, R8G8B8A8P>(unsigned char *d, const unsigned char *s, unsigned int alpha) {
	if ( d[3] == 0 ) {
		composePixel<R8G8B8A8N, EMPTY, R8G8B8A8P>(d, s, alpha);
	} else if ( alpha*s[3] == 0 ) {
		/* NOP */
	} else {
		unsigned int newa = 255*255*255 - (255*255 - alpha*s[3]) * (255 - d[3]);
		d[0] = DIV_ROUND((255*255 - alpha*s[3]) * d[3]*d[0] + 255*255 * alpha*s[0], newa);
		d[1] = DIV_ROUND((255*255 - alpha*s[3]) * d[3]*d[1] + 255*255 * alpha*s[1], newa);
		d[2] = DIV_ROUND((255*255 - alpha*s[3]) * d[3]*d[2] + 255*255 * alpha*s[2], newa);
		d[3] = DIV_ROUND(newa, 255*255);
	}
}

template<> static void composePixel<R8G8B8A8P, EMPTY, R8G8B8A8N>(unsigned char *d, const unsigned char *s, unsigned int alpha) {
	d[0] = DIV_ROUND(alpha*s[3]*s[0], 255*255);
	d[1] = DIV_ROUND(alpha*s[3]*s[1], 255*255);
	d[2] = DIV_ROUND(alpha*s[3]*s[2], 255*255);
	d[3] = DIV_ROUND(255*255 - (255*255 - alpha*s[3]), 255);
}

template<> static void composePixel<R8G8B8A8P, EMPTY, R8G8B8A8P>(unsigned char *d, const unsigned char *s, unsigned int alpha) {
	d[0] = DIV_ROUND(alpha*s[0], 255);
	d[1] = DIV_ROUND(alpha*s[1], 255);
	d[2] = DIV_ROUND(alpha*s[2], 255);
	d[3] = DIV_ROUND(255*255 - (255*255 - alpha*s[3]), 255);
}

template<> static void composePixel<R8G8B8A8P, R8G8B8A8P, R8G8B8A8N>(unsigned char *d, const unsigned char *s, unsigned int alpha) {
	d[0] = DIV_ROUND((255*255 - alpha*s[3]) * d[0] + alpha*s[3]*s[0], 255*255);
	d[1] = DIV_ROUND((255*255 - alpha*s[3]) * d[1] + alpha*s[3]*s[1], 255*255);
	d[2] = DIV_ROUND((255*255 - alpha*s[3]) * d[2] + alpha*s[3]*s[2], 255*255);
	d[3] = DIV_ROUND(255*255*255 - (255*255 - alpha*s[3]) * (255 - d[3]), 255*255);
}

template<> static void composePixel<R8G8B8A8P, R8G8B8A8P, R8G8B8A8P>(unsigned char *d, const unsigned char *s, unsigned int alpha) {
	d[0] = DIV_ROUND((255*255 - alpha*s[3]) * d[0] + 255 * alpha*s[0], 255*255);
	d[1] = DIV_ROUND((255*255 - alpha*s[3]) * d[1] + 255 * alpha*s[1], 255*255);
	d[2] = DIV_ROUND((255*255 - alpha*s[3]) * d[2] + 255 * alpha*s[2], 255*255);
	d[3] = DIV_ROUND(255*255*255 - (255*255 - alpha*s[3]) * (255 - d[3]), 255*255);
}


// composeAlpha, iterates over all pixels and applies composePixel to each of them
template<PIXEL_FORMAT resultFormat, PIXEL_FORMAT backgroundFormat, PIXEL_FORMAT foregroundFormat>
static void composeAlpha(unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha) {
	for(int y=0; y<h; y++) {
		unsigned char* d = px;
		const unsigned char* s = spx;
		for(int x=0; x<w; x++) {
			composePixel<resultFormat, backgroundFormat, foregroundFormat>(d, s, alpha);
			d += pixelSize[resultFormat];
			s += pixelSize[foregroundFormat];
		}
		px += rs;
		spx += srs;
	}
}

template<PIXEL_FORMAT resultFormat, PIXEL_FORMAT backgroundFormat, PIXEL_FORMAT foregroundFormat>
static void composeMask(unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs) {
	for(int y=0; y<h; y++) {
		unsigned char* d = px;
		const unsigned char* s = spx;
		const unsigned char* m = mpx;
		for(int x=0; x<w; x++) {
			composePixel<resultFormat, backgroundFormat, foregroundFormat>(d, s, *m);
			d += pixelSize[resultFormat];
			s += pixelSize[foregroundFormat];
			m += 1;
		}
		px += rs;
		spx += srs;
		mpx += mrs;
	}
}

template<PIXEL_FORMAT resultFormat, PIXEL_FORMAT backgroundFormat>
static void composeColor(unsigned char *px, int w, int h, int rs, const unsigned char *mpx, int mrs, unsigned long rgba) {
	const unsigned char rgba_array[4] = {NR_RGBA32_R(rgba), NR_RGBA32_G(rgba), NR_RGBA32_B(rgba), NR_RGBA32_A(rgba)};
	for(int y=0; y<h; y++) {
		unsigned char* d = px;
		const unsigned char* m = mpx;
		for(int x=0; x<w; x++) {
			composePixel<resultFormat, backgroundFormat, R8G8B8A8N>(d, rgba_array, *m);
			d += pixelSize[resultFormat];
			m += 1;
		}
		px += rs;
		mpx += mrs;
	}
}

/* FINAL DST SRC */

void nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha) {
	composeAlpha<R8G8B8A8N, EMPTY, R8G8B8A8N>(px, w, h, rs, spx, srs, alpha);
}

void nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha) {
	composeAlpha<R8G8B8A8N, EMPTY, R8G8B8A8P>(px, w, h, rs, spx, srs, alpha);
}

void nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha) {
	composeAlpha<R8G8B8A8P, EMPTY, R8G8B8A8N>(px, w, h, rs, spx, srs, alpha);
}

void nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha) {
	composeAlpha<R8G8B8A8P, EMPTY, R8G8B8A8P>(px, w, h, rs, spx, srs, alpha);
}


void nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha) {
	composeAlpha<R8G8B8A8N, R8G8B8A8N, R8G8B8A8N>(px, w, h, rs, spx, srs, alpha);
}

void nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha) {
	composeAlpha<R8G8B8A8N, R8G8B8A8N, R8G8B8A8P>(px, w, h, rs, spx, srs, alpha);
}

void nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha) {
	composeAlpha<R8G8B8A8P, R8G8B8A8P, R8G8B8A8N>(px, w, h, rs, spx, srs, alpha);
}

void nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha) {
	composeAlpha<R8G8B8A8P, R8G8B8A8P, R8G8B8A8P>(px, w, h, rs, spx, srs, alpha);
}

/* FINAL DST SRC MASK */

void nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_A8_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs) {
	composeMask<R8G8B8A8N, EMPTY, R8G8B8A8N>(px, w, h, rs, spx, srs, mpx, mrs);
}

void nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_A8_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs) {
	composeMask<R8G8B8A8N, EMPTY, R8G8B8A8P>(px, w, h, rs, spx, srs, mpx, mrs);
}

void nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_A8_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs) {
	composeMask<R8G8B8A8P, EMPTY, R8G8B8A8N>(px, w, h, rs, spx, srs, mpx, mrs);
}

void nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_A8_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs) {
	composeMask<R8G8B8A8P, EMPTY, R8G8B8A8P>(px, w, h, rs, spx, srs, mpx, mrs);
}


void nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_A8_ref (unsigned char *p, int w, int h, int rs, const unsigned char *s, int srs, const unsigned char *m, int mrs) {
	composeMask<R8G8B8A8N, R8G8B8A8N, R8G8B8A8N>(p, w, h, rs, s, srs, m, mrs);
}

void nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_A8_ref (unsigned char *p, int w, int h, int rs, const unsigned char *s, int srs, const unsigned char *m, int mrs) {
	composeMask<R8G8B8A8N, R8G8B8A8N, R8G8B8A8P>(p, w, h, rs, s, srs, m, mrs);
}

void nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_A8_ref (unsigned char *p, int w, int h, int rs, const unsigned char *s, int srs, const unsigned char *m, int mrs) {
	composeMask<R8G8B8A8P, R8G8B8A8P, R8G8B8A8N>(p, w, h, rs, s, srs, m, mrs);
}

void nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_A8_ref (unsigned char *p, int w, int h, int rs, const unsigned char *s, int srs, const unsigned char *m, int mrs) {
	composeMask<R8G8B8A8P, R8G8B8A8P, R8G8B8A8P>(p, w, h, rs, s, srs, m, mrs);
}

/* FINAL DST MASK COLOR */

void nr_R8G8B8A8_N_EMPTY_A8_RGBA32_ref (unsigned char *px, int w, int h, int rs, const unsigned char *mpx, int mrs, unsigned long rgba) {
	composeColor<R8G8B8A8N, EMPTY>(px, w, h, rs, mpx, mrs, rgba);
}

void nr_R8G8B8A8_P_EMPTY_A8_RGBA32_ref (unsigned char *px, int w, int h, int rs, const unsigned char *mpx, int mrs, unsigned long rgba) {
	composeColor<R8G8B8A8P, EMPTY>(px, w, h, rs, mpx, mrs, rgba);
}


void nr_R8G8B8_R8G8B8_A8_RGBA32_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned long rgba) {
	composeColor<R8G8B8, R8G8B8>(px, w, h, rs, spx, srs, rgba);
}

void nr_R8G8B8A8_N_R8G8B8A8_N_A8_RGBA32_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned long rgba) {
	composeColor<R8G8B8A8N, R8G8B8A8N>(px, w, h, rs, spx, srs, rgba);
}

void nr_R8G8B8A8_P_R8G8B8A8_P_A8_RGBA32_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned long rgba) {
	composeColor<R8G8B8A8P, R8G8B8A8P>(px, w, h, rs, spx, srs, rgba);
}

/* RGB */

void nr_R8G8B8_R8G8B8_R8G8B8A8_P_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha) {
	composeAlpha<R8G8B8, R8G8B8, R8G8B8A8P>(px, w, h, rs, spx, srs, alpha);
}

void nr_R8G8B8_R8G8B8_R8G8B8A8_N_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha) {
	composeAlpha<R8G8B8, R8G8B8, R8G8B8A8N>(px, w, h, rs, spx, srs, alpha);
}

void nr_R8G8B8_R8G8B8_R8G8B8A8_P_A8_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs) {
	composeMask<R8G8B8, R8G8B8, R8G8B8A8P>(px, w, h, rs, spx, srs, mpx, mrs);
}

void nr_R8G8B8_R8G8B8_R8G8B8A8_N_A8_ref (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs) {
	composeMask<R8G8B8, R8G8B8, R8G8B8A8N>(px, w, h, rs, spx, srs, mpx, mrs);
}
