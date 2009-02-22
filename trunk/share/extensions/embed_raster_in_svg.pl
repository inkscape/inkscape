#!/usr/bin/perl

#----------------------------------------------------------------------------------------------
# embed_raster_in_svg
# B. Crowell, crowellXX at lightandmatter.com (replace XX with last two digits of current year)
# (c) 2004 B. Crowell
# This program is available under version 2 of the GPL License, http://www.gnu.org/copyleft/gpl.html,
# or, at your option, under the same license as Perl.
# For information about this program, scroll down in the source code, or invoke the program
# without any command-line arguments.
#----------------------------------------------------------------------------------------------

use strict;

use MIME::Base64;
use Cwd;
use File::Basename;

my $info = <<'INFO';
 usage:
    embed_raster_in_svg foo.svg
 Looks through the svg file for raster images that are given as links, rather than
 being embedded, and embeds them in the file. The links must be local URIs, and if
 they're relative, they must be relative to the directory where the svg file is located.
 Starting with Inkscape 0.41, Inkscape can
 handle embedded raster images as well as ones that are linked to (?).
 Typical input looks like this:
  <image
     xlink:href="beer.jpg"
     sodipodi:absref="/home/bcrowell/Documents/programming/embed_raster_in_svg/tests/beer.jpg"
     width="202.50000"
     height="537.00000"
     id="image1084"
     x="281.67480"
     y="242.58502" />
  The output should look like this:
   <image
     width="202.50000"
     height="537.00000"
     id="image1084"
     x="281.67480"
     y="242.58502"
     xlink:href="data:;base64,...
  The SVG standard only allows the <image> tag to refer to data of type png, jpg, or svg,
  and this script only handles png or jpg.
INFO

undef $/; # slurp whole file

my $svg = $ARGV[0];

if (!$svg) {
  print $info;
  exit;
}

die "input file $svg not found" if ! -e $svg;
die "input file $svg not readable" if ! -r $svg;

open(FILE,"<$svg") or die "error opening file $svg for input, $!";
my $data = <FILE>;
close FILE;

chdir dirname(Cwd::realpath($svg)); # so from now on, we can open raster files if they're relative to this directory


# The file is now in memory.
# First we go once through it and (1) find out if there are indeed any links to raster images,
# (2) rearrange things a little so that the (lengthy) base64 data will be the last attribute of the
# image tag.

my $n = 0;
my @raster_files = ();
$data =~
  s@\<image\s+((?:(?:\w|\:)+\=\"[^"]*\"\s*)*)xlink\:href=\"([^"]+)\"\s*((?:(?:\w|:)+\=\"[^"]*\"\s*)*)\/>@<imageHOBBITSES $1 $3 ITHURTSUS\"$2\"MYPRECIOUSS />@g;
while ($data=~m@<imageHOBBITSES\s*(?:(?:(?:\w|\:)+\=\"[^"]*\"\s*)*)ITHURTSUS\"([^"]+)\"MYPRECIOUSS />@g) {
  my $raster = $1;
  die "error, raster filename $raster contains a double quote" if $raster=~m/\"/;
  if (!($raster=~m/data\:\;/)) {
    ++$n;
    push @raster_files,$raster;
  }
}

if ($n==0) {
  print "no embedded jpgs found in file $svg\n";
  exit;
}


# Eliminate sodipodi:absref attributes. If these are present, Inkscape looks for the linked
# file, and ignores the embedded data. Also, get rid of those nasty hobbitses.
$data=~s@(<image)HOBBITSES\s*((?:(?:\w|\:)+\=\"[^"]*\"\s*)*)sodipodi\:absref\=\"[^"]+\"\s*((:?(?:\w|\:)+\=\"[^"]*\"\s*)*ITHURTSUS)@$1 $2 $3@g;
$data=~s@(<image)HOBBITSES@$1@g;


# Now embed the data:

foreach my $raster(@raster_files) {
  die "file $raster not found relative to directory ".getcwd() if ! -e $raster;
  die "file $raster is nor readable" if ! -r $raster;
  open(RASTER,"<$raster") or die "error opening file $raster for input, $!";
  my $raster_data = <RASTER>;
  close RASTER;
  my $type = '';
  $type='image/png' if $raster_data=~m/^\x{89}PNG/;
  $type='image/jpg' if $raster_data=~m/^\x{ff}\x{d8}/;
  die "file $raster does not appear to be of type PNG or JPG" unless $type;
  my $raster_data_base64 = encode_base64($raster_data);
  ($data =~ s@ITHURTSUS\"$raster\"MYPRECIOUSS@\n     xlink:href=\"data\:$type\;base64,$raster_data_base64\"@) or die "error embedding data for $raster";
  print "embedded raster file $raster, $type\n";
}
my $bak = "$svg.bak";
rename $svg,$bak or die "error renaming file $svg to $bak, $!";
open(FILE,">$svg") or die "error creating new version of file $svg for output, $!";
print FILE $data;
close FILE;





