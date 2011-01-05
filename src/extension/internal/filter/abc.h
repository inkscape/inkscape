#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_ABC_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_ABC_H__
/* Change the 'ABC' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Basic filters
 *   Blur
 *   Diffuse light
 *   Roughen
 *   Specular light
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
/* ^^^ Change the copyright to be you and your e-mail address ^^^ */

#include "filter.h"

#include "extension/internal/clear-n_.h"
#include "extension/system.h"
#include "extension/extension.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Filter {

/**
    \brief    Custom predefined Blur filter.
    
    Simple horizontal and vertical blur.

    Filter's parameters:
    * Horizontal blur (0.01->100., default 2) -> blur (stdDeviation)
    * Vertical blur (0.01->100., default 2) -> blur (stdDeviation)
*/

class Blur : public Inkscape::Extension::Internal::Filter::Filter {
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	Blur ( ) : Filter() { };
	virtual ~Blur ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

	static void init (void) {
		Inkscape::Extension::build_from_mem(
			"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
				"<name>" N_("Blur, custom (ABCs)") "</name>\n"
				"<id>org.inkscape.effect.filter.blur</id>\n"
                "<param name=\"hblur\" gui-text=\"" N_("Horizontal blur:") "\" type=\"float\" min=\"0.01\" max=\"100\">2</param>\n"
                "<param name=\"vblur\" gui-text=\"" N_("Vertical blur:") "\" type=\"float\" min=\"0.01\" max=\"100\">2</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"" N_("Experimental") "\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Simple vertical and horizontal blur effect") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new Blur());
	};

};

gchar const *
Blur::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream hblur;
    std::ostringstream vblur;

    hblur << ext->get_param_float("hblur");
    vblur << ext->get_param_float("vblur");

	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Blur, custom\">\n"
        "<feGaussianBlur stdDeviation=\"%s %s\" result=\"blur\" />\n"
        "</filter>\n", hblur.str().c_str(), vblur.str().c_str());

	return _filter;
}; /* Blur filter */

/**
    \brief    Custom predefined Diffuse light filter.
    
    Basic diffuse bevel to use for building textures

    Filter's parameters:
    * Smoothness (0.->10., default 6.) -> blur (stdDeviation)
    * Elevation (0->360, default 25) -> feDistantLight (elevation)
    * Azimuth (0->360, default 235) -> feDistantLight (azimuth)
    * Lightning color (guint, default -1 [white]) -> diffuse (lighting-color)

    TODO: use the alpha channel to calculate the lightning color
*/

class DiffuseLight : public Inkscape::Extension::Internal::Filter::Filter {
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	DiffuseLight ( ) : Filter() { };
	virtual ~DiffuseLight ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

	static void init (void) {
		Inkscape::Extension::build_from_mem(
			"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
				"<name>" N_("Diffuse light, custom (ABCs)") "</name>\n"
				"<id>org.inkscape.effect.filter.diffuselight</id>\n"
                "<param name=\"smooth\" gui-text=\"" N_("Smoothness:") "\" type=\"float\" min=\"0.0\" max=\"10\">6</param>\n"
                "<param name=\"elevation\" gui-text=\"" N_("Elevation:") "\" type=\"int\" min=\"0\" max=\"360\">25</param>\n"
                "<param name=\"azimuth\" gui-text=\"" N_("Azimuth:") "\" type=\"int\" min=\"0\" max=\"360\">235</param>\n"
                "<param name=\"color\" gui-text=\"" N_("Lightning color") "\" type=\"color\">-1</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"" N_("Experimental") "\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Basic diffuse bevel to use for building textures") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new DiffuseLight());
	};

};

gchar const *
DiffuseLight::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream smooth;
    std::ostringstream elevation;
    std::ostringstream azimuth;
    std::ostringstream r;
    std::ostringstream g;
    std::ostringstream b;
    std::ostringstream a;

    smooth << ext->get_param_float("smooth");
    elevation << ext->get_param_int("elevation");
    azimuth << ext->get_param_int("azimuth");
    guint32 color = ext->get_param_color("color");

    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);
    a << (color & 0xff) / 255.0F;
    
	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Diffuse light, custom\">\n"
        "<feGaussianBlur in=\"SourceGraphic\" stdDeviation=\"%s\" result=\"blur\" />\n"
        "<feDiffuseLighting diffuseConstant=\"1\" surfaceScale=\"10\" lighting-color=\"rgb(%s,%s,%s)\" result=\"diffuse\">\n"
            "<feDistantLight elevation=\"%s\" azimuth=\"%s\" />\n"
        "</feDiffuseLighting>\n"
        "<feComposite in=\"diffuse\" in2=\"diffuse\" operator=\"arithmetic\" k1=\"1\" result=\"composite1\" />\n"
        "<feComposite in=\"composite1\" in2=\"SourceGraphic\" k1=\"%s\" operator=\"arithmetic\" k3=\"1\" result=\"composite2\" />\n"
        "</filter>\n", smooth.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str(), elevation.str().c_str(), azimuth.str().c_str(), a.str().c_str());

	return _filter;
}; /* DiffuseLight filter */

/**
    \brief    Custom predefined Roughen filter.
    
    Small-scale roughening to edges and content

    Filter's parameters:
    * Frequency (*100) (0.0->40., default 1.3) -> turbulence (baseFrequency)
    * Intensity (0.0->50., default 6.6) -> displacement (scale)
*/

