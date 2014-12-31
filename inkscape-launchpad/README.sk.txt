
Inkscape. Kreslite slobodne.
============================

http://www.inkscape.org/

Inkscape je open source kresliaci nástroj podobný chopnosťami programom
ako Illustrator, Freehand a CorelDraw, ktorý používa formát SVG
(Scalable Vector Graphics), štandard W3C. Medzi niektoré podporované
možnosti SVG patria základné tvary, cesty, text, koncové značky, klony,
alfa priesvitnosť, transformácie, farebné prechody a zoskupovanie.
Naviac Inkscape podporuje  metadáta Creative Commons, úpravu uzlov,
vrstvy, komplexné operácie s cestami, text na ceste a úpravu XML
SVG súboru. Tiež dokáže importovať niekoľko formátov ako EPS, PostScript,
JPEG, PNG, BMP a TIFF a exportovať PNG a niekoľko vektorových formátov.

Hlavnou motiváciou Inkscape je poskytnúť open source komunite nástroj
na kreslenie plne konformný so štandardmi W3C XML, SVG a CSS2. Ďalšou
fázou vývoja je konverzia kódu z C/Gtk na C++/Gtkmm, zdôraznenie
odľahčeného jadra, ku ktorému sa mocné vlastnosti pridávajú
prostredníctvom mechanizmu rozšírení a vytvorenie priateľského,
otvoreného vývojového procesu orientovaného na komunitu.

Inkscape používa na kompiláciu inštaláciu štandardný postup:

  ./configure
  make
  make install

Ak nie je prítomný súbor „./configure“, môžete ho vytvoriť spustením
príkazu „./autogen.sh“, ktorý volá niekoľko ďalších programov ako
automake a autoconf. Podrobnosti nájdete v súbore INSTALL.


Požadované závislosti
=====================
Jadro Inkscape závisí na niekoľkých ďalších knižniciach, ktoré si
budete musieť nainštalovať ak ich ešte nemáte v systéme. Zvyčajne
budete potrebovať doinštalovať nasledovné knižnice:

   * Boehm-GC 
   * libsigc++
   * glibmm  
   * gtkmm  

Aktuálne závislosti vrátane odkazov na zdrojové tarbally nájdete na
http://wiki.inkscape.org/wiki/index.php/CompilingInkscape


Závislosti rozšírení
====================
Inkscape má tiež množstvo rozšírení implementujúcich rozličné
vlastnosti ako podpora ďalších formátov okrem SVG atď. Teoreticky sú
všetky rozšírenia nepovinné, ale v praxi budete chcieť, aby tieto
rozšírenia boli nainštalované a fungovali. Nanešťastie sa postupy ako
ich sfunkčniť do veľkej miery rôznia. Tu je niekoľko odporúčaní:

Najprv sa uistite, že máte Perl a Python. Ak používate Windows, mali by
ste si tiež nainštalovať Cygwin.

Ďalej sa musíte uistiť, že sú prítomné závislosti každého rozšírenia.
V závislosti na tom, ktoré rozšírenia potrebujete sa budú závilosti
rôzniť, ale tu sú niektoré, ktoré asi budete musieť nainštalovať:

   * XML::Parser
   * XML::XQL

Ak nainštalujete závislosti do neštandardných adresárov ako napr.
nainštalovanie XML::Parser niekde do vášho domovského adresára, budete
musieť uviesť kde sa tieto závislosti majú hľadať. Napr. v prípade
modulov jazyka Perl nastavte premennú prostredia PERLLIB alebo PERL5LIB
(pozri „man perlrun“).

