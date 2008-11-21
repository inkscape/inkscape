#!/usr/bin/perl

use strict;
use warnings;

=head1 NAME

Inkscape - a helper for Inkscape extensions writen in PERL

=head1 SYNOPSIS

    use Inkscape;
    my ($w, $h) = $inkscape->getCanvasSize;
    $svg->setElAttribute {tag=>'svg',pos=>0}, 'width', $w * 2;

=head1 DESCRIPTION

This package try to do the common initial work in inkscape extensions
and provide a collection of helper methods about inkscape interaction
and SVG basic manipulation.

If you want more power to SVG manipulation, try use SVG::DOM together.
http://search.cpan.org/~ronan/SVG-2.44/lib/SVG/DOM.pm

=cut

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                           Inkscape Package                                #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
{
package Inkscape;
our $VERSION = "0.01";

=head2 The %args hash

The B<%args> hash gives to you all arguments provided by Inkscape.

    $svg->setElAttribute 'svg', 'width', $w * $args{zoom};

The B<id> will allways be an array reference. Also if it is empty.

=cut

#use vars qw($VERSION $inkscape %args @ISA @EXPORT %EXPORT_TAGS $TODO);
#require Exporter;
#@ISA = qw(Exporter);
#my $inkscape = Inkscape->new;
#@EXPORT = qw( $inkscape );

sub import {
  if ( defined $_[1] && $_[1] eq 'AUTO_LOAD' ) {
    #$inkscape->getArgs(@ARGV);
    if ( $#ARGV > -1 && -f $ARGV[$#ARGV] ) {
      #$inkscape->getSVG($ARGV[$#ARGV]);
    }
  }
}

sub getArgs {
  my $self = shift;
  my @argv = @_;
  @argv = @ARGV if ( $#argv == -1 );
  my %args = (id => []);
  foreach ( @argv ) {
    if ( m/^--([^=]+)=(.*)$/ ) {
      my $key = $1;
      my $val = $2;
      $val = 1 if $val eq 'true';
      $val = 0 if $val eq 'false';
      if ( defined $args{$key} ) {
        if ( ref($args{$key}) ne 'ARRAY' ) {
          $args{$key} = [ $args{$key} ];
        }
        push( @{$args{$key}}, $val );
      } else {
        $args{$key} = $val;
      }
    }
  }
  %args;
}

sub getSVG{
  my $self = shift;
  my $file = shift;
  SVGHelper->new( $file );
}

=head2 Inkscape Methods

The $inkscape auto defined object is a helper to use non interactive
inkscape interface. You allways need to provide an SVG file path to
the methods.

=cut

my $singleton;
sub new {
  my $class = shift;
  my $self  = {};
  #$self->{args} = ( id => [] );
  $singleton ||= bless $self, $class;
}

} # end package Inkscape


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                            SVG Package                                    #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
{
package SVGHelper;
our $VERSION = "0.01";

=head2 SVGHelper Methods

The $svg auto defined object is a helper to access the SVG file provided
by the Inkscape. That is not a singleton, so if you want manipulate other
SVG files, you can instanciate a new SVGHelper.

=head3 new

    my $mysvg = SVGHelper->new( '/path/to/my.svg' );

Instantiates SVGHelper with a SVG file.

=cut

sub new {
  my ( $class, $file ) = @_;
  my $self  = {};
  $file = $ARGV[$#ARGV] if ( ! $file );
  open SVG, $file or die "Can't open \"$file\": $!\n";
  $self->{file} = $file;
  $self->{code} = join('',<SVG>);
  close SVG;
  bless $self, $class;
}

=head3 generateSearchRegExp

    if ( $svg->{code} =~ m/$svg->generateSearchRegExp({class=>"hot"})/ )
      print "Yes! there are Hot elements!"

Generates the regexp string to localize tags in the SVG, based in the arguments
on the localizator hash.

Arguments to localize the tags:
    B<{id}> = "<some tag id>"
        Select only one tag with this id. You don't need to use other select
        arguments with this one.
    B<{pos}> = <number>
        When you use inprecise arguments for selection the position will select
        only one tag in the defined position at the finded list of tags.
    B<{tag}> = "<tag_name>"
        Select by tag name, may select a list if you don't use {pos}.
    B<{>B<<any tag attribute>>B<}> = <a valid attribute value>
        Select by

=cut

sub generateSearchRegExp {
  # '.*' in a value will break all. we must change '.' by '[^"]'
  
}

=head3 setElAttribute

    # Setting an atribute in a tag localizated it's id:
    $svg->setElAttribute {id=>'myDrawing'}, 'width', $w * 2;
    # Setting an atribute in a tag localizated it's position:
    $svg->setElAttribute {tag=>'svg',pos=>0}, 'width', $w * 2;
    # Setting an atribute in some tags localizated by atributes:
    $svg->setElAttribute {tag=>'circle',r=>10}, 'r', 50;

This method will set an atribute in a tag or in a colection of tags, selected
by the localizator hash.

=cut

sub setElAttribute {
  my $self = shift;
  my ( $searchArgs, $att, $val ) = @_;
  print $self->generateSearchRegExp( $searchArgs );
}

sub convertUnit {
  my $self = shift;
  $_[0] =~ m/^([.0-9]*)\s*([^ ]*)/;
  my $num = $1;
  my $unFrom = $2 || 'px';
  my $unTo = $_[1];
  my $appendUnit = $_[2];
  # From http://www.w3.org/TR/SVG/coords.html#Units
  # "1pt" equals "1.25px" (and therefore 1.25 user units)
  # "1pc" equals "15px" (and therefore 15 user units)
  # "1mm" would be "3.543307px" (3.543307 user units)
  # "1cm" equals "35.43307px" (and therefore 35.43307 user units)
  # "1in" equals "90px" (and therefore 90 user units)
  my %equivPX = (
        px => 1,
        pt => 1.25,
        pc => 15,
        mm => 3.543307,
        cm => 35.43307,
        in => 90
      );
  ( ( $num * $equivPX{$unFrom} ) / $equivPX{$unTo} ) . ( $appendUnit ? $unTo : '' );
}

=head3 getCanvasSize

    my ($w, $h) = $svg->getCanvasSize;

Get the B<width> and B<height> values of the B<<svg>> tag.

=cut

sub getCanvasSize {
  my $self = shift;
  my $unitTo = $_[0];
  $self->{code} =~ m/<svg\s[^>]*width="([^"]*)"[^>]*height="([^"]*)"[^>]*>/;
  my ( $w, $h ) =
       ( $unitTo ) ?
       ( $self->convertUnit($1, $unitTo), $self->convertUnit($2, $unitTo) ) :
       ( $1, $2 );
}

my $sysNULL = ( -e '/dev/null' )? '/dev/null' : 'NIL';

sub getElPosition {
  my $self = shift;
  my $x = `inkscape --query-id=$_[0] --query-x "$self->{file}" 2>$sysNULL`;
  my $y = `inkscape --query-id=$_[0] --query-y "$self->{file}" 2>$sysNULL`;
  return ( $x )? ( $x, $y ) : undef;
}

sub getElSize {
  my $self = shift;
  my $w = `inkscape --query-id=$_[0] --query-width  "$self->{file}" 2>$sysNULL`;
  my $h = `inkscape --query-id=$_[0] --query-height "$self->{file}" 2>$sysNULL`;
  return ( $w )? ( $w, $h ) : undef;
}

} # end package SVGHelper

=head1 AUTHOR

Aurelio A. Heckert <aurium@gmail.com>

=head1 COPYRIGHT

Copyright (C) 2008 Aurelio A. Heckert <aurium@gmail.com>

This PERL module is a free software licenced under LGPL v3
http://www.gnu.org/licenses/lgpl-3.0-standalone.html

=cut

1;
