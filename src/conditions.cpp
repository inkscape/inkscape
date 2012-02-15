/*
 * SVG conditional attribute evaluation
 *
 * Authors:
 *   Andrius R. <knutux@gmail.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <set>
#include <glibmm/ustring.h>
#include "conditions.h"
#include "xml/repr.h"
#include "rdf.h"

typedef bool (* condition_evaluator)(SPItem const *item, gchar const *value);

struct Condition {
    gchar const *attribute;
    condition_evaluator evaluator;
};

static bool evaluateSystemLanguage(SPItem const *item, gchar const *value);
static bool evaluateRequiredFeatures(SPItem const *item, gchar const *value);
static bool evaluateRequiredExtensions(SPItem const *item, gchar const *value);

/* define any conditional attributes and their handler functions in this array */
static Condition _condition_handlers[] = {
    { "systemLanguage", evaluateSystemLanguage },
    { "requiredFeatures", evaluateRequiredFeatures },
    { "requiredExtensions", evaluateRequiredExtensions },
};

// function which evaluates if item should be displayed
bool sp_item_evaluate(SPItem const *item) {
    bool needDisplay = true;
    for ( unsigned int i = 0 ; needDisplay && (i < sizeof(_condition_handlers) / sizeof(_condition_handlers[0])) ; i++ ) {
        gchar const *value = item->getAttribute(_condition_handlers[i].attribute);
        if ( value && !_condition_handlers[i].evaluator(item, value) ) {
            needDisplay = false;
        }
    }
    return needDisplay;
}

#define ISALNUM(c)    (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || ((c) >= '0' && (c) <= '9'))

static gchar *preprocessLanguageCode(gchar *lngcode) {
    if ( NULL == lngcode )
        return NULL;

    lngcode = g_strstrip(lngcode);
    if ( 0 == *lngcode )
        return lngcode;
    for ( unsigned int i = 0 ; i < strlen(lngcode) ; i++ ) {
        if ( lngcode[i] >= 'A' && lngcode[i] <= 'Z' ) {
            lngcode[i] = g_ascii_tolower(lngcode[i]);
        } else if ( '_' == lngcode[i] ) {
            lngcode[i] = '-';
        } else if ( !ISALNUM(lngcode[i]) && '-' != lngcode[i] ) {
            // only alpha numeric characters and '-' may be contained in language code
            lngcode[0] = 0;
            break;
        }
    }
    
    return lngcode;
}

static bool evaluateSystemLanguage(SPItem const *item, gchar const *value) {
    if ( NULL == value )
        return true;

    std::set<Glib::ustring> language_codes;
    gchar *str = NULL;
    gchar **strlist = g_strsplit( value, ",", 0);

    for ( int i = 0 ; (str = strlist[i]) ; i++ ) {
        gchar *lngcode = preprocessLanguageCode(str);
        if ( 0 == *lngcode )
            continue;
        language_codes.insert(lngcode);

        gchar *pos = strchr (lngcode, '-');
        if (pos)
        {
            // if subtag is used, primary tag is still a perfect match
            *pos = 0;
            if ( language_codes.find(lngcode) == language_codes.end() ) {
                language_codes.insert(lngcode);
            }
        }
    }
    g_strfreev(strlist);

    if (language_codes.empty())
        return false;

    SPDocument *document = item->document;
    Glib::ustring document_language = document->getLanguage();

    if (document_language.size() == 0)
        return false;

    bool match = true;
    strlist = g_strsplit( document_language.c_str(), ",", 0);
    for ( int i = 0 ; (str = strlist[i]) ; i++ ) {
        gchar *lngcode = preprocessLanguageCode(str);
        if ( 0 == *lngcode )
            continue;
        if ( language_codes.find(lngcode) != language_codes.end() ) {
            match = true;
            break;
        }
        match = false;
    }
    g_strfreev(strlist);
    return match;
}

