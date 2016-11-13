# This file contains a list of
#    - available installer translations (*.nsh in this directory)
#    - available Inkscape translations (*.po in /src/po)
#
# The list is formatted for use in the NSIS installer script
# and should be updated whenever new translations are added.


### List of available installer translations.
# Every entry should
#    - have a name (e.g. "Breton") matching a header file in this directory (e.g. "Breton.nsh")
#      that in addition should match the name of an available NSIS translation file (see "NSIS\Contrib\Language files")
#    - include a valid locale ID (see https://msdn.microsoft.com/goglobal/bb964664.aspx)
!macro INSTALLER_TRANSLATIONS _MACRONAME
    !insertmacro ${_MACRONAME} Breton       1150
    !insertmacro ${_MACRONAME} Catalan      1027
    !insertmacro ${_MACRONAME} Czech        1029
    !insertmacro ${_MACRONAME} Danish       1030
    !insertmacro ${_MACRONAME} Dutch        1043
    !insertmacro ${_MACRONAME} Finnish      1035
    !insertmacro ${_MACRONAME} French       1036
    !insertmacro ${_MACRONAME} Galician     1110
    !insertmacro ${_MACRONAME} German       1031
    !insertmacro ${_MACRONAME} Greek        1032
    !insertmacro ${_MACRONAME} Hebrew       1037
    !insertmacro ${_MACRONAME} Icelandic    1039
    !insertmacro ${_MACRONAME} Indonesian   1057
    !insertmacro ${_MACRONAME} Italian      1040
    !insertmacro ${_MACRONAME} Japanese     1041
    !insertmacro ${_MACRONAME} Polish       1045
    !insertmacro ${_MACRONAME} Portuguese   2070
    !insertmacro ${_MACRONAME} PortugueseBR 1046
    !insertmacro ${_MACRONAME} Romanian     1048
    !insertmacro ${_MACRONAME} Russian      1049
    !insertmacro ${_MACRONAME} Slovak       1051
    !insertmacro ${_MACRONAME} Slovenian    1060
    !insertmacro ${_MACRONAME} Spanish      1034
    !insertmacro ${_MACRONAME} SimpChinese  2052
    !insertmacro ${_MACRONAME} TradChinese  1028
    !insertmacro ${_MACRONAME} Ukrainian    1058
!macroend


