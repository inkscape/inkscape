/*
 * Authors:
 *   Daniel Wagenaar <daw@caltech.edu>
 *
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "util/units.h"
#include "image-resolution.h"

#define IR_TRY_PNG 1
#include <png.h>

#ifdef HAVE_EXIF
#include <math.h>
#include <libexif/exif-data.h>
#endif

#define IR_TRY_EXIV 0

#ifdef HAVE_JPEG
#define IR_TRY_JFIF 1
#include <jpeglib.h>
#include <setjmp.h>
#endif

#ifdef WITH_IMAGE_MAGICK
#include <Magick++.h>
#endif

#define noIMAGE_RESOLUTION_DEBUG

#ifdef IMAGE_RESOLUTION_DEBUG
# define debug(f, a...) { g_print("%s(%d) %s:", \
                                  __FILE__,__LINE__,__FUNCTION__); \
                          g_print(f, ## a); \
                          g_print("\n"); \
                        }
#else
# define debug(f, a...) /* */
#endif

namespace Inkscape {
namespace Extension {
namespace Internal {

ImageResolution::ImageResolution(char const *fn) {
    ok_ = false;

    readpng(fn);
    if (!ok_) {
        readexiv(fn);
    }
    if (!ok_) {
        readjfif(fn);
    }
    if (!ok_) {
        readexif(fn);
    }
    if (!ok_) {
        readmagick(fn);
    }
}

bool ImageResolution::ok() const {
    return ok_;
}

double ImageResolution::x() const {
    return x_;
}

double ImageResolution::y() const {
    return y_;
}

  
  
#if IR_TRY_PNG
  
static bool haspngheader(FILE *fp) {
    unsigned char header[8];
    if ( fread(header, 1, 8, fp) != 8 ) {
        return false;
    }

    fseek(fp, 0, SEEK_SET);
   
    if (png_sig_cmp(header, 0, 8)) {
        return false;
    }

    return true;
}

// Implementation using libpng
void ImageResolution::readpng(char const *fn) {
    FILE *fp = fopen(fn, "rb");
    if (!fp) 
        return;

    if (!haspngheader(fp)) {
        fclose(fp);
        return;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!png_ptr) 
        return;
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, 0, 0);
        return;
    }
    
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        fclose(fp);
        return;
    }

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 res_x, res_y;
#ifdef PNG_INCH_CONVERSIONS_SUPPORTED
    debug("PNG_INCH_CONVERSIONS_SUPPORTED");
    res_x = png_get_x_pixels_per_inch(png_ptr, info_ptr);
    res_y = png_get_y_pixels_per_inch(png_ptr, info_ptr);
    if (res_x != 0 && res_y != 0) {
        ok_ = true;
        x_ = res_x * 1.0;  // FIXME: implicit conversion of png_uint_32 to double ok?
        y_ = res_y * 1.0;  // FIXME: implicit conversion of png_uint_32 to double ok?
    }
#else
    debug("PNG_RESOLUTION_METER");
    int unit_type;
    // FIXME: png_get_pHYs() fails to return expected values
    // with clang (based on LLVM 3.2svn) from Xcode 4.6.3 (OS X 10.7.5)
    png_get_pHYs(png_ptr, info_ptr, &res_x, &res_y, &unit_type);

    if (unit_type == PNG_RESOLUTION_METER) {
        ok_ = true;
        x_ = res_x * 2.54 / 100;
        y_ = res_y * 2.54 / 100;
    }
#endif

    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    fclose(fp);

    if (ok_) {
        debug("xdpi: %f", x_);
        debug("ydpi: %f", y_);
    } else {
        debug("FAILED");
    }
}
#else

// Dummy implementation
void ImageResolution::readpng(char const *) {
}

#endif

#if IR_TRY_EXIF

static double exifDouble(ExifEntry *entry, ExifByteOrder byte_order) {
    switch (entry->format) {
        case EXIF_FORMAT_BYTE: {
            return double(entry->data[0]);
        }
        case EXIF_FORMAT_SHORT: {
            return double(exif_get_short(entry->data, byte_order));
        }
        case EXIF_FORMAT_LONG: {
            return double(exif_get_long(entry->data, byte_order));
        }
        case EXIF_FORMAT_RATIONAL: {
            ExifRational r = exif_get_rational(entry->data, byte_order);
            return double(r.numerator) / double(r.denominator);
        }
        case EXIF_FORMAT_SBYTE: {
            return double(*(signed char *)entry->data);
        }
        case EXIF_FORMAT_SSHORT: {
            return double(exif_get_sshort(entry->data, byte_order));
        }
        case EXIF_FORMAT_SLONG: {
            return double(exif_get_slong(entry->data, byte_order));
        }
        case EXIF_FORMAT_SRATIONAL: {
            ExifSRational r = exif_get_srational(entry->data, byte_order);
            return double(r.numerator) / double(r.denominator);
        }
        case EXIF_FORMAT_FLOAT: {
            return double((reinterpret_cast<float *>(entry->data))[0]);
        }
        case EXIF_FORMAT_DOUBLE: {
            return (reinterpret_cast<double *>(entry->data))[0];
        }
        default: {
            return nan(0);
        }
    }
}

