#!/usr/bin/perl

BEGIN {
  # adding the inkscape extension dir in the include path list.
  my $path = `inkscape -x`;
  $path =~ s/\r?\n//;
  push @INC, $path;
}

use strict;

use Inkscape;
my $inkscape = Inkscape->new;

my %args = $inkscape->getArgs();
my $svg = $inkscape->getSVG();

my $unit = $args{unit};
my $ml = $svg->convertUnit( $args{'margin-r'}.$unit, 'px' );
my $mr = $svg->convertUnit( $args{'margin-l'}.$unit, 'px' );
my $mt = $svg->convertUnit( $args{'margin-t'}.$unit, 'px' );
my $mb = $svg->convertUnit( $args{'margin-b'}.$unit, 'px' );

my ( $cw, $ch ) = $svg->getCanvasSize( 'px' );

if ( $args{'use-rect'} ) {
  if ( ! $args{id}[0] ) {
    warn "You asked to use a rect to define the margin, but you didn\'t select an element.\n";
    exit 1;
  }
  $unit = '';
  my ( $x, $y ) = $svg->getElPosition( $args{id}[0] );
  my ( $w, $h ) = $svg->getElSize( $args{id}[0] );
  $mr = $x;
  $ml = $cw - ( $x + $w );
  $mt = $y;
  $mb = $ch - ( $y + $h );
}

# Positions for the cut lines:
my $cutR = $mr;
my $cutL = $cw - $ml;
my $cutT = $mt;
my $cutB = $ch - $mb;

# Printing test attributes:
my $ptaCX = $cw - ( $ml / 2 );
my $ptaCY = $ch / 4;
my $ptaR = $ml / 2.5;
$ptaR = $cw / 50  if ( ($cw / 50) < $ptaR );
my $ptaBoxW = $ptaR / 1.5;

my $theX = 'M '.($ptaCX-$ptaR).','.$ptaCY.' L '.($ptaCX+$ptaR).','.$ptaCY.
           'M '.$ptaCX.','.($ptaCY-$ptaR).' L '.$ptaCX.','.($ptaCY+$ptaR);

sub generateGradientTest {
  my ( $color, $x, $y ) = @_;
  my $boxes = '';
  for ( my $i=0; $i<10; $i++ ) {
    $boxes .= '<rect
    fill="'.$color.'" fill-opacity="'.(1/($i+1)).'"
    stroke="'.$color.'" stroke-width="0.5"
    x="'.$x.'" y="'.($y+($ptaBoxW*$i)).'"
    width="'.$ptaBoxW.'" height="'.$ptaBoxW.'" />';
  }
  return $boxes;
}

my $gradBoxes =
     generateGradientTest( "#000", $ptaCX-$ptaBoxW-1, $ptaCY+$ptaR+10 ) .
     generateGradientTest( "#FF0", $ptaCX+1, $ptaCY+$ptaR+10 ) .
     generateGradientTest( "#0FF", $ptaCX-$ptaBoxW-1, $ptaCY+$ptaR+12+($ptaBoxW*10) ) .
     generateGradientTest( "#F0F", $ptaCX+1, $ptaCY+$ptaR+12+($ptaBoxW*10) );

my $marks = <<END_XML ;
  <g
     inkscape:groupmode="layer"
     inkscape:label="Cut Marks"
     sodipodi:insensitive="true"
     style="display:inline">
    <path fill="none" stroke="#000" stroke-width="0.5" d="
      M $cutR,0 L $cutR,$cutT M $cutR,$cutB L $cutR,$ch
      M $cutL,0 L $cutL,$cutT M $cutL,$cutB L $cutL,$ch
      M 0,$cutT L $cutR,$cutT M $cutL,$cutT L $cw,$cutT
      M 0,$cutB L $cutR,$cutB M $cutL,$cutB L $cw,$cutB
      " />
    <circle class="test-print-align"
      fill="none" stroke="#000" stroke-width="0.5"
      r="$ptaR" cx="$ptaCX" cy="$ptaCY" />
    <path class="test-print-align"
      fill="none" stroke="#000" stroke-width="0.5" d="$theX" />
    $gradBoxes
  </g>
END_XML

$svg->{code} =~ s!</svg>!$marks</svg>!g;

print $svg->{code};
