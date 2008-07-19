
#include <cxxtest/TestSuite.h>

#include "nr-compose.h"
#include "nr-compose-reference.h"
#include <glib.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

static inline unsigned int DIV_ROUND(unsigned int v, unsigned int divisor) { return (v+divisor/2)/divisor; }
static inline unsigned char NR_PREMUL_111(unsigned int c, unsigned int a) { return static_cast<unsigned char>(DIV_ROUND(c*a, 255)); }

template<PIXEL_FORMAT format>
static int IMGCMP(const unsigned char* a, const unsigned char* b, size_t n) { return memcmp(a, b, n); }

template<>
static int IMGCMP<R8G8B8A8N>(const unsigned char* a, const unsigned char* b, size_t n)
{
    // If two pixels each have their alpha channel set to zero they're equivalent
    //   Note that this doesn't work for premultiplied values, as their color values should
    //   be zero when alpha is zero.
    int cr = 0;
    while(n && cr == 0) {
        if ( a[3] != 0 || b[3] != 0 ) {
            cr = memcmp(a, b, 4);
        }
        a+=4;
        b+=4;
        n-=4;
    }
    return cr;
}

class NrComposeTest : public CxxTest::TestSuite {
private:
    int const w, h;

    unsigned char* const dst_rgba_n_org;
    unsigned char* const dst_rgba_p_org;
    unsigned char* const dst_rgb_org;

    unsigned char* const dst1_rgba;
    unsigned char* const dst2_rgba;
    unsigned char* const src_rgba_n;
    unsigned char* const src_rgba_p;
    unsigned char* const dst1_rgb;
    unsigned char* const dst2_rgb;
    unsigned char* const src_rgb;
    unsigned char* const mask;

    static unsigned int const alpha_vals[7];
    static unsigned int const rgb_vals[3];

public:
    NrComposeTest() :
        w(13),
        h(5),

        dst_rgba_n_org(new unsigned char[w*h*4]),
        dst_rgba_p_org(new unsigned char[w*h*4]),
        dst_rgb_org(new unsigned char[w*h*3]),

        dst1_rgba(new unsigned char[w*h*4]),
        dst2_rgba(new unsigned char[w*h*4]),
        src_rgba_n(new unsigned char[w*h*4]),
        src_rgba_p(new unsigned char[w*h*4]),
        dst1_rgb(new unsigned char[w*h*3]),
        dst2_rgb(new unsigned char[w*h*3]),
        src_rgb(new unsigned char[w*h*3]),
        mask(new unsigned char[w*h])
    {
        srand(23874683); // It shouldn't really matter what this is, as long as it's always the same (to be reproducible)

        for(int y=0; y<h; y++) {
            for(int x=0; x<w; x++) {
                dst_rgba_n_org[(x+y*w)*4+3] = 255*rand()/RAND_MAX;
                dst_rgba_p_org[(x+y*w)*4+3] = 255*rand()/RAND_MAX;
                src_rgba_n[(x+y*w)*4+3] = 255*rand()/RAND_MAX;
                src_rgba_p[(x+y*w)*4+3] = 255*rand()/RAND_MAX;
                for(int i=0; i<3; i++) {
                    dst_rgba_n_org[(x+y*w)*4+i] = 255*rand()/RAND_MAX;
                    dst_rgba_p_org[(x+y*w)*4+i] = NR_PREMUL_111(255*rand()/RAND_MAX, dst_rgba_p_org[(x+y*w)*4+3]);
                    src_rgba_n[(x+y*w)*4+i] = 255*rand()/RAND_MAX;
                    src_rgba_p[(x+y*w)*4+i] = NR_PREMUL_111(255*rand()/RAND_MAX, src_rgba_p[(x+y*w)*4+3]);
                    dst_rgb_org[(x+y*w)*3+i] = 255*rand()/RAND_MAX;
                }
                mask[x+y*w] = 255*rand()/RAND_MAX;
            }
        }
    }
    virtual ~NrComposeTest()
    {
        delete[] dst_rgba_n_org;
        delete[] dst_rgba_p_org;
        delete[] dst_rgb_org;

        delete[] dst1_rgba;
        delete[] dst2_rgba;
        delete[] src_rgba_n;
        delete[] src_rgba_p;
        delete[] dst1_rgb;
        delete[] dst2_rgb;
        delete[] src_rgb;
        delete[] mask;
    }

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static NrComposeTest *createSuite() { return new NrComposeTest(); }
    static void destroySuite( NrComposeTest *suite ) { delete suite; }

    // FINAL DST SRC