### List of available Inkscape translations.
# Every entry should
#    - specify a name (e.g. "Amharic") that matches the installer translation name above (if present)
#    - specify a language code (e.g. "am") that has a matching PO file (e.g. "am.po")
#      in the /po folder in the root of Inkscape source directory
!macro INKSCAPE_TRANSLATIONS _MACRONAME
    !insertmacro ${_MACRONAME} Amharic             am
    !insertmacro ${_MACRONAME} Arabic              ar
    !insertmacro ${_MACRONAME} Assamese            as
    !insertmacro ${_MACRONAME} Azerbaijani         az
    !insertmacro ${_MACRONAME} Belarusian          be
    !insertmacro ${_MACRONAME} Bulgarian           bg
    !insertmacro ${_MACRONAME} Bengali             bn
    !insertmacro ${_MACRONAME} BengaliBangladesh   bn_BD
    !insertmacro ${_MACRONAME} Breton              br
    !insertmacro ${_MACRONAME} Bodo                brx
    !insertmacro ${_MACRONAME} Catalan             ca
    !insertmacro ${_MACRONAME} CatalanValencia     ca@valencia
    !insertmacro ${_MACRONAME} Czech               cs
    !insertmacro ${_MACRONAME} Danish              da
    !insertmacro ${_MACRONAME} German              de
    !insertmacro ${_MACRONAME} Dogri               doi
    !insertmacro ${_MACRONAME} Dzongkha            dz
    !insertmacro ${_MACRONAME} Greek               el
    !insertmacro ${_MACRONAME} EnglishAustralian   en_AU
    !insertmacro ${_MACRONAME} EnglishCanadian     en_CA
    !insertmacro ${_MACRONAME} EnglishBritain      en_GB
    !insertmacro ${_MACRONAME} EnglishPiglatin     en_US@piglatin
    !insertmacro ${_MACRONAME} Esperanto           eo
    !insertmacro ${_MACRONAME} Spanish             es
    !insertmacro ${_MACRONAME} SpanishMexico       es_MX
    !insertmacro ${_MACRONAME} Estonian            et
    !insertmacro ${_MACRONAME} Basque              eu
    !insertmacro ${_MACRONAME} Farsi               fa
    !insertmacro ${_MACRONAME} Finnish             fi
    !insertmacro ${_MACRONAME} French              fr
    !insertmacro ${_MACRONAME} Irish               ga
    !insertmacro ${_MACRONAME} Galician            gl
    !insertmacro ${_MACRONAME} Gujarati            gu
    !insertmacro ${_MACRONAME} Hebrew              he
    !insertmacro ${_MACRONAME} Hindi               hi
    !insertmacro ${_MACRONAME} Croatian            hr
    !insertmacro ${_MACRONAME} Hungarian           hu
    !insertmacro ${_MACRONAME} Armenian            hy
    !insertmacro ${_MACRONAME} Indonesian          id
    !insertmacro ${_MACRONAME} Icelandic           is
    !insertmacro ${_MACRONAME} Italian             it
    !insertmacro ${_MACRONAME} Japanese            ja
    !insertmacro ${_MACRONAME} Khmer               km
    !insertmacro ${_MACRONAME} Kannada             kn
    !insertmacro ${_MACRONAME} Korean              ko
    !insertmacro ${_MACRONAME} Konkani             kok
    !insertmacro ${_MACRONAME} KonkaniLatin        kok@latin
    !insertmacro ${_MACRONAME} KashmiriPersoArabic ks@aran
    !insertmacro ${_MACRONAME} KashmiriDevanagari  ks@deva
    !insertmacro ${_MACRONAME} Lithuanian          lt
    !insertmacro ${_MACRONAME} Latvian             lv
    !insertmacro ${_MACRONAME} Maithili            mai
    !insertmacro ${_MACRONAME} Macedonian          mk
    !insertmacro ${_MACRONAME} Malayalam           ml
    !insertmacro ${_MACRONAME} Mongolian           mn
    !insertmacro ${_MACRONAME} Manipuri            mni
    !insertmacro ${_MACRONAME} ManipuriBengali     mni@beng
    !insertmacro ${_MACRONAME} Marathi             mr
    !insertmacro ${_MACRONAME} NorwegianBokmal     nb
    !insertmacro ${_MACRONAME} Nepali              ne
    !insertmacro ${_MACRONAME} Dutch               nl
    !insertmacro ${_MACRONAME} NorwegianNynorsk    nn
    !insertmacro ${_MACRONAME} Odia                or
    !insertmacro ${_MACRONAME} Panjabi             pa
    !insertmacro ${_MACRONAME} Polish              pl
    !insertmacro ${_MACRONAME} Portuguese          pt
    !insertmacro ${_MACRONAME} PortugueseBR        pt_BR
    !insertmacro ${_MACRONAME} Romanian            ro
    !insertmacro ${_MACRONAME} Russian             ru
    !insertmacro ${_MACRONAME} Kinyarwanda         rw
    !insertmacro ${_MACRONAME} Sanskrit            sa
    !insertmacro ${_MACRONAME} Santali             sat
    !insertmacro ${_MACRONAME} SantaliDevanagari   sat@deva
    !insertmacro ${_MACRONAME} Sindhi              sd
    !insertmacro ${_MACRONAME} SindhiDevanagari    sd@deva
    !insertmacro ${_MACRONAME} Slovak              sk
    !insertmacro ${_MACRONAME} Slovenian           sl
    !insertmacro ${_MACRONAME} Albanian            sq
    !insertmacro ${_MACRONAME} Serbian             sr
    !insertmacro ${_MACRONAME} SerbianLatin        sr@latin
    !insertmacro ${_MACRONAME} Swedish             sv
    !insertmacro ${_MACRONAME} Tamil               ta
    !insertmacro ${_MACRONAME} Telugu              te
    !insertmacro ${_MACRONAME} Thai                th
    !insertmacro ${_MACRONAME} Turkish             tr
    !insertmacro ${_MACRONAME} Ukrainian           uk
    !insertmacro ${_MACRONAME} Urdu                ur
    !insertmacro ${_MACRONAME} Vietnamese          vi
    !insertmacro ${_MACRONAME} SimpChinese         zh_CN
    !insertmacro ${_MACRONAME} TradChinese         zh_TW
!macroend