class Roughen : public Inkscape::Extension::Internal::Filter::Filter {
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	Roughen ( ) : Filter() { };
	virtual ~Roughen ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

	static void init (void) {
		Inkscape::Extension::build_from_mem(
			"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
				"<name>" N_("Roughen, custom (ABCs)") "</name>\n"
				"<id>org.inkscape.effect.filter.roughen</id>\n"
                "<param name=\"hfreq\" gui-text=\"" N_("Horizontal frequency (x100):") "\" type=\"float\" min=\"0.0\" max=\"40\">1.3</param>\n"
                "<param name=\"vfreq\" gui-text=\"" N_("Vertical frequency (x100):") "\" type=\"float\" min=\"0.0\" max=\"40\">1.3</param>\n"
                "<param name=\"intensity\" gui-text=\"" N_("Intensity:") "\" type=\"float\" min=\"0.0\" max=\"50\">6.6</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"" N_("Experimental") "\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Small-scale roughening to edges and content") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new Roughen());
	};

};

gchar const *
Roughen::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream hfreq;
    std::ostringstream vfreq;
    std::ostringstream intensity;

    hfreq << (ext->get_param_float("hfreq") / 100);
    vfreq << (ext->get_param_float("vfreq") / 100);
    intensity << ext->get_param_float("intensity");

	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Roughen, custom\">\n"
        "<feTurbulence numOctaves=\"3\" seed=\"0\" type=\"turbulence\" baseFrequency=\"%s %s\" result=\"turbulence\" />\n"
        "<feDisplacementMap in=\"SourceGraphic\" in2=\"turbulence\" scale=\"%s\" yChannelSelector=\"G\" xChannelSelector=\"R\" />\n"
        "</filter>\n", hfreq.str().c_str(), vfreq.str().c_str(), intensity.str().c_str());

	return _filter;
}; /* Roughen filter */

/**
    \brief    Custom predefined Specular light filter.
    
    Basic specular bevel to use for building textures

    Filter's parameters:
    * Smoothness (0.0->10., default 6.) -> blur (stdDeviation)
    * Brightness (0.0->5., default 1.) -> specular (specularConstant)
    * Elevation (0->360, default 25) -> feDistantLight (elevation)
    * Azimuth (0->360, default 235) -> feDistantLight (azimuth)
    * Lightning color (guint, default -1 [white]) -> specular (lighting-color)

    TODO: use the alpha channel to calculate the lightning color (but do we really need a lightning color here?)
*/

class SpecularLight : public Inkscape::Extension::Internal::Filter::Filter {
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	SpecularLight ( ) : Filter() { };
	virtual ~SpecularLight ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

	static void init (void) {
		Inkscape::Extension::build_from_mem(
			"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
				"<name>" N_("Specular light, custom (ABCs)") "</name>\n"
				"<id>org.inkscape.effect.filter.specularlight</id>\n"
                "<param name=\"smooth\" gui-text=\"" N_("Smoothness:") "\" type=\"float\" min=\"0.0\" max=\"10\">6</param>\n"
                "<param name=\"bright\" gui-text=\"" N_("Brightness:") "\" type=\"float\" min=\"0.0\" max=\"5\">1</param>\n"
                "<param name=\"elevation\" gui-text=\"" N_("Elevation:") "\" type=\"int\" min=\"0\" max=\"360\">45</param>\n"
                "<param name=\"azimuth\" gui-text=\"" N_("Azimuth:") "\" type=\"int\" min=\"0\" max=\"360\">235</param>\n"
                "<param name=\"color\" gui-text=\"" N_("Lightning color") "\" type=\"color\">-1</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"" N_("Experimental") "\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Basic specular bevel to use for building textures") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new SpecularLight());
	};

};

gchar const *
SpecularLight::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream smooth;
    std::ostringstream bright;
    std::ostringstream elevation;
    std::ostringstream azimuth;
    std::ostringstream r;
    std::ostringstream g;
    std::ostringstream b;
    std::ostringstream a;

    smooth << ext->get_param_float("smooth");
    bright << ext->get_param_float("bright");
    elevation << ext->get_param_int("elevation");
    azimuth << ext->get_param_int("azimuth");
    guint32 color = ext->get_param_color("color");

    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);
    a << (color & 0xff) / 255.0F;
    
	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Specular light, custom\">\n"
        "<feGaussianBlur in=\"SourceAlpha\" stdDeviation=\"%s\" result=\"blur\" />\n"
        "<feSpecularLighting in=\"blur\" specularExponent=\"25\" specularConstant=\"%s\" surfaceScale=\"10\" lighting-color=\"rgb(%s,%s,%s)\" result=\"specular\">\n"
           "<feDistantLight elevation=\"%s\" azimuth=\"%s\" />\n"
        "</feSpecularLighting>\n"
        "<feComposite in=\"specular\" in2=\"SourceGraphic\" k3=\"1\" k2=\"%s\" operator=\"arithmetic\" result=\"composite1\" />\n"
        "<feComposite in=\"composite1\" in2=\"SourceAlpha\" operator=\"in\" result=\"composite2\" />\n"
        "</filter>\n", smooth.str().c_str(), bright.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str(), elevation.str().c_str(), azimuth.str().c_str(), a.str().c_str());

	return _filter;
}; /* SpecularLight filter */


}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'COLOR' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_ABC_H__ */