static std::vector<Glib::ustring> splitByWhitespace(gchar const *value) {
    std::vector<Glib::ustring> parts;
    gchar *str = NULL;
    gchar **strlist = g_strsplit( value, ",", 0);

    for ( int i = 0 ; (str = strlist[i]) ; i++ ) {
        gchar *part = g_strstrip(str);
        if ( 0 == *part )
            continue;
        parts.push_back(part);
    }
    g_strfreev(strlist);
    return parts;
}

#define SVG11FEATURE    "http://www.w3.org/TR/SVG11/feature#"
#define SVG10FEATURE    "org.w3c."

static bool evaluateSVG11Feature(gchar const *feature) {
    static gchar const *_supported_features[] = {
        "SVG", // incomplete "SVG-static" - missing support for "Filter"
           /* SVG - user agent supports at least one of the following:
                "SVG-static", "SVG-animation", "SVG-dynamic" or "SVGDOM" */
        // "SVGDOM", // not sure
           /* SVGDOM - user agent supports at least one of the following:
                 "SVGDOM-static", "SVGDOM-animation" or "SVGDOM-dynamic" */
        "SVG-static", // incomplete - missing support for "Filter"
           /* SVG-static - user agent supports the following features:
                "CoreAttribute", "Structure", "ContainerAttribute",
                "ConditionalProcessing", "Image", "Style", "ViewportAttribute",
                "Shape", "Text", "PaintAttribute", "OpacityAttribute",
                "GraphicsAttribute", "Marker", "ColorProfile",
                "Gradient", "Pattern", "Clip", "Mask", "Filter",
                "XlinkAttribute", "Font", "Extensibility" */
        // "SVGDOM-static", // not sure
           /* SVGDOM-static - All of the DOM interfaces and methods
                that correspond to SVG-static */
        // "SVG-animation", // no support
           /* SVG-animation - All of the language features from "SVG-static"
                plus the feature "feature#Animation" */
        // "SVGDOM-animation", // no support
           /* SVGDOM-animation - All of the DOM interfaces and methods
                that correspond to SVG-animation */
        // "SVG-dynamic", // no support
           /* SVG-dynamic - user agent supports all "SVG-animation" and the following features:
                "Hyperlinking", "Scripting", "View", "Cursor",
                "GraphicalEventsAttribute", "DocumentEventsAttribute", "AnimationEventsAttribute" */
        // "SVGDOM-dynamic", // no support
           /* SVGDOM-dynamic - All of the DOM interfaces and methods
                that correspond to SVG-dynamic */
        "CoreAttribute",
        "Structure",
        "BasicStructure",
        "ContainerAttribute",
        "ConditionalProcessing",
        "Image",
        "Style",
        "ViewportAttribute", // not sure
        "Shape",
        "Text",
        "BasicText",
        "PaintAttribute",
        "BasicPaintAttribute",
        "OpacityAttribute",
        "GraphicsAttribute",
        "BasicGraphicsAttribute",
        "Marker",
        "ColorProfile",
        "Gradient",
        "Pattern",
        "Clip",
        "BasicClip",
        "Mask",
        // "Filter",
        // "BasicFilter",
        // "DocumentEventsAttribute",
        // "GraphicalEventsAttribute",
        // "AnimationEventsAttribute",
        // "Cursor", // not sure
        "Hyperlinking", // not sure
        "XlinkAttribute", // not sure
        "ExternalResourcesRequired", // not sure
        "View",
        // "Script",
        // "Animation",
        "Font",
        "BasicFont",
        "Extensibility", // not sure
    };
    
    for ( unsigned int i = 0 ; i < sizeof(_supported_features)/sizeof(_supported_features[0]); i++ ) {
        if ( 0 == strcasecmp(feature, _supported_features[0]) )
            return true;
    }
    return false;
}

static bool evaluateSVG10Feature(gchar const *feature) {
    static gchar const *_supported_features[] = {
        "svg.static", // incomplete - no filter effects
        "dom.svg.static", // not sure
        // "svg.animation",
        // "dom.svg.animation",
        // "svg.dynamic",
        // "dom.svg.dynamic"
        // "svg.all",
        // "dom.svg.all"
    };
    for ( unsigned int i = 0 ; i < sizeof(_supported_features)/sizeof(_supported_features[0]); i++ ) {
        if ( 0 == strcasecmp(feature, _supported_features[0]) )
            return true;
    }
    return false;
}

