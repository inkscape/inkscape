Inkscape. Dessinez librement.
=============================

http://www.inkscape.org/

Inkscape est un logiciel de dessin vectoriel libre avec des possibilités
similaires à celles d'Illustrator, Freehand ou CorelDraw et utilisant le format
standard du W3C "scalable vector graphics" (SVG). Formes basiques, chemins,
texte, marqueurs, clonage, transparence, transformations, dégradés et groupage
figurent parmi les fonctionnalités SVG supportées. De plus, Inkscape supporte
les meta-données Creative Commons, l'édition de nœuds, les calques, les
opérations complexes sur les chemins, le texte suivant un chemin, et l'édition
du SVG XML. Il peut aussi importer depuis plusieurs formats comme EPS,
Postscript, JPEG, PNG, BMP, et TIFF et exporter en PNG aussi bien qu'en de
nombreux formats vectoriels.

Le but principal d'Inkscape est de fournir à la communauté du logiciel libre
un outil de dessin totalement conforme aux spécifications XML, SVG et CSS2 du
W3C. De plus les tâches planifiées incluent la conversion du code de C/Gtk en
C++/Gtkmm, la mise en valeur d'un noyau léger avec des fonctionnalités ajoutées
par un mécanisme d'extension, et l'établissement d'un processus de
développement amical, ouvert et tourné vers la communeauté.

Inkscape se compile et s'installe selon la procédure standard : 

  ./configure
  make
  make install

Si le fichier "./configure" n'est pas présent, vous pouvez le créer en
exécutant la commande  "./autogen.sh", qui appelle alternativement d'autres
programmes tels que automake et autoconf. Consultez le fichier INSTALL pour de
plus amples détails.


Dépendances nécessaires
=======================
Le cœur d'Inkscape dépend de plusieurs autres bibliothèques qu'il vous faudra
installer si elle ne sont pas déjà présentes sur votre système. Typiquement,
vous devriez avoir à installer :
  * Boehm-GC
  * libsigc++
  * glibmm
  * gtkmm

Veuillez consulter http://wiki.inkscape.org/wiki/index.php/CompilingInkscape
pour les dépendances les plus courantes, cette page comportant aussi des liens
les fichiers sources (au format .tar.gz)


Dépendances pour les extensions
===============================
Inkscape comporte aussi un certain nombre d'extensions apportant diverses
fonctionnalités (comme le support de formats de fichiers autres que le SVG...).
En théorie toutes les extensions sont optionnelles; cependant, en pratique,
vous voudrez sans doute les installer et faire fonctionner. Malheureusement,
leur bon fonctionnement peut beaucoup varier. Voici quelques recommandations :

D'abord, vérifiez que Python et Perl sont bien installés. Sous Windows,
installez aussi Cygwin.

Ensuite, assurez-vous que les dépendances nécessaires à chaque extension sont
présentes. Ces dépendances varient en fonction des extensions dont vous avez
besoin, mais en voici que vous devriez installer de toute façon :
  * XML::Parser
  * XML::XQL

Si vous installez des dépendances dans des emplacements non standards (ex : si
vous installez XML::Parser dans votre répertoire personnel), il vous faudra 
peut-être effectuer quelques étapes supplémentaires afin d'indiquer où ces
dépendances peuvent être trouvées. Par exemple, pour les modules Perl, il vous
faut définir les variables PERLLIB ou PERL5LIB (voyez 'man perlrun').