// Implementation using libexif
void ImageResolution::readexif(char const *fn) {
    ExifData *ed;
    ed = exif_data_new_from_file(fn);
    if (!ed)
        return;
        
    ExifByteOrder byte_order = exif_data_get_byte_order(ed);
    
    ExifEntry *xres = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_X_RESOLUTION);
    ExifEntry *yres = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_Y_RESOLUTION);
    ExifEntry *unit = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_RESOLUTION_UNIT);
    
    if ( xres && yres ) {
        x_ = exifDouble(xres, byte_order);
        y_ = exifDouble(yres, byte_order);
        if (unit) {
            double u = exifDouble(unit, byte_order);
            if ( u == 3 ) {
                x_ *= 2.54;
                y_ *= 2.54;
            }
        }
        ok_ = true;
    }
    exif_data_free(ed);
    
    if (ok_) {
        debug("xdpi: %f", x_);
        debug("ydpi: %f", y_);
    } else {
        debug("FAILED");
    }
}

#else

// Dummy implementation
void ImageResolution::readexif(char const *) {
}
 
#endif
  
#if IR_TRY_EXIV

void ImageResolution::readexiv(char const *fn) {
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(fn);
    if (!image.get())
        return;

    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty())
        return;

    Exiv2::ExifData::const_iterator end = exifData.end();
    bool havex = false;
    bool havey = false;
    bool haveunit = false;
    int unit;
    for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
        if (ok_)
            break;
        if ( i->tag() == 0x011a ) {
            // X Resolution
            x_ = i->toFloat();
            havex = true;
        } else if ( i->tag() == 0x011b ) {
            // Y Resolution
            y_ = i->toFloat();
            havey = true;
        } else if ( i->tag() == 0x0128 ) {
            unit = i->toLong();
        }
        ok_ = havex && havey && haveunit;
    }
    if (haveunit) {
        if ( unit == 3 ) {
            x_ *= 2.54;
            y_ *= 2.54;
        }
    }
    ok_ = havex && havey;
    
    if (ok_) {
        debug("xdpi: %f", x_);
        debug("ydpi: %f", y_);
    } else {
        debug("FAILED");
    }
}

#else

// Dummy implementation
void ImageResolution::readexiv(char const *) {
}

#endif

#if IR_TRY_JFIF

static void irjfif_error_exit(j_common_ptr cinfo) {
    longjmp(*(jmp_buf*)cinfo->client_data, 1);
}

static void irjfif_emit_message(j_common_ptr, int) {
}

static void irjfif_output_message(j_common_ptr) {
}

static void irjfif_format_message(j_common_ptr, char *) {
}

static void irjfif_reset(j_common_ptr) {
}

void ImageResolution::readjfif(char const *fn) {
    FILE *ifd = fopen(fn, "rb");
    if (!ifd) {
        return;
    }

    struct jpeg_decompress_struct cinfo;
    jmp_buf jbuf;
    struct jpeg_error_mgr jerr;

    if (setjmp(jbuf)) {
        fclose(ifd);
        jpeg_destroy_decompress(&cinfo);
        return;
    }
    
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jerr.error_exit = &irjfif_error_exit;
    jerr.emit_message = &irjfif_emit_message;
    jerr.output_message = &irjfif_output_message;
    jerr.format_message = &irjfif_format_message;
    jerr.reset_error_mgr = &irjfif_reset;
    cinfo.client_data = (void*)&jbuf;

    jpeg_stdio_src(&cinfo, ifd);
    jpeg_read_header(&cinfo, TRUE);
    
    debug("cinfo.[XY]_density");
    if (cinfo.saw_JFIF_marker) { // JFIF APP0 marker was seen
        if ( cinfo.density_unit == 1 ) { // dots/inch
            x_ = cinfo.X_density;
            y_ = cinfo.Y_density;
            ok_ = true;
        } else if ( cinfo.density_unit == 2 ) { // dots/cm
            x_ = cinfo.X_density * 2.54;
            y_ = cinfo.Y_density * 2.54;
            ok_ = true;
        }
        /* According to http://www.jpeg.org/public/jfif.pdf (page 7):
         * "Xdensity and Ydensity should always be non-zero".
         * but in some cases, they are (see LP bug #1275443) */
        if (x_ == 0 or y_ == 0) {
            ok_ = false;
        }
    }
    jpeg_destroy_decompress(&cinfo);
    fclose(ifd);
    
    if (ok_) {
        debug("xdpi: %f", x_);
        debug("ydpi: %f", y_);
    } else {
        debug("FAILED");
    }
}

#else

// Dummy implementation
void ImageResolution::readjfif(char const *) {
}

#endif

#ifdef WITH_IMAGE_MAGICK
void ImageResolution::readmagick(char const *fn) {
    Magick::Image image;
    debug("Trying image.read");
    try {
        image.read(fn);
    } catch (Magick::Error & err) {
        debug("ImageMagick error: %s", err.what());
        return;
    } catch (std::exception & err) {
        debug("ImageResolution::readmagick: %s", err.what());
        return;
    }
    debug("image.[xy]Resolution");
    std::string const type = image.magick();
    x_ = image.xResolution();
    y_ = image.yResolution();

// TODO: find out why the hell the following convertion is necessary
    if (type == "BMP") {
        x_ = Inkscape::Util::Quantity::convert(x_, "in", "cm");
        y_ = Inkscape::Util::Quantity::convert(y_, "in", "cm");
    }
    
    if (x_ != 0 && y_ != 0) {
        ok_ = true;
    }

    if (ok_) {
        debug("xdpi: %f", x_);
        debug("ydpi: %f", y_);
    } else {
        debug("FAILED");
        debug("Using default Inkscape import resolution");
    }
}

#else

// Dummy implementation
void ImageResolution::readmagick(char const *) {
}

#endif /* WITH_IMAGE_MAGICK */

}
}
}