    void testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_N()
    {
        for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
            unsigned int alpha = alpha_vals[i];
            char msg[40];
            sprintf(msg, "alpha = %u", alpha);
            memcpy(dst1_rgba, dst_rgba_n_org, w*h*4);
            memcpy(dst2_rgba, dst_rgba_n_org, w*h*4);
            nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N(dst1_rgba, w, h, w*4, src_rgba_n, w*4, alpha);
            nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_ref(dst2_rgba, w, h, w*4, src_rgba_n, w*4, alpha);
            TSM_ASSERT(msg, IMGCMP<R8G8B8A8N>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
        }
    }

    void testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_P()
    {
        for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
            unsigned int alpha = alpha_vals[i];
            char msg[40];
            sprintf(msg, "alpha = %u", alpha);
            memcpy(dst1_rgba, dst_rgba_n_org, w*h*4);
            memcpy(dst2_rgba, dst_rgba_n_org, w*h*4);
            nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P(dst1_rgba, w, h, w*4, src_rgba_p, w*4, alpha);
            nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_ref(dst2_rgba, w, h, w*4, src_rgba_p, w*4, alpha);
            TSM_ASSERT(msg, IMGCMP<R8G8B8A8N>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
        }
    }

    void testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_N()
    {
        for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
            unsigned int alpha = alpha_vals[i];
            char msg[40];
            sprintf(msg, "alpha = %u", alpha);
            memcpy(dst1_rgba, dst_rgba_p_org, w*h*4);
            memcpy(dst2_rgba, dst_rgba_p_org, w*h*4);
            nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N(dst1_rgba, w, h, w*4, src_rgba_n, w*4, alpha);
            nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_ref(dst2_rgba, w, h, w*4, src_rgba_n, w*4, alpha);
            TSM_ASSERT(msg, IMGCMP<R8G8B8A8P>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
        }
    }

    void testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_P()
    {
        for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
            unsigned int alpha = alpha_vals[i];
            char msg[40];
            sprintf(msg, "alpha = %u", alpha);
            memcpy(dst1_rgba, dst_rgba_p_org, w*h*4);
            memcpy(dst2_rgba, dst_rgba_p_org, w*h*4);
            nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P(dst1_rgba, w, h, w*4, src_rgba_p, w*4, alpha);
            nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_ref(dst2_rgba, w, h, w*4, src_rgba_p, w*4, alpha);
            TSM_ASSERT(msg, IMGCMP<R8G8B8A8P>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
        }
    }

    void testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N()
    {
        for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
            unsigned int alpha = alpha_vals[i];
            char msg[40];
            sprintf(msg, "alpha = %u", alpha);
            memcpy(dst1_rgba, dst_rgba_n_org, w*h*4);
            memcpy(dst2_rgba, dst_rgba_n_org, w*h*4);
            nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N(dst1_rgba, w, h, w*4, src_rgba_n, w*4, alpha);
            nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_ref(dst2_rgba, w, h, w*4, src_rgba_n, w*4, alpha);
            TSM_ASSERT(msg, IMGCMP<R8G8B8A8N>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
        }
    }

    void testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P()
    {
        for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
            unsigned int alpha = alpha_vals[i];
            char msg[40];
            sprintf(msg, "alpha = %u", alpha);
            memcpy(dst1_rgba, dst_rgba_n_org, w*h*4);
            memcpy(dst2_rgba, dst_rgba_n_org, w*h*4);
            nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P(dst1_rgba, w, h, w*4, src_rgba_p, w*4, alpha);
            nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_ref(dst2_rgba, w, h, w*4, src_rgba_p, w*4, alpha);
            TSM_ASSERT(msg, IMGCMP<R8G8B8A8N>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
        }
    }

    void testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N()
    {
        for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
            unsigned int alpha = alpha_vals[i];
            char msg[40];
            sprintf(msg, "alpha = %u", alpha);
            memcpy(dst1_rgba, dst_rgba_p_org, w*h*4);
            memcpy(dst2_rgba, dst_rgba_p_org, w*h*4);
            nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N(dst1_rgba, w, h, w*4, src_rgba_n, w*4, alpha);
            nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_ref(dst2_rgba, w, h, w*4, src_rgba_n, w*4, alpha);
            TSM_ASSERT(msg, IMGCMP<R8G8B8A8P>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
        }
    }

