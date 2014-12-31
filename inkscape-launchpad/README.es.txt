
Inkscape. Dibuja en libertad.
=============================

http://www.inkscape.org/

Inkscape es una herramienta de dibujo de código abierto con capacidades
similares a Illustrator, Freehand y CorelDraw, que utiliza el formato estándar
para gráficos vectoriales escalables (SVG = scalable vector graphics) del W3C.
Algunas de las características de SVG incluidas son: formas básicas, trazos,
texto, marcadores, clones, mezclas alfa, transformaciones, degradados y
agrupación. Además Inkscape permite la inclusión de metadatos de Creative Commons,
edición de nodos, capas, operaciones complejas con trazos, textos en trayectos
y edición de XML SVG. También importa varios formatos, por ejemplo, EPS,
Postscript, JPEG, PNG, BMP y TIFF y exporta en PNG, así como en varios formatos
basados en vectores.

La principal motivación de Inkscape es la de proporcionar a la comunidad de
código abierto con una herramienta de dibujo que cumpla con las especificaciones
XML, SVG y CSS2 del W3C. Entre las tareas planeadas adicionales se encuentra la
conversión del código de C/Gtk a C++/Gtkmm, haciendo hincapié en un núcleo
ligero con la posibilidad de añadir características potentes a través de un
mecanismo de extensiones y el establecimiento de un proceso de desarrollo
amistoso, abierto y orientado a la comunidad.

Inkscape utiliza el procedimiento estándar para su compilación e instalación:

./configure make make install 

Si no existe un archivo «./configure» puede crearlo al ejecutar el comando
«./autogen.sh» que, a su vez, llamará a otros programas como automake y autoconf.
Vea el archivo INSTALL para obtener más detalles.


Dependencias requeridas
=====================
El núcleo de Inkscape depende de varias otras bibliotecas que necesitará
instalar si no existen ya en su sistema. Las bibliotecas que normalmente
necesitará instalar son:

 * Boehm-GC
 * libsigc++
 * glibmm
 * gtkmm  

Acceda a http://wiki.inkscape.org/wiki/index.php/CompilingInkscape para obtener
una lista de las dependencias más actuales, además de los enlaces a los paquetes
de código fuente.


Dependencias de extensiones
======================
Inkscape también dispone de varias extensiones que desarrollan algunas
características como el soporte para formatos de archivos distintos de SVG, etc.
Teóricamente todas las extensiones son opcionales, aunque en la practica querrá
tenerlas instaladas y funcionando. Desafortunadamente hay una gran variedad de
formas de conseguir que éstas funcionen correctamente. He aquí algunas
recomendaciones:

En primer lugar, asegúrese de que tiene Perl y Python. Si utiliza windows
también debería instalar Cygwin.

Además deberá asegurarse de que estén presentes las dependencias de cada
extensión. Las dependencias variarán según las extensiones que necesite; aquí hay
algunas que necesitará instalar:

 * XML::Parser
 * XML::XQL

Si instala dependencias en rutas que no son las habituales, como por ejemplo
instalar XML::Parser en algún lugar de su directorio personal, deberá dar
algunos pasos más para indicar dónde se encuentran esas dependencias. Por ejemplo,
con los módulos de Perl, ajuste la variable PERLLIB o PERL5LIB (véase
«man perlrun»)