static bool evaluateSingleFeature(gchar const *value) {
    if ( NULL == value )
        return false;
    gchar const *found;
    found = strstr(value, SVG11FEATURE);
    if ( value == found )
        return evaluateSVG11Feature(found + strlen(SVG11FEATURE));
    found = strstr(value, SVG10FEATURE);
    if ( value == found )
        return evaluateSVG10Feature(found + strlen(SVG10FEATURE));
    return false;
}

static bool evaluateRequiredFeatures(SPItem const */*item*/, gchar const *value) {
    if ( NULL == value )
        return true;

    std::vector<Glib::ustring> parts = splitByWhitespace(value);
    if (parts.empty())
    {
        return false;
    }
    
    for ( unsigned int i = 0 ; i < parts.size() ; i++ ) {
        if (!evaluateSingleFeature(parts[i].c_str())) {
            return false;
        }
    }
    
    return true;
}

static bool evaluateRequiredExtensions(SPItem const */*item*/, gchar const *value) {
    if ( NULL == value )
        return true;
    return false;
}

/*
 * Language codes and names:
aa Afar
ab Abkhazian
af Afrikaans
am Amharic
ar Arabic
as Assamese
ay Aymara
az Azerbaijani

ba Bashkir
be Byelorussian
bg Bulgarian
bh Bihari
bi Bislama
bn Bengali; Bangla
bo Tibetan
br Breton

ca Catalan
co Corsican
cs Czech
cy Welsh

da Danish
de German
dz Bhutani

el Greek
en English
eo Esperanto
es Spanish
et Estonian
eu Basque

fa Persian
fi Finnish
fj Fiji
fo Faroese
fr French
fy Frisian

ga Irish
gd Scots Gaelic
gl Galician
gn Guarani
gu Gujarati

ha Hausa
he Hebrew (formerly iw)
hi Hindi
hr Croatian
hu Hungarian
hy Armenian

ia Interlingua
id Indonesian (formerly in)
ie Interlingue
ik Inupiak
is Icelandic
it Italian
iu Inuktitut

ja Japanese
jw Javanese

ka Georgian
kk Kazakh
kl Greenlandic
km Cambodian
kn Kannada
ko Korean
ks Kashmiri
ku Kurdish
ky Kirghiz

la Latin
ln Lingala
lo Laothian
lt Lithuanian
lv Latvian, Lettish

mg Malagasy
mi Maori
mk Macedonian
ml Malayalam
mn Mongolian
mo Moldavian
mr Marathi
ms Malay
mt Maltese
my Burmese

na Nauru
ne Nepali
nl Dutch
no Norwegian

oc Occitan
om (Afan) Oromo
or Oriya

pa Punjabi
pl Polish
ps Pashto, Pushto
pt Portuguese

qu Quechua

rm Rhaeto-Romance
rn Kirundi
ro Romanian
ru Russian
rw Kinyarwanda

sa Sanskrit
sd Sindhi
sg Sangho
sh Serbo-Croatian
si Sinhalese
sk Slovak
sl Slovenian
sm Samoan
sn Shona
so Somali
sq Albanian
sr Serbian
ss Siswati
st Sesotho
su Sundanese
sv Swedish
sw Swahili

ta Tamil
te Telugu
tg Tajik
th Thai
ti Tigrinya
tk Turkmen
tl Tagalog
tn Setswana
to Tonga
tr Turkish
ts Tsonga
tt Tatar
tw Twi

ug Uighur
uk Ukrainian
ur Urdu
uz Uzbek

vi Vietnamese
vo Volapuk

wo Wolof

xh Xhosa

yi Yiddish (formerly ji)
yo Yoruba

za Zhuang
zh Chinese
zu Zulu
 */


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