    void testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P()
    {
        for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
            unsigned int alpha = alpha_vals[i];
            char msg[40];
            sprintf(msg, "alpha = %u", alpha);
            memcpy(dst1_rgba, dst_rgba_p_org, w*h*4);
            memcpy(dst2_rgba, dst_rgba_p_org, w*h*4);
            nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P(dst1_rgba, w, h, w*4, src_rgba_p, w*4, alpha);
            nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_ref(dst2_rgba, w, h, w*4, src_rgba_p, w*4, alpha);
            TSM_ASSERT(msg, IMGCMP<R8G8B8A8P>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
        }
    }

    // FINAL DST SRC MASK

    void testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_A8()
    {
        memcpy(dst1_rgba, dst_rgba_n_org, w*h*4);
        memcpy(dst2_rgba, dst_rgba_n_org, w*h*4);
        nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_A8(dst1_rgba, w, h, w*4, src_rgba_n, w*4, mask, w);
        nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_A8_ref(dst2_rgba, w, h, w*4, src_rgba_n, w*4, mask, w);
        TS_ASSERT( IMGCMP<R8G8B8A8N>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
    }

    void testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_A8()
    {
        memcpy(dst1_rgba, dst_rgba_n_org, w*h*4);
        memcpy(dst2_rgba, dst_rgba_n_org, w*h*4);
        nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_A8(dst1_rgba, w, h, w*4, src_rgba_p, w*4, mask, w);
        nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_A8_ref(dst2_rgba, w, h, w*4, src_rgba_p, w*4, mask, w);
        TS_ASSERT( IMGCMP<R8G8B8A8N>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
    }

    void testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_A8()
    {
        memcpy(dst1_rgba, dst_rgba_p_org, w*h*4);
        memcpy(dst2_rgba, dst_rgba_p_org, w*h*4);
        nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_A8(dst1_rgba, w, h, w*4, src_rgba_n, w*4, mask, w);
        nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_A8_ref(dst2_rgba, w, h, w*4, src_rgba_n, w*4, mask, w);
        TS_ASSERT( IMGCMP<R8G8B8A8P>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
    }

    void testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_A8()
    {
        memcpy(dst1_rgba, dst_rgba_p_org, w*h*4);
        memcpy(dst2_rgba, dst_rgba_p_org, w*h*4);
        nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_A8(dst1_rgba, w, h, w*4, src_rgba_p, w*4, mask, w);
        nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_A8_ref(dst2_rgba, w, h, w*4, src_rgba_p, w*4, mask, w);
        TS_ASSERT( IMGCMP<R8G8B8A8P>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
    }

    void testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_A8()
    {
        memcpy(dst1_rgba, dst_rgba_n_org, w*h*4);
        memcpy(dst2_rgba, dst_rgba_n_org, w*h*4);
        nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_A8(dst1_rgba, w, h, w*4, src_rgba_n, w*4, mask, w);
        nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_A8_ref(dst2_rgba, w, h, w*4, src_rgba_n, w*4, mask, w);
        TS_ASSERT( IMGCMP<R8G8B8A8N>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
    }

    void testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_A8()
    {
        memcpy(dst1_rgba, dst_rgba_n_org, w*h*4);
        memcpy(dst2_rgba, dst_rgba_n_org, w*h*4);
        nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_A8(dst1_rgba, w, h, w*4, src_rgba_p, w*4, mask, w);
        nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_A8_ref(dst2_rgba, w, h, w*4, src_rgba_p, w*4, mask, w);
        TS_ASSERT( IMGCMP<R8G8B8A8N>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
    }

    void testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_A8()
    {
        memcpy(dst1_rgba, dst_rgba_p_org, w*h*4);
        memcpy(dst2_rgba, dst_rgba_p_org, w*h*4);
        nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_A8(dst1_rgba, w, h, w*4, src_rgba_n, w*4, mask, w);
        nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_A8_ref(dst2_rgba, w, h, w*4, src_rgba_n, w*4, mask, w);
        TS_ASSERT( IMGCMP<R8G8B8A8P>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
    }

    void testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_A8()
    {
        memcpy(dst1_rgba, dst_rgba_p_org, w*h*4);
        memcpy(dst2_rgba, dst_rgba_p_org, w*h*4);
        nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_A8(dst1_rgba, w, h, w*4, src_rgba_p, w*4, mask, w);
        nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_A8_ref(dst2_rgba, w, h, w*4, src_rgba_p, w*4, mask, w);
        TS_ASSERT( IMGCMP<R8G8B8A8P>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
    }

    // FINAL DST MASK COLOR

    void testnr_R8G8B8A8_N_EMPTY_A8_RGBA32()
    {
        for(size_t j=0; j<G_N_ELEMENTS(rgb_vals); j++) {
            for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
                unsigned int rgba = rgb_vals[j]+alpha_vals[i];
                char msg[100];
                sprintf(msg, "color = (%u,%u,%u,%u)", (rgba>>24u)&0xff, (rgba>>16u)&0xff, (rgba>>8u)&0xff, rgba&0xff);
                memcpy(dst1_rgba, dst_rgba_n_org, w*h*4);
                memcpy(dst2_rgba, dst_rgba_n_org, w*h*4);
                nr_R8G8B8A8_N_EMPTY_A8_RGBA32(dst1_rgba, w, h, w*4, mask, w, rgba);
                nr_R8G8B8A8_N_EMPTY_A8_RGBA32_ref(dst2_rgba, w, h, w*4, mask, w, rgba);
                TSM_ASSERT(msg, IMGCMP<R8G8B8A8N>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
            }
        }
    }

    void testnr_R8G8B8A8_P_EMPTY_A8_RGBA32()
    {
        for(size_t j=0; j<G_N_ELEMENTS(rgb_vals); j++) {
            for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
                unsigned int rgba = rgb_vals[j]+alpha_vals[i];
                char msg[100];
                sprintf(msg, "color = (%u,%u,%u,%u)", (rgba>>24u)&0xff, (rgba>>16u)&0xff, (rgba>>8u)&0xff, rgba&0xff);
                memcpy(dst1_rgba, dst_rgba_p_org, w*h*4);
                memcpy(dst2_rgba, dst_rgba_p_org, w*h*4);
                nr_R8G8B8A8_P_EMPTY_A8_RGBA32(dst1_rgba, w, h, w*4, mask, w, rgba);
                nr_R8G8B8A8_P_EMPTY_A8_RGBA32_ref(dst2_rgba, w, h, w*4, mask, w, rgba);
                TSM_ASSERT(msg, IMGCMP<R8G8B8A8P>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
            }
        }
    }

    void testnr_R8G8B8_R8G8B8_A8_RGBA32()
    {
        for(size_t j=0; j<G_N_ELEMENTS(rgb_vals); j++) {
            for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
                unsigned int rgba = rgb_vals[j]+alpha_vals[i];
                char msg[100];
                sprintf(msg, "color = (%u,%u,%u,%u)", (rgba>>24u)&0xff, (rgba>>16u)&0xff, (rgba>>8u)&0xff, rgba&0xff);
                memcpy(dst1_rgb, dst_rgb_org, w*h*3);
                memcpy(dst2_rgb, dst_rgb_org, w*h*3);
                nr_R8G8B8_R8G8B8_A8_RGBA32(dst1_rgb, w, h, w*3, mask, w, rgba);
                nr_R8G8B8_R8G8B8_A8_RGBA32_ref(dst2_rgb, w, h, w*3, mask, w, rgba);
                TSM_ASSERT(msg, IMGCMP<R8G8B8>(dst1_rgb, dst2_rgb, w*h*3) == 0 );
            }
        }
    }

    void testnr_R8G8B8A8_N_R8G8B8A8_N_A8_RGBA32()
    {
        for(size_t j=0; j<G_N_ELEMENTS(rgb_vals); j++) {
            for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
                unsigned int rgba = rgb_vals[j]+alpha_vals[i];
                char msg[100];
                sprintf(msg, "color = (%u,%u,%u,%u)", (rgba>>24u)&0xff, (rgba>>16u)&0xff, (rgba>>8u)&0xff, rgba&0xff);
                memcpy(dst1_rgba, dst_rgba_n_org, w*h*4);
                memcpy(dst2_rgba, dst_rgba_n_org, w*h*4);
                nr_R8G8B8A8_N_R8G8B8A8_N_A8_RGBA32(dst1_rgba, w, h, w*4, mask, w, rgba);
                nr_R8G8B8A8_N_R8G8B8A8_N_A8_RGBA32_ref(dst2_rgba, w, h, w*4, mask, w, rgba);
                TSM_ASSERT(msg, IMGCMP<R8G8B8A8N>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
            }
        }
    }

    void testnr_R8G8B8A8_P_R8G8B8A8_P_A8_RGBA32()
    {
        for(size_t j=0; j<G_N_ELEMENTS(rgb_vals); j++) {
            for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
                unsigned int rgba = rgb_vals[j]+alpha_vals[i];
                char msg[100];
                sprintf(msg, "color = (%u,%u,%u,%u)", (rgba>>24u)&0xff, (rgba>>16u)&0xff, (rgba>>8u)&0xff, rgba&0xff);
                memcpy(dst1_rgba, dst_rgba_p_org, w*h*4);
                memcpy(dst2_rgba, dst_rgba_p_org, w*h*4);
                nr_R8G8B8A8_P_R8G8B8A8_P_A8_RGBA32(dst1_rgba, w, h, w*4, mask, w, rgba);
                nr_R8G8B8A8_P_R8G8B8A8_P_A8_RGBA32_ref(dst2_rgba, w, h, w*4, mask, w, rgba);
                TSM_ASSERT(msg, IMGCMP<R8G8B8A8P>(dst1_rgba, dst2_rgba, w*h*4) == 0 );
            }
        }
    }

    // RGB

    void testnr_R8G8B8_R8G8B8_R8G8B8A8_N()
    {
        for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
            unsigned int alpha = alpha_vals[i];
            char msg[40];
            sprintf(msg, "alpha = %u", alpha);
            memcpy(dst1_rgb, dst_rgb_org, w*h*3);
            memcpy(dst2_rgb, dst_rgb_org, w*h*3);
            nr_R8G8B8_R8G8B8_R8G8B8A8_N(dst1_rgb, w, h, w*3, src_rgba_n, w*4, alpha);
            nr_R8G8B8_R8G8B8_R8G8B8A8_N_ref(dst2_rgb, w, h, w*3, src_rgba_n, w*4, alpha);
            TSM_ASSERT(msg, IMGCMP<R8G8B8>(dst1_rgb, dst2_rgb, w*h*3) == 0 );
        }
    }

    void testnr_R8G8B8_R8G8B8_R8G8B8A8_P()
    {
        for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
            unsigned int alpha = alpha_vals[i];
            char msg[40];
            sprintf(msg, "alpha = %u", alpha);
            memcpy(dst1_rgb, dst_rgb_org, w*h*3);
            memcpy(dst2_rgb, dst_rgb_org, w*h*3);
            nr_R8G8B8_R8G8B8_R8G8B8A8_P(dst1_rgb, w, h, w*3, src_rgba_p, w*4, alpha);
            nr_R8G8B8_R8G8B8_R8G8B8A8_P_ref(dst2_rgb, w, h, w*3, src_rgba_p, w*4, alpha);
            TSM_ASSERT(msg, IMGCMP<R8G8B8>(dst1_rgb, dst2_rgb, w*h*3) == 0 );
        }
    }

    void testnr_R8G8B8_R8G8B8_R8G8B8A8_N_A8()
    {
        for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
            unsigned int alpha = alpha_vals[i];
            char msg[40];
            sprintf(msg, "alpha = %u", alpha);
            memcpy(dst1_rgb, dst_rgb_org, w*h*3);
            memcpy(dst2_rgb, dst_rgb_org, w*h*3);
            nr_R8G8B8_R8G8B8_R8G8B8A8_N_A8(dst1_rgb, w, h, w*3, src_rgba_n, w*4, mask, w);
            nr_R8G8B8_R8G8B8_R8G8B8A8_N_A8_ref(dst2_rgb, w, h, w*3, src_rgba_n, w*4, mask, w);
            TSM_ASSERT(msg, IMGCMP<R8G8B8>(dst1_rgb, dst2_rgb, w*h*3) == 0 );
        }
    }

    void testnr_R8G8B8_R8G8B8_R8G8B8A8_P_A8()
    {
        for(size_t i=0; i<G_N_ELEMENTS(alpha_vals); i++) {
            unsigned int alpha = alpha_vals[i];
            char msg[40];
            sprintf(msg, "alpha = %u", alpha);
            memcpy(dst1_rgb, dst_rgb_org, w*h*3);
            memcpy(dst2_rgb, dst_rgb_org, w*h*3);
            nr_R8G8B8_R8G8B8_R8G8B8A8_P_A8(dst1_rgb, w, h, w*3, src_rgba_p, w*4, mask, w);
            nr_R8G8B8_R8G8B8_R8G8B8A8_P_A8_ref(dst2_rgb, w, h, w*3, src_rgba_p, w*4, mask, w);
            TSM_ASSERT(msg, IMGCMP<R8G8B8>(dst1_rgb, dst2_rgb, w*h*3) == 0 );
        }
    }
};

unsigned int const NrComposeTest::alpha_vals[7] = {0, 1, 127, 128, 129, 254, 255};
unsigned int const NrComposeTest::rgb_vals[3] = {
    (  0u<<24u)+(  1u<<16u)+( 92u<<8u),
    (127u<<24u)+(128u<<16u)+(129u<<8u),
    (163u<<24u)+(254u<<16u)+(255u<<8u)};

/*
Local Variables:
mode:c++
c-file-style:"stroustrup"
c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
indent-tabs-mode:nil
fill-column:99
End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
