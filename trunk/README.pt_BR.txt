Inkscape. Desenhando Livremente.
======================

http://www.inkscape.org/

O Inkscape é uma ferramenta de desenho vetorial, open source (código abertor),
com capacidades similares ao Illustrator, Freehand e CorelDraw. Ele utiliza o formato SVG 
(scalable
vector graphics), um padrão da W3C. Algumas das características suportadas pelo padrão SVG 
são:
formas primitívas, caminhos, texto, marcadores, clones, canal alfa, transformações, 
gradientes, 
filtros e agrupamento de objetos. Como adicional, o Inkscape suporta metadados Creative 
Commons, 
edição de nós, camadas(layers), operações complexas entre caminhos, texto-em-caminho, e um 
editor 
de SVG XML. Ele também importa importantes formatos como EPS, PostScript, JPEG, PNG, BMP e 
TIFF. E exporta 
em PNG além de muitos outros formatos baseados em vetor.

A principal motivação do Inkscape é prover a Comunidade Open Source uma completa ferramenta 
de 
desenho, utilizando os padrões SVG e CSS2 definidos pela W3C. Um trabalho adicional planeja 
é 
incluir a conversão do código base de C/Gtk para C++/Gtkmm, enfatizando um núcleo leve com 
as 
características poderosas adicionadas através de um mecanismo de extensão, e o 
estabelecimento 
de um amigável processo de desenvolvimento orientado a objetos para a comunidade livre.

O Inkscape utiliza os procedimentos básicos para sua compilação e instalação:

 ./configure
  make
  make install

Se o arquivo "./configure" não estiver presente, você pode cria-lo rodando o comando 
"./autogen.sh", 
o qual chama, por conta própria, outros programas como automake e autoconf. Veja a seção 
INSTALL 
para mais detalhes.

Dependencias requeridas
=====================

O Inkscape 
O núcleo do Inkscape depende de diversas outras bibliotecas que, se já não estiverem 
instaladas no seu 
sistema, você precisará instala-las. As bibliotecas mais típicas que você necessitará 
instalar são:

   * Boehm-GC 
   * libsigc++
   * glibmm  
   * gtkmm  

Por favor veja http://wiki.inkscape.org/wiki/index.php/CompilingInkscape (em inglês) para 
saber as 
dependências mais recentes, incluindo links aos tarballs da fonte.

Dependências da extensão
======================
O Inkscape também tem um numero de extensões para a implementação de várias características, 
tais 
como o suporte a arquivos que não possuem o formato SVG, etc. Na teoria, todas as extensões 
são 
opcionais, mas de qualquer modo na prática você terá que tê-las instaladas e funcionando. 
Infelizmente,
 existe uma grande variedade de dependências para que certas extensões funcionem 
corretamente. Aqui 
estão as recomendações:

Primeiramente, certifique-se de ter o Perl e o Python. Se você estiver em Windows você deve 
também 
instalar Cygwin. 

Em seguida, você precisara verificar as dependências de cada extensão presente. Dependendo 
das 
extensões que você necessita, as dependencias vão variar, mas as dependências que você 
provavelmente precisará instalar são:
   * XML::Parser
   * XML::XQL

Se você instalou as dependencias fora dos locais padrão, como instalar o XML::Parser em 
algum lugar 
do seu diretório Home, você precisará verificar os passos extras necessários para indicar 
onde aquelas 
dependências devem ser encontradas. Por exemplo, com módulos do Perl, você deve ajustar as 
variáveis 
PERLLIB ou PERL5LIB ( veja 'man perlrun' )

